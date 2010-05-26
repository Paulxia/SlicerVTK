/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkQtView.cxx,v $

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2009 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkQtView.h"

#include <QApplication>
#include <QPixmap>
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkQtView, "$Revision: 1.5 $");

//----------------------------------------------------------------------------
vtkQtView::vtkQtView()
{

}

//----------------------------------------------------------------------------
vtkQtView::~vtkQtView()
{

}

//----------------------------------------------------------------------------
void vtkQtView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkQtView::ProcessQtEvents()
{
  QApplication::processEvents();
}

//----------------------------------------------------------------------------
void vtkQtView::ProcessQtEventsNoUserInput()
{
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

//----------------------------------------------------------------------------
bool vtkQtView::SaveImage(const char* filename)
{
  // This is ok even if this->GetWidget() returns null.
  return QPixmap::grabWidget(this->GetWidget()).save(filename);
}
