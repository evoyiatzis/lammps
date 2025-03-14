/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   LAMMPS development team: developers@lammps.org

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Contributing author: Joel Clemmer (SNL)
------------------------------------------------------------------------- */

#include "fix_bond_history.h"

#include "atom.h"
#include "comm.h"
#include "error.h"
#include "group.h"
#include "memory.h"
#include "modify.h"
#include "neighbor.h"

#include <map>
#include <utility>

using namespace LAMMPS_NS;
using namespace FixConst;

static constexpr double LB_FACTOR = 1.5;
static constexpr int DELTA = 8192;

/* ---------------------------------------------------------------------- */

FixBondHistory::FixBondHistory(LAMMPS *lmp, int narg, char **arg) :
    Fix(lmp, narg, arg), bondstore(nullptr), bondtype_orig(nullptr), bondstore_comp(nullptr),
    bondstore_orig(nullptr), id_fix(nullptr), id_array(nullptr)

{
  if (narg != 5) error->all(FLERR, "Illegal fix bond/history command");
  update_flag = utils::inumeric(FLERR, arg[3], false, lmp);
  ndata = utils::inumeric(FLERR, arg[4], false, lmp);
  nbond = atom->bond_per_atom;

  if (nbond == 0) error->all(FLERR, "Cannot store bond variables without any bonds");

  stored_flag = false;
  restart_global = 1;
  create_attribute = 1;

  // Flag whether bond arrays are updated such that data may need to be
  // copied to atom arrays before exchanging atoms
  // Prevents sequential calls to pre_exchange() without post_neighbor()
  updated_bond_flag = 0;

  maxbond = 0;
}

/* ---------------------------------------------------------------------- */

FixBondHistory::~FixBondHistory()
{
  if (id_fix && modify->nfix) modify->delete_fix(id_fix);
  delete[] id_fix;
  delete[] id_array;

  memory->destroy(bondstore);
  memory->destroy(bondstore_comp);
  memory->destroy(bondtype_orig);
}

/* ---------------------------------------------------------------------- */

