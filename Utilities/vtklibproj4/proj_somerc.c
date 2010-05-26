/*
** libproj -- library of cartographic projections
**
** Copyright (c) 2003, 2006   Gerald I. Evenden
*/
static const char
LIBPROJ_ID[] = "$Id: proj_somerc.c,v 1.1 2008-11-07 16:41:16 jeff Exp $";
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
  double  kR; \
  void *en, *en2;
#define PROJ_LIB__
#include  <lib_proj.h>
PROJ_HEAD(somerc, "Swiss. Obl. Mercator") "\n\tCyl, Ell\n\tFor CH1903";
#define EPS  1.e-10
#define NITER 6
FORWARD(e_forward);

  lp = proj_translate(proj_gauss(lp, P->en), P->en2);
  xy.x = P->kR * lp.lam;
  xy.y = P->kR * log(tan(FORTPI + 0.5 * lp.phi));
  return (xy);
}
INVERSE(e_inverse); /* ellipsoid & spheroid */

  lp.phi = 2. * (atan(exp(xy.y / P->kR)) - FORTPI);
  lp.lam = xy.x / P->kR;
  return (proj_inv_gauss(proj_inv_translate(lp, P->en2), P->en));
}
FREEUP; if (P){
  if (P->en) free(P->en);
  if (P->en2) free(P->en2);
  free(P);
  }
}
ENTRY2(somerc, en, en2)
  double phip0, Rc;

  if (!(P->en = proj_gauss_ini(P->e, P->phi0, &phip0, &Rc))) E_ERROR_0;
  if (!(P->en2 = proj_translate_ini(HALFPI - phip0, 0.))) E_ERROR_0;
  P->kR = P->k0 * Rc;
  P->inv = e_inverse;
  P->fwd = e_forward;
ENDENTRY(P)
/*
** $Log: proj_somerc.c,v $
** Revision 1.1  2008-11-07 16:41:16  jeff
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
