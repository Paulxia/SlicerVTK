/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDataRepresentation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkDataRepresentation.h"

#include "vtkAnnotationLink.h"
#include "vtkCommand.h"
#include "vtkConvertSelectionDomain.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"

#include <vtkstd/map>

//---------------------------------------------------------------------------
// vtkDataRepresentation::Internals
//---------------------------------------------------------------------------

class vtkDataRepresentation::Internals {
public:

  // This is a cache of shallow copies of inputs provided for convenience.
  // It is a map from (port index, connection index) to (original input data object, shallow copy).
  // NOTE: The original input data object pointer is not reference counted, so it should
  // not be assumed to be a valid pointer. It is only used for pointer comparison.
  vtkstd::map<vtkstd::pair<int, int>,
              vtkstd::pair<vtkDataObject*, vtkSmartPointer<vtkDataObject> > >
    InputInternal;

  // This is a cache of vtkConvertSelectionDomain filters provided for convenience.
  // It is a map from (port index, connection index) to convert selection domain filter.
  vtkstd::map<vtkstd::pair<int, int>, vtkSmartPointer<vtkConvertSelectionDomain> >
    ConvertDomainInternal;
};

//---------------------------------------------------------------------------
// vtkDataRepresentation::Command
//----------------------------------------------------------------------------

class vtkDataRepresentation::Command : public vtkCommand
{
public:
  static Command* New() {  return new Command(); }
  virtual void Execute(vtkObject *caller, unsigned long eventId,
                       void *callData)
    {
    if (this->Target)
      {
      this->Target->ProcessEvents(caller, eventId, callData);
      }
    }
  void SetTarget(vtkDataRepresentation* t)
    {
    this->Target = t;
    }
private:
  Command() { this->Target = 0; }
  vtkDataRepresentation* Target;
};

//----------------------------------------------------------------------------
// vtkDataRepresentation
//----------------------------------------------------------------------------

vtkCxxRevisionMacro(vtkDataRepresentation, "1.14");
vtkStandardNewMacro(vtkDataRepresentation);
vtkCxxSetObjectMacro(vtkDataRepresentation,
  AnnotationLinkInternal, vtkAnnotationLink);
vtkCxxSetObjectMacro(vtkDataRepresentation, SelectionArrayNames, vtkStringArray);

//----------------------------------------------------------------------------
vtkDataRepresentation::vtkDataRepresentation()
{
  this->Implementation = new vtkDataRepresentation::Internals();
  // Listen to event indicating that the algorithm is done executing.
  // We may need to clear the data object cache after execution.
  this->Observer = Command::New();
  this->AddObserver(vtkCommand::EndEvent, this->Observer);
  
  this->Selectable = true;
  this->SelectionArrayNames = vtkStringArray::New();
  this->SelectionType = vtkSelectionNode::INDICES;
  this->AnnotationLinkInternal = vtkAnnotationLink::New();
  this->SetNumberOfOutputPorts(0);
}

//----------------------------------------------------------------------------
vtkDataRepresentation::~vtkDataRepresentation()
{
  delete this->Implementation;
  this->Observer->Delete();
  this->SetSelectionArrayNames(0);
  this->SetAnnotationLinkInternal(0);
}

//----------------------------------------------------------------------------
void vtkDataRepresentation::SetAnnotationLink(vtkAnnotationLink* link)
{
  this->SetAnnotationLinkInternal(link);
}

