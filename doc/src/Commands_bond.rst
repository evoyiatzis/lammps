.. _bond:

Bond styles
===========

All LAMMPS :doc:`bond_style <bond_style>` commands.  Some styles have
accelerated versions.  This is indicated by additional letters in
parenthesis: g = GPU, i = INTEL, k = KOKKOS, o = OPENMP, t =
OPT.

.. table_from_list::
   :columns: 5

   * :doc:`none <bond_none>`
   * :doc:`zero <bond_zero>`
   * :doc:`hybrid (k) <bond_hybrid>`
   *
   *
   *
   *
   *
   *
   *
   * :doc:`bpm/rotational <bond_bpm_rotational>`
   * :doc:`bpm/spring <bond_bpm_spring>`
   * :doc:`bpm/spring/plastic <bond_bpm_spring_plastic>`
   * :doc:`class2 (ko) <bond_class2>`
   * :doc:`fene (iko) <bond_fene>`
   * :doc:`fene/expand (o) <bond_fene_expand>`
   * :doc:`fene/nm <bond_fene>`
   * :doc:`gaussian <bond_gaussian>`
   * :doc:`gromos (o) <bond_gromos>`
   * :doc:`harmonic (iko) <bond_harmonic>`
   * :doc:`harmonic/restrain <bond_harmonic_restrain>`
   * :doc:`harmonic/shift (o) <bond_harmonic_shift>`
   * :doc:`harmonic/shift/cut (o) <bond_harmonic_shift_cut>`
   * :doc:`lepton (o) <bond_lepton>`
   * :doc:`mesocnt <bond_mesocnt>`
   * :doc:`mm3 <bond_mm3>`
   * :doc:`morse (o) <bond_morse>`
   * :doc:`nonlinear (o) <bond_nonlinear>`
   * :doc:`oxdna/fene <bond_oxdna>`
   * :doc:`oxdna2/fene <bond_oxdna>`
   * :doc:`oxrna2/fene <bond_oxdna>`
   * :doc:`quartic (o) <bond_quartic>`
   * :doc:`rheo/shell <bond_rheo_shell>`
   * :doc:`special <bond_special>`
   * :doc:`table (o) <bond_table>`

.. _angle:

Angle styles
============

All LAMMPS :doc:`angle_style <angle_style>` commands.  Some styles have
accelerated versions.  This is indicated by additional letters in
parenthesis: g = GPU, i = INTEL, k = KOKKOS, o = OPENMP, t =
OPT.

.. table_from_list::
   :columns: 5

   * :doc:`none <angle_none>`
   * :doc:`zero <angle_zero>`
   * :doc:`hybrid (k) <angle_hybrid>`
   *
   *
   *
   *
   *
   *
   *
   * :doc:`amoeba <angle_amoeba>`
   * :doc:`charmm (iko) <angle_charmm>`
   * :doc:`class2 (ko) <angle_class2>`
   * :doc:`class2/p6 <angle_class2>`
   * :doc:`cosine (ko) <angle_cosine>`
   * :doc:`cosine/buck6d <angle_cosine_buck6d>`
   * :doc:`cosine/delta (o) <angle_cosine_delta>`
   * :doc:`cosine/periodic (o) <angle_cosine_periodic>`
   * :doc:`cosine/shift (o) <angle_cosine_shift>`
   * :doc:`cosine/shift/exp (o) <angle_cosine_shift_exp>`
   * :doc:`cosine/squared (o) <angle_cosine_squared>`
   * :doc:`cosine/squared/restricted (o) <angle_cosine_squared_restricted>`
   * :doc:`cross <angle_cross>`
   * :doc:`dipole (o) <angle_dipole>`
   * :doc:`fourier (o) <angle_fourier>`
   * :doc:`fourier/simple (o) <angle_fourier_simple>`
   * :doc:`gaussian <angle_gaussian>`
   * :doc:`harmonic (iko) <angle_harmonic>`
   * :doc:`lepton (o) <angle_lepton>`
   * :doc:`mesocnt <angle_mesocnt>`
   * :doc:`mm3 <angle_mm3>`
   * :doc:`mwlc <angle_mwlc>`
   * :doc:`quartic (o) <angle_quartic>`
   * :doc:`spica (ko) <angle_spica>`
   * :doc:`table (o) <angle_table>`

.. _dihedral:

Dihedral styles
===============

All LAMMPS :doc:`dihedral_style <dihedral_style>` commands.  Some styles
have accelerated versions.  This is indicated by additional letters in
parenthesis: g = GPU, i = INTEL, k = KOKKOS, o = OPENMP, t =
OPT.

.. table_from_list::
   :columns: 5

   * :doc:`none <dihedral_none>`
   * :doc:`zero <dihedral_zero>`
   * :doc:`hybrid (k) <dihedral_hybrid>`
   *
   *
   *
   *
   *
   *
   *
   * :doc:`charmm (iko) <dihedral_charmm>`
   * :doc:`charmmfsw (k) <dihedral_charmm>`
   * :doc:`class2 (ko) <dihedral_class2>`
   * :doc:`cosine/shift/exp (o) <dihedral_cosine_shift_exp>`
   * :doc:`cosine/squared/restricted <dihedral_cosine_squared_restricted>`
   * :doc:`fourier (io) <dihedral_fourier>`
   * :doc:`harmonic (iko) <dihedral_harmonic>`
   * :doc:`helix (o) <dihedral_helix>`
   * :doc:`lepton (o) <dihedral_lepton>`
   * :doc:`multi/harmonic (ko) <dihedral_multi_harmonic>`
   * :doc:`nharmonic (o) <dihedral_nharmonic>`
   * :doc:`opls (iko) <dihedral_opls>`
   * :doc:`quadratic (o) <dihedral_quadratic>`
   * :doc:`spherical <dihedral_spherical>`
   * :doc:`table (o) <dihedral_table>`
   * :doc:`table/cut <dihedral_table>`

.. _improper:

Improper styles
===============

All LAMMPS :doc:`improper_style <improper_style>` commands.  Some styles
have accelerated versions.  This is indicated by additional letters in
parenthesis: g = GPU, i = INTEL, k = KOKKOS, o = OPENMP, t =
OPT.

.. table_from_list::
   :columns: 5

   * :doc:`none <improper_none>`
   * :doc:`zero <improper_zero>`
   * :doc:`hybrid (k) <improper_hybrid>`
   *
   *
   *
   *
   *
   *
   *
   * :doc:`amoeba <improper_amoeba>`
   * :doc:`class2 (ko) <improper_class2>`
   * :doc:`cossq (o) <improper_cossq>`
   * :doc:`cvff (io) <improper_cvff>`
   * :doc:`distance <improper_distance>`
   * :doc:`distharm <improper_distharm>`
   * :doc:`fourier (o) <improper_fourier>`
   * :doc:`harmonic (iko) <improper_harmonic>`
   * :doc:`inversion/harmonic <improper_inversion_harmonic>`
   * :doc:`ring (o) <improper_ring>`
   * :doc:`sqdistharm <improper_sqdistharm>`
   * :doc:`umbrella (o) <improper_umbrella>`
