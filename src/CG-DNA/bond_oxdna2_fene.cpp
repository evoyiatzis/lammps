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
   Contributing author: Oliver Henrich (University of Strathclyde, Glasgow)
------------------------------------------------------------------------- */

#include "bond_oxdna2_fene.h"
#include "constants_oxdna.h"

using namespace LAMMPS_NS;

/* ----------------------------------------------------------------------
    compute vector COM-sugar-phosphate backbone interaction site in oxDNA2
------------------------------------------------------------------------- */
void BondOxdna2Fene::compute_interaction_sites(double e1[3], double e2[3], double /*e3*/[3],
                                               double r[3]) const
{
  double d_cs_x = ConstantsOxdna::get_d_cs_x();
  double d_cs_y = ConstantsOxdna::get_d_cs_y();

  r[0] = d_cs_x * e1[0] + d_cs_y * e2[0];
  r[1] = d_cs_x * e1[1] + d_cs_y * e2[1];
  r[2] = d_cs_x * e1[2] + d_cs_y * e2[2];
}
