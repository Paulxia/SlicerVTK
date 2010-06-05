/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVolumetricPass.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkVolumetricPass.h"
#include "vtkObjectFactory.h"
#include <assert.h>

vtkCxxRevisionMacro(vtkVolumetricPass, "1.1");
vtkStandardNewMacro(vtkVolumetricPass);

// ----------------------------------------------------------------------------
vtkVolumetricPass::vtkVolumetricPass()
{
}

// ----------------------------------------------------------------------------
vtkVolumetricPass::~vtkVolumetricPass()
{
}

// ----------------------------------------------------------------------------
void vtkVolumetricPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// ----------------------------------------------------------------------------
// Description:
// Perform rendering according to a render state \p s.
// \pre s_exists: s!=0
void vtkVolumetricPass::Render(const vtkRenderState *s)
{
  assert("pre: s_exists" && s!=0);
  
  this->NumberOfRenderedProps=0;
  this->RenderFilteredVolumetricGeometry(s);
}
