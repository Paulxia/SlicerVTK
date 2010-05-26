/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageSeedConnectivity.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageSeedConnectivity - SeedConnectivity with user defined seeds.
// .SECTION Description
// vtkImageSeedConnectivity marks pixels connected to user supplied seeds.
// The input must be unsigned char, and the output is also unsigned char.  If
// a seed supplied by the user does not have pixel value "InputTrueValue",
// then the image is scanned +x, +y, +z until a pixel is encountered with
// value "InputTrueValue".  This new pixel is used as the seed .  Any pixel
// with out value "InputTrueValue" is consider off.  The output pixels values
// are 0 for any off pixel in input, "OutputTrueValue" for any pixels
// connected to seeds, and "OutputUnconnectedValue" for any on pixels not
// connected to seeds.  The same seeds are used for all images in the image
// set.

#ifndef __vtkImageSeedConnectivity_h
#define __vtkImageSeedConnectivity_h

#include "vtkImageAlgorithm.h"

class vtkImageConnector;
class vtkImageConnectorSeed;

class VTK_IMAGING_EXPORT vtkImageSeedConnectivity : public vtkImageAlgorithm
{
public:
  static vtkImageSeedConnectivity *New();
  vtkTypeMacro(vtkImageSeedConnectivity,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Methods for manipulating the seed pixels.
  void RemoveAllSeeds();
  void AddSeed(int num, int *index);
  void AddSeed(int i0, int i1, int i2);
  void AddSeed(int i0, int i1);

  // Description:
  // Set/Get what value is considered as connecting pixels.
  vtkSetMacro(InputConnectValue, int);
  vtkGetMacro(InputConnectValue, int);

  // Description:
  // Set/Get the value to set connected pixels to.
  vtkSetMacro(OutputConnectedValue, int);
  vtkGetMacro(OutputConnectedValue, int);

  // Description:
  // Set/Get the value to set unconnected pixels to.
  vtkSetMacro(OutputUnconnectedValue, int);
  vtkGetMacro(OutputUnconnectedValue, int);
  
  // Description:
  // Get the vtkImageCOnnector used by this filter.
  vtkGetObjectMacro(Connector,vtkImageConnector);

  // Description:
  // Set the number of axes to use in connectivity.
  vtkSetMacro(Dimensionality,int);
  vtkGetMacro(Dimensionality,int);
  
protected:
  vtkImageSeedConnectivity();
  ~vtkImageSeedConnectivity();

  unsigned char InputConnectValue;
  unsigned char OutputConnectedValue;
  unsigned char OutputUnconnectedValue;
  vtkImageConnectorSeed *Seeds;
  vtkImageConnector *Connector;
  int Dimensionality;
  
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkImageSeedConnectivity(const vtkImageSeedConnectivity&);  // Not implemented.
  void operator=(const vtkImageSeedConnectivity&);  // Not implemented.
};



#endif


  
