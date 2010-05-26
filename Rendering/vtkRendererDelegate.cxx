/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkRendererDelegate.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkRendererDelegate.h"

vtkCxxRevisionMacro(vtkRendererDelegate, "$Revision: 1.1 $");

vtkRendererDelegate::vtkRendererDelegate()
{
  this->Used=false;
}

vtkRendererDelegate::~vtkRendererDelegate()
{
}

void vtkRendererDelegate::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Used: ";
  if(this->Used)
    {
      os<<"On";
    }
  else
    {
      os<<"Off";
    }
  os<<endl;
}
