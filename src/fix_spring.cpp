// clang-format off
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
   Contributing author: Paul Crozier (SNL)
------------------------------------------------------------------------- */

#include "fix_spring.h"

#include "atom.h"
#include "error.h"
#include "group.h"
#include "respa.h"
#include "update.h"

#include <cmath>
#include <cstring>

using namespace LAMMPS_NS;
using namespace FixConst;

static constexpr double SMALL = 1.0e-10;

enum{TETHER,COUPLE};

/* ---------------------------------------------------------------------- */

FixSpring::FixSpring(LAMMPS *lmp, int narg, char **arg) :
  Fix(lmp, narg, arg),
  group2(nullptr)
{
  if (narg < 9) utils::missing_cmd_args(FLERR, "fix spring", error);

  scalar_flag = 1;
  vector_flag = 1;
  size_vector = 4;
  global_freq = 1;
  extscalar = 1;
  extvector = 1;
  energy_global_flag = 1;
  dynamic_group_allow = 1;
  respa_level_support = 1;
  ilevel_respa = 0;

  if (strcmp(arg[3], "tether") == 0) {
    if (narg != 9)
      error->all(FLERR, Error::NOPOINTER, "Incorrect number of arguments for tether mode");
    styleflag = TETHER;
    k_spring = utils::numeric(FLERR,arg[4],false,lmp);
    xflag = yflag = zflag = 1;
    if (strcmp(arg[5],"NULL") == 0) xflag = 0;
    else xc = utils::numeric(FLERR,arg[5],false,lmp);
    if (strcmp(arg[6],"NULL") == 0) yflag = 0;
    else yc = utils::numeric(FLERR,arg[6],false,lmp);
    if (strcmp(arg[7],"NULL") == 0) zflag = 0;
    else zc = utils::numeric(FLERR,arg[7],false,lmp);
    r0 = utils::numeric(FLERR,arg[8],false,lmp);
    if (r0 < 0) error->all(FLERR, 8, "R0 < 0 for fix spring command");

  } else if (strcmp(arg[3],"couple") == 0) {
    if (narg != 10)
      error->all(FLERR, Error::NOPOINTER, "Incorrect number of arguments for couple mode");
    styleflag = COUPLE;

    group2 = utils::strdup(arg[4]);
    igroup2 = group->find(group2);
    if (igroup2 < 0)
      error->all(FLERR, 4, "Could not find fix spring couple second group ID {}", group2);
    if (igroup2 == igroup)
      error->all(FLERR,"The two groups cannot be the same in fix spring couple");
    group2bit = group->get_bitmask_by_id(FLERR, group2, "fix spring");

    k_spring = utils::numeric(FLERR,arg[5],false,lmp);
    xflag = yflag = zflag = 1;
    if (strcmp(arg[6],"NULL") == 0) xflag = 0;
    else xc = utils::numeric(FLERR,arg[6],false,lmp);
    if (strcmp(arg[7],"NULL") == 0) yflag = 0;
    else yc = utils::numeric(FLERR,arg[7],false,lmp);
    if (strcmp(arg[8],"NULL") == 0) zflag = 0;
    else zc = utils::numeric(FLERR,arg[8],false,lmp);
    r0 = utils::numeric(FLERR,arg[9],false,lmp);
    if (r0 < 0) error->all(FLERR,"R0 < 0 for fix spring command");

  } else error->all(FLERR, 3, "Unknown fix spring keyword {}", arg[3]);

  ftotal[0] = ftotal[1] = ftotal[2] = ftotal[3] = 0.0;
}

/* ---------------------------------------------------------------------- */

FixSpring::~FixSpring()
{
  delete [] group2;
}

/* ---------------------------------------------------------------------- */

int FixSpring::setmask()
{
  int mask = 0;
  mask |= POST_FORCE;
  mask |= POST_FORCE_RESPA;
  mask |= MIN_POST_FORCE;
  return mask;
}

/* ---------------------------------------------------------------------- */

void FixSpring::init()
{
  // recheck that group 2 has not been deleted

  if (group2) {
    igroup2 = group->find(group2);
    if (igroup2 < 0)
      error->all(FLERR, Error::NOLASTLINE, "Could not find fix spring couple second group ID {}",
                 group2);
    group2bit = group->get_bitmask_by_id(FLERR, group2, "fix spring");
  }

  masstotal = group->mass(igroup);
  if (styleflag == COUPLE) masstotal2 = group->mass(igroup2);

  if (utils::strmatch(update->integrate_style,"^respa")) {
    ilevel_respa = (dynamic_cast<Respa *>(update->integrate))->nlevels-1;
    if (respa_level >= 0) ilevel_respa = MIN(respa_level,ilevel_respa);
  }
}

/* ---------------------------------------------------------------------- */

void FixSpring::setup(int vflag)
{
  if (utils::strmatch(update->integrate_style,"^verlet"))
    post_force(vflag);
  else {
    (dynamic_cast<Respa *>(update->integrate))->copy_flevel_f(ilevel_respa);
    post_force_respa(vflag,ilevel_respa,0);
    (dynamic_cast<Respa *>(update->integrate))->copy_f_flevel(ilevel_respa);
  }
}

/* ---------------------------------------------------------------------- */

void FixSpring::min_setup(int vflag)
{
  post_force(vflag);
}

/* ---------------------------------------------------------------------- */

void FixSpring::post_force(int /*vflag*/)
{
  if (styleflag == TETHER) spring_tether();
  else spring_couple();
}

/* ---------------------------------------------------------------------- */

