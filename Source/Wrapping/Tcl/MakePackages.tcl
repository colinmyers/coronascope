#!/usr/bin/env tclsh

# Execute this script each time you add a new directory or file.

# Packages

pkg_mkIndex -direct -verbose vtkcsm
pkg_mkIndex -direct -verbose vtkcsmcommon
pkg_mkIndex -direct -verbose vtkcsmimaging
pkg_mkIndex -direct -verbose vtkcsmunsorted

exit