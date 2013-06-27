#ifndef __CoronaScopeDocMain_h
#define __CoronaScopeDocMain_h

//-------------------------------------------------------------------------
// This file is here solely to provide the mainpage documentation.
// ------------------------------------------------------------------------

/*! \mainpage CoronaScope API Reference
 *
 * \version 1.0
 * \date 17th June 2013
 * \author Colin Myers<br>
 * School of Computing<br>
 * University of Leeds<br>
 * UK<br>
 * www.comp.leeds.ac.uk/csm<br>
 * csm@comp.leeds.ac.uk
 *
 * \section description_sec Description
 * <p>CoronaScope is a graph visualization tool built on VTK, that provides tools for
 * highlighting <i>landmarks</i>, or graph annotations. These annotations can then
 * be used to provide navigational hints by way of a widget that shows the position
 * of landmarks that are no longer visible on the screen after pan/zoom. The widget
 * is interactive and can be used to automatically pan along edges to reach a
 * selected off-screen landmark.</p>
 *
 * <p>This package contains a set of libraries arranged in a similar structure to VTK
 * (e.g. Graphics, Infovis, etc.). The libraries contain filters that implement
 * the landmarks and off-screen widget functions described above. The examples
 * folder contains minimal examples of how to use the new classes as well as
 * "CoronaScope", a desktop graph visualization application built using Qt.
 * CoronaScope implements the techniques described in my PhD thesis, "Navigating
 * Networks using Overlays".</p>
 *
 * \section deps_sec Dependencies
 * == VTK-5.10.0 (http://www.vtk.org) built with the following flags enabled:<br>
 *    VTK_USE_BOOST<br>
 *    VTK_USE_GUISUPPORT<br>
 *    VTK_USE_INFOVIS<br>
 *    VTK_USE_VIEWS<br>
 *    VTK_USE_QT<br>
 *<br>
 * >= Qt-4.6.0 (http://qt.digia.com)<br>
 *<br>
 * >= BOOST-1.40 (http://www.boost.org)<br>
 *<br>
 * == OGDF v.2012.07 (http://www.ogdf.net/doku.php) [OPTIONAL]<br>
 *
 * \section install_sec Installation (Linux)
 * 0. Download the source from: <a href="#">here</a><br>
 * 1. tar xvf vtkCoronaScope.tar.gz<br>
 * 2. $ cd vtkCoronaScope<br>
 * 3. $ mkdir Build<br>
 * 4. $ cd Build<br>
 * 5. $ ccmake ../Source/.<br>
 * If not found automatically, you may need to enter the paths to dependencies.<br>
 * Be sure to set "Build Examples" to ON.<br>
 * If you have OGDF installed, set "Use OGDF" to ON.<br>
 * 6. $ make<br>
 *<br>
 * To run:<br>
 * 1. $ cd Build/bin<br>
 * 2. ./CoronaScope<br>
 *
 * \section issues_sec Known Issues
 * Setting edge or vertex labels to "None" when using vtkConeTree layout causes
 * CoronaScope to crash.
 *
 */

#endif // __CoronaScopeDocMain_h
