/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: TransmitRectilinearGrid.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// Tests vtkTransmitRectilinearGrid.

/*
** This test only builds if MPI is in use
*/
#include <mpi.h>
#include "vtkMPICommunicator.h"


#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"
#include "vtkParallelFactory.h"
#include "vtkMPIController.h"

#include "vtkRectilinearGrid.h"
#include "vtkRectilinearGridReader.h"
#include "vtkDataObject.h"
#include "vtkTransmitRectilinearGridPiece.h"
#include "vtkContourFilter.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkElevationFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCompositeRenderManager.h"
#include "vtkCamera.h"
#include "vtkProcess.h"

class MyProcess : public vtkProcess
{
public:
  static MyProcess *New();
  vtkTypeRevisionMacro(MyProcess, vtkProcess);
  
  virtual void Execute();

  void SetArgs(int anArgc,
               char *anArgv[]);
  
protected:
  MyProcess();
  
  int Argc;
  char **Argv;
};

vtkStandardNewMacro(MyProcess);

MyProcess::MyProcess()
{
  this->Argc=0;
  this->Argv=0;
}

void MyProcess::SetArgs(int anArgc,
                        char *anArgv[])
{
  this->Argc=anArgc;
  this->Argv=anArgv;  
}

void MyProcess::Execute()
{
  this->ReturnValue=1;
  int numProcs=this->Controller->GetNumberOfProcesses();
  int me=this->Controller->GetLocalProcessId();
  
  int i, go;

  vtkCompositeRenderManager *prm = vtkCompositeRenderManager::New();

  // READER

  vtkRectilinearGridReader *rgr = NULL;
  vtkRectilinearGrid *rg = NULL;

  if (me == 0)
    {
    rgr = vtkRectilinearGridReader::New();

    char* fname = 
      vtkTestUtilities::ExpandDataFileName(
        this->Argc, this->Argv, "Data/RectGrid2.vtk");

    rgr->SetFileName(fname);

    rg = rgr->GetOutput();

    rgr->Update();

    delete [] fname;

    go = 1;

    if ((rg == NULL) || (rg->GetNumberOfCells() == 0))
      {
      if (rg) cout << "Failure: input file has no cells" << endl;
      go = 0;
      }
    }
  
  vtkMPICommunicator *comm =
    vtkMPICommunicator::SafeDownCast(this->Controller->GetCommunicator());

  comm->Broadcast(&go, 1, 0);

  if (!go)
    {
    if (rgr)
      {
      rgr->Delete();
      }
    prm->Delete();
    return;
    }

  // FILTER WE ARE TRYING TO TEST
  vtkTransmitRectilinearGridPiece *pass = vtkTransmitRectilinearGridPiece::New();
  pass->SetController(this->Controller);
  if (me == 0)
    {
    pass->SetInput(rg); 
    }

  // FILTERING
  vtkContourFilter *cf = vtkContourFilter::New();
  cf->SetInput(pass->GetOutput());
  cf->SetNumberOfContours(1);
  cf->SetValue(0,0.1);
  (cf->GetInput())->RequestExactExtentOn();
  cf->ComputeNormalsOff();
  vtkElevationFilter *elev = vtkElevationFilter::New();
  elev->SetInput(cf->GetOutput());
  elev->SetScalarRange(me, me + .001);

  // COMPOSITE RENDER
  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInput(vtkPolyData::SafeDownCast(elev->GetOutput()));
  mapper->SetScalarRange(0, numProcs);
  vtkActor *actor = vtkActor::New();
  actor->SetMapper(mapper);
  vtkRenderer *renderer = prm->MakeRenderer();
  renderer->AddActor(actor);
  vtkRenderWindow *renWin = prm->MakeRenderWindow();
  renWin->AddRenderer(renderer);
  renderer->SetBackground(0,0,0);
  renWin->SetSize(300,300);
  renWin->SetPosition(0, 360*me);
  prm->SetRenderWindow(renWin);
  prm->SetController(this->Controller);
  prm->InitializeOffScreen();   // Mesa GL only
  if (me == 0)
    {
    prm->ResetAllCameras();
    }

  // We must update the whole pipeline here, otherwise node 0
  // goes into GetActiveCamera which updates the pipeline, putting
  // it into vtkDistributedDataFilter::Execute() which then hangs.
  // If it executes here, dd will be up-to-date won't have to 
  // execute in GetActiveCamera.

  mapper->SetPiece(me);
  mapper->SetNumberOfPieces(numProcs);
  mapper->Update();

  const int MY_RETURN_VALUE_MESSAGE=0x11;
  
  if (me == 0)
    {
    vtkCamera *camera = renderer->GetActiveCamera();
    //camera->UpdateViewport(renderer);
    camera->SetParallelScale(16);

    renWin->Render();
    renWin->Render();

    this->ReturnValue = vtkRegressionTester::Test(this->Argc, this->Argv, renWin, 10);

    for (i=1; i < numProcs; i++)
      {
      this->Controller->Send(&this->ReturnValue, 1, i,MY_RETURN_VALUE_MESSAGE);
      }

    prm->StopServices();
    }
  else
    {
    prm->StartServices();
    this->Controller->Receive(&this->ReturnValue, 1, 0,MY_RETURN_VALUE_MESSAGE);
    }

  // CLEAN UP 
  renWin->Delete(); 
  renderer->Delete(); 
  actor->Delete(); 
  mapper->Delete(); 
  elev->Delete(); 
  cf->Delete(); 
  pass->Delete();
  if (me == 0)
    {
    rgr->Delete();
    }
  prm->Delete();
}

int main(int argc, char **argv)
{
  // This is here to avoid false leak messages from vtkDebugLeaks when
  // using mpich. It appears that the root process which spawns all the
  // main processes waits in MPI_Init() and calls exit() when
  // the others are done, causing apparent memory leaks for any objects
  // created before MPI_Init().
  MPI_Init(&argc, &argv);

  // Note that this will create a vtkMPIController if MPI
  // is configured, vtkThreadedController otherwise.
  vtkMPIController *contr = vtkMPIController::New();
  contr->Initialize(&argc, &argv, 1);

  int retVal = 1;

  vtkMultiProcessController::SetGlobalController(contr);

  int numProcs = contr->GetNumberOfProcesses();
  int me = contr->GetLocalProcessId();

  if (numProcs != 2)
    {
    if (me == 0)
      {
      cout << "DistributedData test requires 2 processes" << endl;
      }
    contr->Delete();
    return retVal;
    }

  if (!contr->IsA("vtkMPIController"))
    {
    if (me == 0)
      {
      cout << "DistributedData test requires MPI" << endl;
      }
    contr->Delete();
    return retVal;   // is this the right error val?   TODO
    }

  MyProcess *p=MyProcess::New();
  p->SetArgs(argc,argv);
  contr->SetSingleProcessObject(p);
  contr->SingleMethodExecute();

  retVal=p->GetReturnValue();
  p->Delete();
  contr->Finalize();
  contr->Delete();

  return !retVal;
}
