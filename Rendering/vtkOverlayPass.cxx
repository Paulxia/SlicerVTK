/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOverlayPass.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkOverlayPass.h"
#include "vtkObjectFactory.h"
#include <assert.h>

vtkCxxRevisionMacro(vtkOverlayPass, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkOverlayPass);

// ----------------------------------------------------------------------------
vtkOverlayPass::vtkOverlayPass()
{
}

// ----------------------------------------------------------------------------
vtkOverlayPass::~vtkOverlayPass()
{
}

// ----------------------------------------------------------------------------
void vtkOverlayPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

// ----------------------------------------------------------------------------
// Description:
// Perform rendering according to a render state \p s.
// \pre s_exists: s!=0
void vtkOverlayPass::Render(const vtkRenderState *s)
{
  assert("pre: s_exists" && s!=0);
  
  this->NumberOfRenderedProps=0;
  this->RenderFilteredOverlay(s);
}
