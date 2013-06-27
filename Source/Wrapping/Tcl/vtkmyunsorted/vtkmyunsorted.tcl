package require vtkcsmimaging
package require vtk

#
# Here you should pick the name of one your imaging local classes
# instead of vtkBar2.
#

if {[info commands vtkBar2] != "" ||
    [::vtk::load_component vtkcsmUnsortedTCL] == ""} {
    package provide vtkcsmunsorted 4.0
}
