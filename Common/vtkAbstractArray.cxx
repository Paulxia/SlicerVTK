/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAbstractArray.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAbstractArray.h"

#include "vtkBitArray.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkShortArray.h"
#include "vtkSignedCharArray.h"
#include "vtkStringArray.h"
#include "vtkUnicodeStringArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkVariantArray.h"

#if defined(VTK_TYPE_USE_LONG_LONG)
# include "vtkLongLongArray.h"
# include "vtkUnsignedLongLongArray.h"
#endif

#if defined(VTK_TYPE_USE___INT64)
# include "vtk__Int64Array.h"
# if defined(VTK_TYPE_CONVERT_UI64_TO_DOUBLE)
#  include "vtkUnsigned__Int64Array.h"
# endif
#endif

vtkCxxRevisionMacro(vtkAbstractArray, "1.17");

//----------------------------------------------------------------------------
// Construct object with sane defaults.
vtkAbstractArray::vtkAbstractArray(vtkIdType vtkNotUsed(numComp))
{
  this->Size = 0;
  this->MaxId = -1;
  this->NumberOfComponents = 1;
  this->Name = NULL;
  this->Information = NULL;
}

//----------------------------------------------------------------------------
vtkAbstractArray::~vtkAbstractArray()
{
  this->SetName(NULL);
  this->SetInformation(NULL);
}

//----------------------------------------------------------------------------
void vtkAbstractArray::SetInformation(vtkInformation *args)
{
  // Same as in vtkCxxSetObjectMacro, but no Modified() so that
  // this doesn't cause extra pipeline updates.
  vtkDebugMacro(<< this->GetClassName() << " (" << this
      << "): setting Information to " << args );
  if (this->Information != args)
    {
    vtkInformation* tempSGMacroVar = this->Information;
    this->Information = args;
    if (this->Information != NULL) { this->Information->Register(this); }
    if (tempSGMacroVar != NULL)
      {
      tempSGMacroVar->UnRegister(this);
      }
    }
}

//----------------------------------------------------------------------------
void vtkAbstractArray::GetTuples(vtkIdList* ptIds, vtkAbstractArray* aa)
{
  if (aa->GetNumberOfComponents() != this->GetNumberOfComponents())
    {
    vtkWarningMacro("Number of components for input and output do not match.");
    return;
    }
  // Here we give the slowest implementation. Subclasses can override
  // to use the knowledge about the data.
  vtkIdType num = ptIds->GetNumberOfIds();
  for (vtkIdType i = 0; i < num; i++)
    {
    aa->SetTuple(i, ptIds->GetId(i), this);
    }
}

//----------------------------------------------------------------------------
void vtkAbstractArray::GetTuples(vtkIdType p1, vtkIdType p2, 
  vtkAbstractArray* aa)
{
  if (aa->GetNumberOfComponents() != this->GetNumberOfComponents())
    {
    vtkWarningMacro("Number of components for input and output do not match.");
    return;
    }
  
  // Here we give the slowest implementation. Subclasses can override
  // to use the knowledge about the data.
  vtkIdType num = p2 - p1 + 1;
  for (vtkIdType i = 0; i < num; i++)
    {
    aa->SetTuple(i, (p1+i), this);
    }
}

//----------------------------------------------------------------------------
void vtkAbstractArray::DeepCopy( vtkAbstractArray* da )
{
  if (da && da->HasInformation() && da!=this)
    {
    this->CopyInformation(da->GetInformation(),/*deep=*/1);
    }
}

//----------------------------------------------------------------------------
int vtkAbstractArray::CopyInformation(vtkInformation *infoFrom, int deep)
{
  // Copy all keys. NOTE: subclasses rely on this.
  vtkInformation *myInfo=this->GetInformation();
  myInfo->Copy(infoFrom,deep);

  // remove any keys we own that are not to be coppied
  // here.

  return 1;
}

//----------------------------------------------------------------------------
vtkInformation* vtkAbstractArray::GetInformation()
{
  if ( ! this->Information )
    {
    vtkInformation* info = vtkInformation::New();
    this->SetInformation( info );
    info->FastDelete();
    }
  return this->Information;
}