int FixBondHistory::setmask()
{
  int mask = 0;
  mask |= PRE_EXCHANGE;
  mask |= POST_NEIGHBOR;
  return mask;
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::post_constructor()
{
  // Store saved bond quantities for each atom using fix property atom
  // Don't copy history to data files because this fix is typically
  // not yet instantiated - history is only preserved across restarts

  id_fix = utils::strdup(id + std::string("_FIX_PROP_ATOM"));
  id_array = utils::strdup(std::string("d2_") + id);
  modify->add_fix(fmt::format("{} {} property/atom {} {} writedata no", id_fix,
                              group->names[igroup], id_array, nbond * ndata));
  int tmp1, tmp2;
  index = atom->find_custom(&id_array[3], tmp1, tmp2);
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::update_atom_value(int i, int m, int idata, double value)
{
  if (idata >= ndata || m > nbond) error->one(FLERR, "Index exceeded in fix bond history");
  atom->darray[index][i][m * ndata + idata] = value;
}

/* ---------------------------------------------------------------------- */

double FixBondHistory::get_atom_value(int i, int m, int idata)
{
  if (idata >= ndata || m > nbond) error->one(FLERR, "Index exceeded in fix bond history");
  return atom->darray[index][i][m * ndata + idata];
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::setup_pre_exchange()
{
  pre_exchange();
}

/* ----------------------------------------------------------------------
   If stored values are updated, need to copy to atom arrays before exchanging
   If bondstore array has been allocated, call before nlist rebuild
   Also call prior to irregular communication in other fixes (e.g. deform)
------------------------------------------------------------------------- */

void FixBondHistory::pre_exchange()
{
  if (!update_flag) return;
  if (!stored_flag) return;
  if (!updated_bond_flag) return;

  int i1, i2, n, m, idata;
  int **bondlist = neighbor->bondlist;
  int nbondlist = neighbor->nbondlist;
  double **stored = atom->darray[index];

  int nlocal = atom->nlocal;
  tagint **bond_atom = atom->bond_atom;
  int **bond_type = atom->bond_type;
  int *num_bond = atom->num_bond;
  tagint *tag = atom->tag;

  for (n = 0; n < nbondlist; n++) {
    i1 = bondlist[n][0];
    i2 = bondlist[n][1];

    // skip bond if already broken or not allocated
    if (bondlist[n][2] <= 0 || !setflag[bondlist[n][2]]) { continue; }

    if (i1 < nlocal) {
      for (m = 0; m < num_bond[i1]; m++) {
        if (bond_atom[i1][m] == tag[i2] && setflag[bond_type[i1][m]]) {
          for (idata = 0; idata < ndata; idata++) {
            stored[i1][m * ndata + idata] = bondstore[n][idata];
          }
        }
      }
    }

    if (i2 < nlocal) {
      for (m = 0; m < num_bond[i2]; m++) {
        if (bond_atom[i2][m] == tag[i1] && setflag[bond_type[i2][m]]) {
          for (idata = 0; idata < ndata; idata++) {
            stored[i2][m * ndata + idata] = bondstore[n][idata];
          }
        }
      }
    }
  }

  updated_bond_flag = 0;
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::allocate()
{
  //Ideally would just ask ntopo for maxbond, but protected
  if (comm->nprocs == 1)
    maxbond = atom->nbonds;
  else
    maxbond = static_cast<int>(LB_FACTOR * atom->nbonds / comm->nprocs);
  memory->create(bondstore, maxbond, ndata, "fix_bond_store:bondstore");
  if (hybrid_flag) {
    memory->create(bondstore_comp, maxbond, ndata, "fix_bond_store:bondstore_comp");
    memory->create(bondtype_orig, maxbond, "fix_bond_store:bondtype_orig");
  }
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::setup_post_neighbor()
{
  hybrid_flag = 0;
  for (int i = 1; i <= atom->nbondtypes; i++)
    if (!setflag[i]) hybrid_flag = 1;

  if (maxbond == 0) allocate();

  pre_exchange();
  post_neighbor();
}

/* ----------------------------------------------------------------------
   called after neighbor list is build
   build array of stored bond quantities from fix property atom
------------------------------------------------------------------------- */

void FixBondHistory::post_neighbor()
{
  //Grow array if number of bonds has increased
  while (neighbor->nbondlist >= maxbond) {
    maxbond += DELTA;
    memory->grow(bondstore, maxbond, ndata, "fix_bond_store:bondstore");
    if (hybrid_flag) {
      memory->grow(bondstore_comp, maxbond, ndata, "fix_bond_store:bondstore_comp");
      memory->grow(bondtype_orig, maxbond, "fix_bond_store:bondtype_orig");
    }
  }

  int i1, i2, n, m, idata;
  int **bondlist = neighbor->bondlist;
  int nbondlist = neighbor->nbondlist;
  double **stored = atom->darray[index];

  int nlocal = atom->nlocal;
  tagint **bond_atom = atom->bond_atom;
  int **bond_type = atom->bond_type;
  int *num_bond = atom->num_bond;
  tagint *tag = atom->tag;

  for (n = 0; n < nbondlist; n++) {
    i1 = bondlist[n][0];
    i2 = bondlist[n][1];

    // skip bond if already broken or not allocated
    if (bondlist[n][2] <= 0 || !setflag[bondlist[n][2]]) { continue; }

    if (i1 < nlocal) {
      for (m = 0; m < num_bond[i1]; m++) {
        if (bond_atom[i1][m] == tag[i2] && setflag[bond_type[i1][m]]) {
          for (idata = 0; idata < ndata; idata++) {
            bondstore[n][idata] = stored[i1][m * ndata + idata];
          }
        }
      }
    }

    if (i2 < nlocal) {
      for (m = 0; m < num_bond[i2]; m++) {
        if (bond_atom[i2][m] == tag[i1] && setflag[bond_type[i2][m]]) {
          for (idata = 0; idata < ndata; idata++) {
            bondstore[n][idata] = stored[i2][m * ndata + idata];
          }
        }
      }
    }
  }

  if (hybrid_flag) {
    nbondlist_orig = nbondlist;
    for (n = 0; n < nbondlist; n++) bondtype_orig[n] = bondlist[n][2];
  }

  updated_bond_flag = 1;
}

/* ---------------------------------------------------------------------- */

double FixBondHistory::memory_usage()
{
  return (double) maxbond * ndata * sizeof(double);
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::write_restart(FILE *fp)
{
  int n = 0;
  double list[1];
  list[n++] = stored_flag;

  // Update stored values if needed
  pre_exchange();

  if (comm->me == 0) {
    int size = n * sizeof(double);
    fwrite(&size, sizeof(int), 1, fp);
    fwrite(list, sizeof(double), n, fp);
  }
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::restart(char *buf)
{
  int n = 0;
  double *list = (double *) buf;
  stored_flag = static_cast<int>(list[n++]);
}

/* ----------------------------------------------------------------------
   initialize bond values to zero, called when atom is created
------------------------------------------------------------------------- */

void FixBondHistory::set_arrays(int i)
{
  double **stored = atom->darray[index];
  for (int m = 0; m < nbond; m++)
    for (int idata = 0; idata < ndata; idata++) stored[i][m * ndata + idata] = 0.0;
}

/* ----------------------------------------------------------------------
   Compress history arrays, cutting out unused types, for bond hybrid
------------------------------------------------------------------------- */

void FixBondHistory::compress_history()
{
  // if this is a re-neighbor step or updating, compress bondstore

  int type;
  int ncomp = 0;
  if (update_flag || (neighbor->ago == 0)) {
    for (int n = 0; n < nbondlist_orig; n++) {
      type = bondtype_orig[n];

      if (type <= 0) continue;
      if (!setflag[type]) continue;

      for (int m = 0; m < ndata; m++) bondstore_comp[ncomp][m] = bondstore[n][m];
      ncomp += 1;
    }
  }

  // replace ptr to original array
  bondstore_orig = bondstore;
  bondstore = bondstore_comp;
}

/* ---------------------------------------------------------------------- */

void FixBondHistory::uncompress_history()
{
  if (update_flag) {
    int type;
    int ncomp = 0;
    for (int n = 0; n < nbondlist_orig; n++) {
      type = bondtype_orig[n];

      if (type <= 0) continue;
      if (!setflag[type]) continue;

      for (int m = 0; m < ndata; m++) bondstore_orig[n][m] = bondstore[ncomp][m];
      ncomp += 1;
    }
  }

  // restore ptr to original array
  bondstore = bondstore_orig;
}

/* ----------------------------------------------------------------------
   Delete bond by zeroing data
------------------------------------------------------------------------- */

void FixBondHistory::delete_history(int i, int m)
{
  double **stored = atom->darray[index];
  for (int idata = 0; idata < ndata; idata++) stored[i][m * ndata + idata] = 0.0;
}

/* ----------------------------------------------------------------------
   Shift bond data to a new location
------------------------------------------------------------------------- */

void FixBondHistory::shift_history(int i, int m, int k)
{
  if (m == k) return;

  double **stored = atom->darray[index];
  for (int idata = 0; idata < ndata; idata++)
    stored[i][m * ndata + idata] = stored[i][k * ndata + idata];
}

/* ----------------------------------------------------------------------
   Temporarily caches history for a deleted bond which
   could be recreated before the cache is emptied
   NOTE: the cache methods still need to be tested, need an example first
------------------------------------------------------------------------- */

void FixBondHistory::cache_history(int i, int m)
{
  // Order tags to create a unique key pair
  tagint max_tag = MAX(atom->tag[i], atom->bond_atom[i][m]);
  tagint min_tag = MIN(atom->tag[i], atom->bond_atom[i][m]);
  auto key = std::make_pair(min_tag, max_tag);

  // Copy data to vector
  double **stored = atom->darray[index];
  std::vector<double> data;
  for (int idata = 0; idata < ndata; idata++) data.push_back(stored[i][m * ndata + idata]);

  // Add data to cache
  cached_histories.insert(std::make_pair(key, data));
}

/* ----------------------------------------------------------------------
   Checks to see if a newly created bond has cached history
------------------------------------------------------------------------- */

void FixBondHistory::check_cache(int i, int m)
{
  // Order tags to create a unique key pair
  tagint max_tag = MAX(atom->tag[i], atom->bond_atom[i][m]);
  tagint min_tag = MIN(atom->tag[i], atom->bond_atom[i][m]);
  auto key = std::make_pair(min_tag, max_tag);

  // Check if it exists, if so, copy data
  double **stored = atom->darray[index];
  std::vector<double> data;
  auto pos = cached_histories.find(key);
  if (pos != cached_histories.end()) {
    data = pos->second;
    for (int idata = 0; idata < ndata; idata++) stored[i][m * ndata + idata] = data[idata];
  }
}

/* ----------------------------------------------------------------------
   Delete saved memory
------------------------------------------------------------------------- */

void FixBondHistory::clear_cache()
{
  cached_histories.clear();
}
