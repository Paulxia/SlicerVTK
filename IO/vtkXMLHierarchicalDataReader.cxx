/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkXMLHierarchicalDataReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXMLHierarchicalDataReader.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkXMLHierarchicalDataReader);
vtkCxxRevisionMacro(vtkXMLHierarchicalDataReader, "$Revision: 1.6 $");
//----------------------------------------------------------------------------
vtkXMLHierarchicalDataReader::vtkXMLHierarchicalDataReader()
{
}

//----------------------------------------------------------------------------
vtkXMLHierarchicalDataReader::~vtkXMLHierarchicalDataReader()
{
}

//----------------------------------------------------------------------------
void vtkXMLHierarchicalDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