//----------------------------------------------------------------------------
void vtkDataRepresentation::ProcessEvents(vtkObject *caller, unsigned long eventId, void *vtkNotUsed(callData))
{
  // After the algorithm executes, if the release data flag is on,
  // clear the input shallow copy cache.
  if (caller == this && eventId == vtkCommand::EndEvent)
    {
    // Release input data if requested.
    for (int i = 0; i < this->GetNumberOfInputPorts(); ++i)
      {
      for (int j = 0; j < this->GetNumberOfInputConnections(i); ++j)
        {
        vtkInformation* inInfo = this->GetExecutive()->GetInputInformation(i, j);
        vtkDataObject* dataObject = inInfo->Get(vtkDataObject::DATA_OBJECT());
        if (dataObject && (dataObject->GetGlobalReleaseDataFlag() ||
            inInfo->Get(vtkDemandDrivenPipeline::RELEASE_DATA())))
          {
          vtkstd::pair<int, int> p(i, j);
          this->Implementation->InputInternal.erase(p);
          this->Implementation->ConvertDomainInternal.erase(p);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDataRepresentation::GetInternalOutputPort(int port, int conn)
{
  if (port >= this->GetNumberOfInputPorts() ||
    conn >= this->GetNumberOfInputConnections(port))
    {
    vtkErrorMacro("Port " << port << ", connection "
      << conn << " is not defined on this representation.");
    return 0;
    }

  // The cached shallow copy is out of date when the input data object
  // changed, or the shallow copy modified time is less than the
  // input modified time.
  vtkstd::pair<int, int> p(port, conn);
  vtkDataObject* input = this->GetInputDataObject(port, conn);
  if (this->Implementation->InputInternal.find(p) ==
    this->Implementation->InputInternal.end() ||
    this->Implementation->InputInternal[p].first != input ||
    this->Implementation->InputInternal[p].second->GetMTime() < input->GetMTime())
    {
    this->Implementation->InputInternal[p].first = input;
    this->Implementation->InputInternal[p].second.TakeReference(
      input->NewInstance());
    this->Implementation->InputInternal[p].second->ShallowCopy(input);
    }

  return this->Implementation->InputInternal[p].second->GetProducerPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDataRepresentation::GetInternalAnnotationOutputPort(
  int port, int conn)
{
  if (port >= this->GetNumberOfInputPorts() ||
    conn >= this->GetNumberOfInputConnections(port))
    {
    vtkErrorMacro("Port " << port << ", connection "
      << conn << " is not defined on this representation.");
    return 0;
    }

  // Create a new filter in the cache if necessary.
  vtkstd::pair<int, int> p(port, conn);
  if (this->Implementation->ConvertDomainInternal.find(p) ==
    this->Implementation->ConvertDomainInternal.end())
    {
    this->Implementation->ConvertDomainInternal[p] =
      vtkSmartPointer<vtkConvertSelectionDomain>::New();
    }

  // Set up the inputs to the cached filter.
  vtkConvertSelectionDomain* domain = this->Implementation->ConvertDomainInternal[p];
  domain->SetInputConnection(0,
    this->GetAnnotationLink()->GetOutputPort(0));
  domain->SetInputConnection(1,
    this->GetAnnotationLink()->GetOutputPort(1));
  domain->SetInputConnection(2,
    this->GetInternalOutputPort(port, conn));

  // Output port 0 of the convert domain filter is the linked
  // annotation(s) (the vtkAnnotationLayers object).
  return domain->GetOutputPort();
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkDataRepresentation::GetInternalSelectionOutputPort(
  int port, int conn)
{
  // First make sure the convert domain filter is up to date.
  if (!this->GetInternalAnnotationOutputPort(port, conn))
    {
    return 0;
    }

  // Output port 1 of the convert domain filter is the current selection
  // that was contained in the linked annotation.
  vtkstd::pair<int, int> p(port, conn);
  if (this->Implementation->ConvertDomainInternal.find(p) !=
    this->Implementation->ConvertDomainInternal.end())
    {
    return this->Implementation->ConvertDomainInternal[p]->GetOutputPort(1);
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkDataRepresentation::Select(
  vtkView* view, vtkSelection* selection, bool extend)
{
  if (this->Selectable)
    {
    vtkSelection* converted = this->ConvertSelection(view, selection);
    if (converted)
      {
      this->UpdateSelection(converted, extend);
      if (converted != selection)
        {
        converted->Delete();
        }
      }
    }
}

//----------------------------------------------------------------------------
vtkSelection* vtkDataRepresentation::ConvertSelection(
  vtkView* vtkNotUsed(view), vtkSelection* selection)
{
  return selection;
}

//----------------------------------------------------------------------------
void vtkDataRepresentation::UpdateSelection(vtkSelection* selection, bool extend)
{
  if (extend)
    {
    selection->Union(this->AnnotationLinkInternal->GetCurrentSelection());
    }
  this->AnnotationLinkInternal->SetCurrentSelection(selection);
  this->InvokeEvent(vtkCommand::SelectionChangedEvent, reinterpret_cast<void*>(selection));
}

//----------------------------------------------------------------------------
void vtkDataRepresentation::SetSelectionArrayName(const char* name)
{
  if (!this->SelectionArrayNames)
    {
    this->SelectionArrayNames = vtkStringArray::New();
    }
  this->SelectionArrayNames->Initialize();
  this->SelectionArrayNames->InsertNextValue(name);
}

//----------------------------------------------------------------------------
const char* vtkDataRepresentation::GetSelectionArrayName()
{
  if (this->SelectionArrayNames &&
      this->SelectionArrayNames->GetNumberOfTuples() > 0)
    {
    return this->SelectionArrayNames->GetValue(0);
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkDataRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "AnnotationLink: " << (this->AnnotationLinkInternal ? "" : "(null)") << endl;
  if (this->AnnotationLinkInternal)
    {
    this->AnnotationLinkInternal->PrintSelf(os, indent.GetNextIndent());
    }  
  os << indent << "Selectable: " << this->Selectable << endl;
  os << indent << "SelectionType: " << this->SelectionType << endl;
  os << indent << "SelectionArrayNames: " << (this->SelectionArrayNames ? "" : "(null)") << endl;
  if (this->SelectionArrayNames)
    {
    this->SelectionArrayNames->PrintSelf(os, indent.GetNextIndent());
    }
}
