/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   LAMMPS development team: developers@lammps.org

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#ifdef COMPUTE_CLASS
// clang-format off
ComputeStyle(global/atom,ComputeGlobalAtom);
// clang-format on
#else

#ifndef LMP_COMPUTE_GLOBAL_ATOM_H
#define LMP_COMPUTE_GLOBAL_ATOM_H

#include "compute.h"

namespace LAMMPS_NS {

class ComputeGlobalAtom : public Compute {
 public:
  ComputeGlobalAtom(class LAMMPS *, int, char **);
  ~ComputeGlobalAtom() override;
  void init() override;
  void compute_peratom() override;
  double memory_usage() override;

 protected:
  struct value_t {
    int which;
    int argindex;
    int iarg;
    std::string id;
    union {
      class Compute *c;
      class Fix *f;
      int v;
    } val;
  };
  std::vector<value_t> values;
  value_t reference;

  int nmax, maxvector;
  int *indices;
  double *varatom;
  double *vecglobal;
};

}    // namespace LAMMPS_NS

#endif
#endif