//----------------------------------------------------------------------------
template <class T>
int vtkAbstractArrayGetDataTypeSize(T*)
{
  return sizeof(T);
}

int vtkAbstractArray::GetDataTypeSize(int type)
{
  switch (type)
    {
    vtkTemplateMacro(
      return vtkAbstractArrayGetDataTypeSize(static_cast<VTK_TT*>(0))
      );
      
    case VTK_BIT:
      return 0;
      break;

    case VTK_STRING:
      return 0;
      break;

    case VTK_UNICODE_STRING:
      return 0;
      break;

    default:
      vtkGenericWarningMacro(<<"Unsupported data type!");
    }
  
  return 1;
}

// ----------------------------------------------------------------------
vtkAbstractArray* vtkAbstractArray::CreateArray(int dataType)
{
  switch (dataType)
    {
    case VTK_BIT:
      return vtkBitArray::New();

    case VTK_CHAR:
      return vtkCharArray::New();

    case VTK_SIGNED_CHAR:
      return vtkSignedCharArray::New();

    case VTK_UNSIGNED_CHAR:
      return vtkUnsignedCharArray::New();

    case VTK_SHORT:
      return vtkShortArray::New();

    case VTK_UNSIGNED_SHORT:
      return vtkUnsignedShortArray::New();

    case VTK_INT:
      return vtkIntArray::New();

    case VTK_UNSIGNED_INT:
      return vtkUnsignedIntArray::New();

    case VTK_LONG:
      return vtkLongArray::New();

    case VTK_UNSIGNED_LONG:
      return vtkUnsignedLongArray::New();

#if defined(VTK_TYPE_USE_LONG_LONG)
    case VTK_LONG_LONG:
      return vtkLongLongArray::New();

    case VTK_UNSIGNED_LONG_LONG:
      return vtkUnsignedLongLongArray::New();
#endif

#if defined(VTK_TYPE_USE___INT64)
    case VTK___INT64:
      return vtk__Int64Array::New();
      break;

# if defined(VTK_TYPE_CONVERT_UI64_TO_DOUBLE)
    case VTK_UNSIGNED___INT64:
      return vtkUnsigned__Int64Array::New();
      break;
# endif
#endif

    case VTK_FLOAT:
      return vtkFloatArray::New();

    case VTK_DOUBLE:
      return vtkDoubleArray::New();

    case VTK_ID_TYPE:
      return vtkIdTypeArray::New();
   
    case VTK_STRING:
      return vtkStringArray::New();

    case VTK_UNICODE_STRING:
      return vtkUnicodeStringArray::New();

    case VTK_VARIANT:
      return vtkVariantArray::New();

    default:
      break;
    }

  vtkGenericWarningMacro("Unsupported data type: " << dataType
                         << "! Setting to VTK_DOUBLE");
  return vtkDoubleArray::New();
}

//---------------------------------------------------------------------------
template <typename T>
vtkVariant vtkAbstractArrayGetVariantValue(T* arr, vtkIdType index)
{
  return vtkVariant(arr[index]);
}

//----------------------------------------------------------------------------
vtkVariant vtkAbstractArray::GetVariantValue(vtkIdType i)
{
  vtkVariant val;
  switch(this->GetDataType())
    {
    vtkExtraExtendedTemplateMacro(val = vtkAbstractArrayGetVariantValue(
      static_cast<VTK_TT*>(this->GetVoidPointer(0)), i));
    }
  return val;
}

//----------------------------------------------------------------------------
void vtkAbstractArray::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  const char* name = this->GetName();
  if (name)
    {
    os << indent << "Name: " << name << "\n";
    }
  else
    {
    os << indent << "Name: (none)\n";
    }
  os << indent << "Data type: " << this->GetDataTypeAsString() << "\n";
  os << indent << "Size: " << this->Size << "\n";
  os << indent << "MaxId: " << this->MaxId << "\n";
  os << indent << "NumberOfComponents: " << this->NumberOfComponents << endl;
  os << indent << "Information: " << this->Information << endl;
  if ( this->Information )
    {
    this->Information->PrintSelf( os, indent.GetNextIndent() );
    }
}
