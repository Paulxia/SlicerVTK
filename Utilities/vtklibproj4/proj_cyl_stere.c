/*
** libproj -- library of cartographic projections
**
** Copyright (c) 2003, 2006   Gerald I. Evenden
*/
static const char
LIBPROJ_ID[] = "$Id: proj_cyl_stere.c,v 1.1 2008-11-07 16:41:14 jeff Exp $";
/*
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#define PROJ_PARMS__ \
  double xf, yf; \
  double rxf, ryf;
#define PROJ_LIB__
#include  <lib_proj.h>
PROJ_HEAD(cyl_stere, "Cylindrical Stereographic") "\n\tCyl, Sph\n\tlat_0=0";
FORWARD(s_forward); /* spheroid */
  xy.x = P->xf * lp.lam;
  xy.y = P->yf * tan(.5 * lp.phi);
  return (xy);
}
INVERSE(s_inverse); /* spheroid */
  lp.lam = P->rxf * xy.x;
  lp.phi = 2. * atan(xy.y * P->ryf);
  return (lp);
}
FREEUP; if (P) free(P); }
ENTRY0(cyl_stere)

  if ((P->xf = cos(P->phi0)) <= 0.) E_ERROR(-24);
  P->ryf = 1. / (P->yf = P->xf + 1.);
  P->rxf = 1. / P->xf;
  P->es = 0.;
  P->inv = s_inverse;
  P->fwd = s_forward;
ENDENTRY(P)
/*
** $Log: proj_cyl_stere.c,v $
** Revision 1.1  2008-11-07 16:41:14  jeff
** ENH: Adding a 2D geoview. Adding the geographic projection library libproj4
** to Utilities. Updating the architecture of the geospatial views. All
** multi-resolution sources are now subclasses of vtkGeoSource. Each source
** has its own worker thread for fetching refined images or geometry.
** On the 3D side, vtkGeoGlobeSource is an appropriate source for vtkGeoTerrain,
** and vtkGeoAlignedImageSource is an appropriate source for
** vtkGeoAlignedImageRepresentation. On the 2D side, vtkGeoProjectionSource is an
** appropriate source for vtkGeoTerrain2D, and the image source is the same.
**
** Revision 3.1  2006/01/11 01:38:18  gie
** Initial
**
*/
