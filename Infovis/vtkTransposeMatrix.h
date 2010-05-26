/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkTransposeMatrix.h,v $
  
-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkTransposeMatrix_h
#define __vtkTransposeMatrix_h

#include "vtkArrayDataAlgorithm.h"

// .NAME vtkTransposeMatrix - Computes the transpose of an input matrix.
//
// .SECTION Thanks
// Developed by Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories.

class VTK_INFOVIS_EXPORT vtkTransposeMatrix : public vtkArrayDataAlgorithm
{
public:
  static vtkTransposeMatrix* New();
  vtkTypeRevisionMacro(vtkTransposeMatrix, vtkArrayDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkTransposeMatrix();
  ~vtkTransposeMatrix();

  int RequestData(
    vtkInformation*, 
    vtkInformationVector**, 
    vtkInformationVector*);

private:
  vtkTransposeMatrix(const vtkTransposeMatrix&); // Not implemented
  void operator=(const vtkTransposeMatrix&);   // Not implemented
};

#endif

