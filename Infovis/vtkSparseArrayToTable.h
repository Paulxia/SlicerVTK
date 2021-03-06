/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSparseArrayToTable.h
  
-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkSparseArrayToTable_h
#define __vtkSparseArrayToTable_h

#include "vtkTableAlgorithm.h"

// .NAME vtkSparseArrayToTable - Converts a sparse array to a vtkTable.
//
// .SECTION Description
// Converts any sparse array to a vtkTable containing one row for each value
// stored in the array.  The table will contain one column of coordinates for each
// dimension in the source array, plus one column of array values.  A common use-case
// for vtkSparseArrayToTable would be converting a sparse array into a table
// suitable for use as an input to vtkTableToGraph.
//
// The coordinate columns in the output table will be named using the dimension labels
// from the source array,  The value column name can be explicitly set using
// SetValueColumn().
//
// .SECTION Thanks
// Developed by Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories.

class VTK_INFOVIS_EXPORT vtkSparseArrayToTable : public vtkTableAlgorithm
{
public:
  static vtkSparseArrayToTable* New();
  vtkTypeRevisionMacro(vtkSparseArrayToTable, vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the name of the output table column that contains array values.
  // Default: "value"
  vtkGetStringMacro(ValueColumn);
  vtkSetStringMacro(ValueColumn);

protected:
  vtkSparseArrayToTable();
  ~vtkSparseArrayToTable();

  int FillInputPortInformation(int, vtkInformation*);

  int RequestData(
    vtkInformation*, 
    vtkInformationVector**, 
    vtkInformationVector*);

  char* ValueColumn;

private:
  vtkSparseArrayToTable(const vtkSparseArrayToTable&); // Not implemented
  void operator=(const vtkSparseArrayToTable&);   // Not implemented
};

#endif

