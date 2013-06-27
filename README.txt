 * \version 1.0
 * \date 17th June 2013
 * \author Colin Myers
 * School of Computing
 * University of Leeds
 * UK
 * www.comp.leeds.ac.uk/csm
 * csm@comp.leeds.ac.uk
 *
 * DESCRIPTION
 * CoronaScope is a graph visualization tool built on VTK, that provides tools for
 * highlighting landmarks, or graph annotations. These annotations can then
 * be used to provide navigational hints by way of a widget that shows the position
 * of landmarks that are no longer visible on the screen after pan/zoom. The widget
 * is interactive and can be used to automatically pan along edges to reach a
 * selected off-screen landmark.
 *
 * This package contains a set of libraries arranged in a similar structure to VTK
 * (e.g. Graphics, Infovis, etc.). The libraries contain filters that implement
 * the landmarks and off-screen widget functions described above. The examples
 * folder contains minimal examples of how to use the new classes as well as
 * "CoronaScope", a desktop graph visualization application built using Qt.
 * CoronaScope implements the techniques described in my PhD thesis, "Navigating
 * Networks using Overlays".
 *
 * DEPENDENCIES
 * == VTK-5.10.0 (http://www.vtk.org) built with the following flags enabled:
 *    VTK_USE_BOOST
 *    VTK_USE_GUISUPPORT
 *    VTK_USE_INFOVIS
 *    VTK_USE_VIEWS
 *    VTK_USE_QT
 *
 * >= Qt-4.6.0 (http://qt.digia.com)
 *
 * >= BOOST-1.40 (http://www.boost.org)
 *
 * == OGDF v.2012.07 (http://www.ogdf.net/doku.php) [OPTIONAL]
 *
 * INSTALLATION (Linux)
 * 0. Download the source from: <a href="#">here</a>
 * 1. tar xvf vtkCoronaScope.tar.gz
 * 2. $ cd vtkCoronaScope
 * 3. $ mkdir Build
 * 4. $ cd Build
 * 5. $ ccmake ../Source/.
 * If not found automatically, you may need to enter the paths to dependencies.
 * Be sure to set "Build Examples" to ON.
 * If you have OGDF installed, set "Use OGDF" to ON.
 * 6. $ make
 *
 * To run:
 * 1. $ cd Build/bin
 * 2. ./CoronaScope
 *
 * KNOWN ISSUES
 * Setting edge or vertex labels to "None" when using vtkConeTree layout causes
 * CoronaScope to crash.
 * 
 */