void FixSpring::spring_tether()
{
  double xcm[3];

  if (group->dynamic[igroup])
    masstotal = group->mass(igroup);

  group->xcm(igroup,masstotal,xcm);

  // fx,fy,fz = components of k * (r-r0) / masstotal

  double dx,dy,dz,fx,fy,fz,r,dr;

  dx = xcm[0] - xc;
  dy = xcm[1] - yc;
  dz = xcm[2] - zc;
  if (!xflag) dx = 0.0;
  if (!yflag) dy = 0.0;
  if (!zflag) dz = 0.0;
  r = sqrt(dx*dx + dy*dy + dz*dz);
  r = MAX(r,SMALL);
  dr = r - r0;

  fx = k_spring*dx*dr/r;
  fy = k_spring*dy*dr/r;
  fz = k_spring*dz*dr/r;
  ftotal[0] = -fx;
  ftotal[1] = -fy;
  ftotal[2] = -fz;
  ftotal[3] = sqrt(fx*fx + fy*fy + fz*fz);
  if (dr < 0.0) ftotal[3] = -ftotal[3];
  espring = 0.5*k_spring * dr*dr;

  if (masstotal > 0.0) {
    fx /= masstotal;
    fy /= masstotal;
    fz /= masstotal;
  }

  // apply restoring force to atoms in group

  double **f = atom->f;
  int *mask = atom->mask;
  int *type = atom->type;
  double *mass = atom->mass;
  double *rmass = atom->rmass;
  int nlocal = atom->nlocal;

  double massone;

  if (rmass) {
    for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        massone = rmass[i];
        f[i][0] -= fx*massone;
        f[i][1] -= fy*massone;
        f[i][2] -= fz*massone;
      }
  } else {
    for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        massone = mass[type[i]];
        f[i][0] -= fx*massone;
        f[i][1] -= fy*massone;
        f[i][2] -= fz*massone;
      }
  }
}

/* ---------------------------------------------------------------------- */

void FixSpring::spring_couple()
{
  double xcm[3],xcm2[3];

  if (group->dynamic[igroup])
    masstotal = group->mass(igroup);

  if (group->dynamic[igroup2])
    masstotal2 = group->mass(igroup2);

  group->xcm(igroup,masstotal,xcm);
  group->xcm(igroup2,masstotal2,xcm2);

  // fx,fy,fz = components of k * (r-r0) / masstotal
  // fx2,fy2,fz2 = components of k * (r-r0) / masstotal2

  double dx,dy,dz,fx,fy,fz,fx2,fy2,fz2,r,dr;

  dx = xcm2[0] - xcm[0] - xc;
  dy = xcm2[1] - xcm[1] - yc;
  dz = xcm2[2] - xcm[2] - zc;
  if (!xflag) dx = 0.0;
  if (!yflag) dy = 0.0;
  if (!zflag) dz = 0.0;
  r = sqrt(dx*dx + dy*dy + dz*dz);
  r = MAX(r,SMALL);
  dr = r - r0;

  fx = k_spring*dx*dr/r;
  fy = k_spring*dy*dr/r;
  fz = k_spring*dz*dr/r;
  ftotal[0] = fx;
  ftotal[1] = fy;
  ftotal[2] = fz;
  ftotal[3] = sqrt(fx*fx + fy*fy + fz*fz);
  if (dr < 0.0) ftotal[3] = -ftotal[3];
  espring = 0.5*k_spring * dr*dr;

  if (masstotal2 > 0.0) {
    fx2 = fx/masstotal2;
    fy2 = fy/masstotal2;
    fz2 = fz/masstotal2;
  } else fx2 = fy2 = fz2 = 0.0;

  if (masstotal > 0.0) {
    fx /= masstotal;
    fy /= masstotal;
    fz /= masstotal;
  } else fx = fy = fz = 0.0;

  // apply restoring force to atoms in group
  // f = -k*(r-r0)*mass/masstotal

  double **f = atom->f;
  int *mask = atom->mask;
  int *type = atom->type;
  double *mass = atom->mass;
  double *rmass = atom->rmass;
  int nlocal = atom->nlocal;

  double massone;

  if (rmass) {
    for (int i = 0; i < nlocal; i++) {
      if (mask[i] & groupbit) {
        massone = rmass[i];
        f[i][0] += fx*massone;
        f[i][1] += fy*massone;
        f[i][2] += fz*massone;
      }
      if (mask[i] & group2bit) {
        massone = rmass[i];
        f[i][0] -= fx2*massone;
        f[i][1] -= fy2*massone;
        f[i][2] -= fz2*massone;
      }
    }
  } else {
    for (int i = 0; i < nlocal; i++) {
      if (mask[i] & groupbit) {
        massone = mass[type[i]];
        f[i][0] += fx*massone;
        f[i][1] += fy*massone;
        f[i][2] += fz*massone;
      }
      if (mask[i] & group2bit) {
        massone = mass[type[i]];
        f[i][0] -= fx2*massone;
        f[i][1] -= fy2*massone;
        f[i][2] -= fz2*massone;
      }
    }
  }
}

/* ---------------------------------------------------------------------- */

void FixSpring::post_force_respa(int vflag, int ilevel, int /*iloop*/)
{
  if (ilevel == ilevel_respa) post_force(vflag);
}

/* ---------------------------------------------------------------------- */

void FixSpring::min_post_force(int vflag)
{
  post_force(vflag);
}

/* ----------------------------------------------------------------------
   energy of stretched spring
------------------------------------------------------------------------- */

double FixSpring::compute_scalar()
{
  return espring;
}

/* ----------------------------------------------------------------------
   return components of total spring force on fix group
------------------------------------------------------------------------- */

double FixSpring::compute_vector(int n)
{
  return ftotal[n];
}
