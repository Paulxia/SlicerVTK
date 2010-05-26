/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: SimpleView.h,v $
  Language:  C++
  Date:      $Date: 2009-03-25 13:56:11 $
  Version:   $Revision: 1.3 $

  Copyright 2009 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/
#ifndef SimpleView_H
#define SimpleView_H

#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.
#include <QMainWindow>

// Forward Qt class declarations
class Ui_SimpleView;

// Forward VTK class declarations
class vtkQtTableView;


class SimpleView : public QMainWindow
{
  Q_OBJECT

public:

  // Constructor/Destructor
  SimpleView(); 
  ~SimpleView();

public slots:

  virtual void slotOpenFile();
  virtual void slotExit();

protected:
   
protected slots:

private:

  vtkSmartPointer<vtkQtTableView>         TableView;
    
  // Designer form
  Ui_SimpleView *ui;
};

#endif // SimpleView_H
