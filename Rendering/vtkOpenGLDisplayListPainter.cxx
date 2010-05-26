/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOpenGLDisplayListPainter.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkOpenGLDisplayListPainter.h"

#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTimerLog.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
#  include "vtkOpenGL.h"
#endif

#include <vtkstd/map>

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkStandardNewMacro(vtkOpenGLDisplayListPainter);
#endif

class vtkOpenGLDisplayListPainter::vtkInternals
{
public:
  typedef vtkstd::map<unsigned long, GLuint> DisplayListMapType;
  DisplayListMapType DisplayListMap;

  // Refers to the build time of the first display list.
  vtkTimeStamp BuildTime;

  void ReleaseAllLists(vtkWindow* win)
    {
    // Checking is win->GetMapped() is causing segfaults on AIX.
    if (win /*&& win->GetMapped()*/)
      {
      DisplayListMapType::iterator iter;
      for (iter = this->DisplayListMap.begin(); iter != this->DisplayListMap.end();
        iter++)
        {
        glDeleteLists(iter->second, 1);
        }
      }
    this->DisplayListMap.clear();
    }

  void ReleaseList(unsigned long key)
    {
    DisplayListMapType::iterator iter = this->DisplayListMap.find(key);
    if (iter != this->DisplayListMap.end())
      {
      glDeleteLists(iter->second, 1);
      this->DisplayListMap.erase(iter);
      }
    }

  void UpdateBuildTime()
    {
    if (this->DisplayListMap.size() == 1)
      {
      this->BuildTime.Modified();
      }
    }

};

//-----------------------------------------------------------------------------
vtkOpenGLDisplayListPainter::vtkOpenGLDisplayListPainter()
{
  this->Internals = new vtkInternals();
}

//-----------------------------------------------------------------------------
vtkOpenGLDisplayListPainter::~vtkOpenGLDisplayListPainter()
{
  if (this->LastWindow)
    {
    this->ReleaseGraphicsResources(this->LastWindow);
    }
  delete this->Internals;
  this->Internals = 0;
}

//-----------------------------------------------------------------------------
void vtkOpenGLDisplayListPainter::ReleaseGraphicsResources(vtkWindow* win)
{
  if (win && win->GetMapped())
    {
    win->MakeCurrent();
    this->Internals->ReleaseAllLists(win);
    }
  this->Internals->DisplayListMap.clear();
  this->Superclass::ReleaseGraphicsResources(win);
  this->LastWindow = NULL;
}

//-----------------------------------------------------------------------------
void vtkOpenGLDisplayListPainter::RenderInternal(vtkRenderer *renderer,
                                                 vtkActor *actor, 
                                                 unsigned long typeflags,
                                                 bool forceCompileOnly)
{
  if (this->GetMTime() > this->Internals->BuildTime ||
    (this->LastWindow && (renderer->GetRenderWindow() != this->LastWindow.GetPointer())))
    {
    // MTime changes when input changes or someother iVar changes, so display
    // lists are obsolete so we can let go of them.
    this->ReleaseGraphicsResources(this->LastWindow);
    renderer->GetRenderWindow()->MakeCurrent();
    }

  if (this->ImmediateModeRendering)
    {
    // don't use display lists at all.
    if (!forceCompileOnly)
      {
      this->Superclass::RenderInternal(renderer, actor, typeflags,
        forceCompileOnly);
      }
    return;
    }

  this->TimeToDraw = 0.0;

  vtkDataObject* input = this->GetInput();
  // if something has changed regenrate display lists.

  // First check for the cases where all display lists (irrespective of
  // typeflags are obsolete.
  if (
    // Since input changed
    input->GetMTime() > this->Internals->BuildTime  ||
    // actor's properties were modified
    actor->GetProperty()->GetMTime() > this->Internals->BuildTime ||
    // mapper information was modified
    this->Information->GetMTime() > this->Internals->BuildTime)
    {
    this->Internals->ReleaseAllLists(this->LastWindow);
    this->LastWindow = 0;
    }

  vtkInternals::DisplayListMapType::iterator iter = 
    this->Internals->DisplayListMap.find(typeflags);
  if (iter == this->Internals->DisplayListMap.end())
    {
    GLuint list = glGenLists(1);
    
    glNewList(list, GL_COMPILE);
    // generate the display list.
    this->Superclass::RenderInternal(renderer, actor, typeflags,
                                     forceCompileOnly);
    glEndList();

    this->Internals->DisplayListMap[typeflags] = list;
    this->Internals->UpdateBuildTime();

    this->LastWindow = renderer->GetRenderWindow();
    iter = this->Internals->DisplayListMap.find(typeflags);
    }

  if (!forceCompileOnly)
    {
    // Time the actual drawing.
    this->Timer->StartTimer();
    // render the display list.
    // if nothing has changed we use an old display list else
    // we use the newly generated list.
    glCallList(iter->second);
    // glFinish(); // To compute time correctly, we need to wait 
    // till OpenGL finishes.
    this->Timer->StopTimer();
    this->TimeToDraw += this->Timer->GetElapsedTime();
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGLDisplayListPainter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
