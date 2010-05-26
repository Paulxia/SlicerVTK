/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkXMLMultiGroupDataReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXMLMultiGroupDataReader.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkXMLMultiGroupDataReader);
vtkCxxRevisionMacro(vtkXMLMultiGroupDataReader, "$Revision: 1.5 $");
//----------------------------------------------------------------------------
vtkXMLMultiGroupDataReader::vtkXMLMultiGroupDataReader()
{
}

//----------------------------------------------------------------------------
vtkXMLMultiGroupDataReader::~vtkXMLMultiGroupDataReader()
{
}

//----------------------------------------------------------------------------
void vtkXMLMultiGroupDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

