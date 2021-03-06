/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkContextMapper2D.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkContextMapper2D.h"

#include "vtkTable.h"
#include "vtkInformation.h"
#include "vtkExecutive.h"

#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkContextMapper2D, "1.1");
vtkStandardNewMacro(vtkContextMapper2D);
//-----------------------------------------------------------------------------
vtkContextMapper2D::vtkContextMapper2D()
{
  // We take 1 input and no outputs
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(0);
}

//-----------------------------------------------------------------------------
vtkContextMapper2D::~vtkContextMapper2D()
{
}

//----------------------------------------------------------------------------
void vtkContextMapper2D::SetInput(vtkTable *input)
{
  if(input)
    {
    vtkDebugMacro(<< "Input table set.");
    this->SetInputConnection(0, input->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    vtkDebugMacro(<< "Null input table set.");
    this->SetInputConnection(0, 0);
    }
}

//----------------------------------------------------------------------------
vtkTable * vtkContextMapper2D::GetInput()
{
  return vtkTable::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
}

//-----------------------------------------------------------------------------
int vtkContextMapper2D::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
  return 1;
}


//-----------------------------------------------------------------------------
void vtkContextMapper2D::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
