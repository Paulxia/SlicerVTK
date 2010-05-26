/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkHyperOctreeAlgorithm.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkHyperOctreeAlgorithm.h"

#include "vtkCommand.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkHyperOctree.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTrivialProducer.h"


//----------------------------------------------------------------------------
vtkHyperOctreeAlgorithm::vtkHyperOctreeAlgorithm()
{
  // by default assume filters have one input and one output
  // subclasses that deviate should modify this setting
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkHyperOctreeAlgorithm::~vtkHyperOctreeAlgorithm()
{
}

//----------------------------------------------------------------------------
void vtkHyperOctreeAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkHyperOctree* vtkHyperOctreeAlgorithm::GetOutput()
{
  return this->GetOutput(0);
}

//----------------------------------------------------------------------------
vtkHyperOctree* vtkHyperOctreeAlgorithm::GetOutput(int port)
{
  return vtkHyperOctree::SafeDownCast(this->GetOutputDataObject(port));
}

//----------------------------------------------------------------------------
void vtkHyperOctreeAlgorithm::SetOutput(vtkDataObject* d)
{
  this->GetExecutive()->SetOutputData(0, d);
}

//----------------------------------------------------------------------------
vtkDataObject* vtkHyperOctreeAlgorithm::GetInput()
{
  return this->GetInput(0);
}

//----------------------------------------------------------------------------
vtkDataObject* vtkHyperOctreeAlgorithm::GetInput(int port)
{
  if (this->GetNumberOfInputConnections(port) < 1)
    {
    return 0;
    }
  return this->GetExecutive()->GetInputData(port, 0);
}

//----------------------------------------------------------------------------
vtkHyperOctree* vtkHyperOctreeAlgorithm::GetHyperOctreeInput(int port)
{
  return vtkHyperOctree::SafeDownCast(this->GetInput(port));
}

//----------------------------------------------------------------------------
int vtkHyperOctreeAlgorithm::ProcessRequest(vtkInformation* request,
                                         vtkInformationVector** inputVector,
                                         vtkInformationVector* outputVector)
{ 
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    return this->RequestData(request, inputVector, outputVector);
    }

  if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
    return this->RequestUpdateExtent(request, inputVector, outputVector);
    }

  // execute information
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    return this->RequestInformation(request, inputVector, outputVector);
    }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkHyperOctreeAlgorithm::FillOutputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  // now add our info
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkHyperOctree");
  return 1;
}

//----------------------------------------------------------------------------
int vtkHyperOctreeAlgorithm::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkHyperOctree");
  return 1;
}

//----------------------------------------------------------------------------
int vtkHyperOctreeAlgorithm::RequestInformation(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* vtkNotUsed(outputVector))
{
  // do nothing let subclasses handle it
  return 1;
}

//----------------------------------------------------------------------------
int vtkHyperOctreeAlgorithm::RequestUpdateExtent(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* vtkNotUsed(outputVector))
{
  int numInputPorts = this->GetNumberOfInputPorts();
  for (int i=0; i<numInputPorts; i++)
    {
    int numInputConnections = this->GetNumberOfInputConnections(i);
    for (int j=0; j<numInputConnections; j++)
      {
      vtkInformation* inputInfo = inputVector[i]->GetInformationObject(j);
      inputInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an imaging style Execute method.
int vtkHyperOctreeAlgorithm::RequestData(
  vtkInformation* request,
  vtkInformationVector** vtkNotUsed( inputVector ),
  vtkInformationVector* outputVector)
{
  // the default implimentation is to do what the old pipeline did find what
  // output is requesting the data, and pass that into ExecuteData

  // which output port did the request come from
  int outputPort = 
    request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());

  // if output port is negative then that means this filter is calling the
  // update directly, in that case just assume port 0
  if (outputPort == -1)
      {
      outputPort = 0;
      }
  
  // get the data object
  vtkInformation *outInfo = 
    outputVector->GetInformationObject(outputPort);
  // call ExecuteData
  this->ExecuteData( outInfo->Get(vtkDataObject::DATA_OBJECT()) );

  return 1;
}

//----------------------------------------------------------------------------
// Assume that any source that implements ExecuteData 
// can handle an empty extent.
void vtkHyperOctreeAlgorithm::ExecuteData(vtkDataObject *output)
{
  // I want to find out if the requested extent is empty.
  if (output && this->UpdateExtentIsEmpty(output))
    {
    output->Initialize();
    return;
    }
  
  this->Execute();
}

//----------------------------------------------------------------------------
void vtkHyperOctreeAlgorithm::Execute()
{
  vtkErrorMacro(<< "Definition of Execute() method should be in subclass and you should really use the ExecuteData(vtkInformation *request,...) signature instead");
}


//----------------------------------------------------------------------------
void vtkHyperOctreeAlgorithm::SetInput(vtkDataObject* input)
{
  this->SetInput(0, input);
}

//----------------------------------------------------------------------------
void vtkHyperOctreeAlgorithm::SetInput(int index, vtkDataObject* input)
{
  if(input)
    {
    this->SetInputConnection(index, input->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(index, 0);
    }
}

//----------------------------------------------------------------------------
void vtkHyperOctreeAlgorithm::AddInput(vtkDataObject* input)
{
  this->AddInput(0, input);
}

//----------------------------------------------------------------------------
void vtkHyperOctreeAlgorithm::AddInput(int index, vtkDataObject* input)
{
  if(input)
    {
    this->AddInputConnection(index, input->GetProducerPort());
    }
}
