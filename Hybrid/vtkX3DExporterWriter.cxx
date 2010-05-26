/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkX3DExporterWriter.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkX3DExporterWriter.h"

#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkX3DExporterWriter, "$Revision: 1.1 $");
//----------------------------------------------------------------------------
vtkX3DExporterWriter::vtkX3DExporterWriter()
{
}

//----------------------------------------------------------------------------
vtkX3DExporterWriter::~vtkX3DExporterWriter()
{
}

//----------------------------------------------------------------------------
void vtkX3DExporterWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

