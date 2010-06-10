/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkClipConvexPolyData.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkClipConvexPolyData.h"

#include "vtkCellArray.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneCollection.h"
#include "vtkPolyData.h"

#include <vtkstd/vector>
#include <vtkstd/set>
#include <vtkstd/algorithm>
#include <vtkstd/iterator>

vtkCxxRevisionMacro(vtkClipConvexPolyData,"$Revision: 1.4 $");
vtkStandardNewMacro(vtkClipConvexPolyData);

vtkCxxSetObjectMacro(vtkClipConvexPolyData,Planes,vtkPlaneCollection);

// ----------------------------------------------------------------------------
class vtkCCPDVertex
{
public:
  double Point[3];
};

// ----------------------------------------------------------------------------
class vtkCCPDPolygon
{
public:
  vtkstd::vector<vtkCCPDVertex*> Vertices;
  vtkstd::vector<vtkCCPDVertex*> NewVertices;
};

// ----------------------------------------------------------------------------
class vtkClipConvexPolyDataInternals
{
public:
  vtkstd::vector<vtkCCPDPolygon*> Polygons;
};

// ----------------------------------------------------------------------------
// Constructor
vtkClipConvexPolyData::vtkClipConvexPolyData()
{
  this->Planes=NULL;
  this->Internal=new vtkClipConvexPolyDataInternals;
  
}

// ----------------------------------------------------------------------------
// Destructor 
vtkClipConvexPolyData::~vtkClipConvexPolyData()
{
  this->SetPlanes(NULL);
  this->ClearInternals();
  delete this->Internal;
}

// ----------------------------------------------------------------------------
// Description:
// Redefines this method, as this filter depends on time of its components
// (planes)
unsigned long int vtkClipConvexPolyData::GetMTime()
{
  unsigned long int result=Superclass::GetMTime();
  if(this->Planes!=0)
    {
    unsigned long int planesTime=this->Planes->GetMTime();
    if(planesTime>result)
      {
      result=planesTime;
      }
    }
  return result;
}

// ----------------------------------------------------------------------------
void vtkClipConvexPolyData::ClearInternals()
{
  unsigned int j;
  for(unsigned int i=0; i<this->Internal->Polygons.size(); i++)
    {
    for (j=0; j<this->Internal->Polygons[i]->Vertices.size(); j++)
      {
      delete this->Internal->Polygons[i]->Vertices[j];
      }
    this->Internal->Polygons[i]->Vertices.clear();

    for (j=0; j<this->Internal->Polygons[i]->NewVertices.size(); j++)
      {
      delete this->Internal->Polygons[i]->NewVertices[j];
      }
    this->Internal->Polygons[i]->NewVertices.clear();

    
    delete this->Internal->Polygons[i];
    }
  this->Internal->Polygons.clear(); 
}

// ----------------------------------------------------------------------------
void vtkClipConvexPolyData::ClearNewVertices()
{
  for(unsigned int i=0; i<this->Internal->Polygons.size(); i++)
    {
    for(unsigned int j=0; j<this->Internal->Polygons[i]->NewVertices.size();
        j++)
      {
      delete this->Internal->Polygons[i]->NewVertices[j];
      }
    this->Internal->Polygons[i]->NewVertices.clear();
    }
}

// ----------------------------------------------------------------------------
void vtkClipConvexPolyData::RemoveEmptyPolygons()
{
  bool done = false;
  
  while (!done )
    {
    done = true;
    for(unsigned int i=0; i<this->Internal->Polygons.size(); i++)
      {
      if ( this->Internal->Polygons[i]->Vertices.size() == 0 )
        {
        vtkstd::vector<vtkCCPDPolygon*>::iterator where =
          vtkstd::find(this->Internal->Polygons.begin(),
               this->Internal->Polygons.end(),
               this->Internal->Polygons[i]);
        if ( where != this->Internal->Polygons.end() )
          {
          delete this->Internal->Polygons[i];
          this->Internal->Polygons.erase(where);
          done = false;
          break;
          }
        }
      }
    }
}

// ----------------------------------------------------------------------------
//
int vtkClipConvexPolyData::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // Pre-conditions
  if(this->Planes==0)
    {
    vtkErrorMacro("plane collection is null");
    return 0;
    }
  if(this->Planes->GetNumberOfItems()==0)
    {
    vtkErrorMacro("plane collection is empty");
    return 0;
    }
  
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkCellArray *polys = input->GetPolys();
  vtkPoints *points = input->GetPoints();

  // Compute tolerance to be 0.00001 of diagonal
  double min[3] = {VTK_DOUBLE_MAX, VTK_DOUBLE_MAX, VTK_DOUBLE_MAX};
  double max[3] = {VTK_DOUBLE_MIN, VTK_DOUBLE_MIN, VTK_DOUBLE_MIN};
  
  size_t i, j;
  double tolerance;
  for ( i = 0; i < static_cast<size_t>(points->GetNumberOfPoints()); i++ )
    {
    double pt[3];
    points->GetPoint(static_cast<vtkIdType>(i), pt);
    for ( j = 0; j < 3; j++ )
      {
      min[j] = (pt[j] < min[j])?(pt[j]):(min[j]);
      max[j] = (pt[j] > max[j])?(pt[j]):(max[j]);
      }
    }
  tolerance = sqrt(vtkMath::Distance2BetweenPoints(min,max))*0.00001;

  // Copy the polygons from the polys array to the internal
  // data structure
  vtkIdType npts;
  vtkIdType *pts;
  polys->InitTraversal();
  while ( polys->GetNextCell(npts, pts) )
    {
    vtkCCPDPolygon *polygon = new vtkCCPDPolygon;
    for ( i = 0; i < static_cast<size_t>(npts); i++ )
      {
      vtkCCPDVertex *v = new vtkCCPDVertex;
      points->GetPoint(pts[i], v->Point);
      polygon->Vertices.push_back(v);
      }
    this->Internal->Polygons.push_back(polygon);
    }

  this->Planes->InitTraversal();
  vtkPlane *plane;

  // For each plane in the collection, clip the polygons with the plane.
  while ( (plane = this->Planes->GetNextItem()) )
    {
    this->ClipWithPlane( plane, tolerance );
    }

  // Create a new set of points and polygons into which the results will
  // be stored
  vtkPoints *outPoints = vtkPoints::New();
  vtkCellArray *outPolys  = vtkCellArray::New();
  //std::cout << "******Output polygons******\n";
  for ( i = 0; i < this->Internal->Polygons.size(); i++ )
    {
    const size_t numPoints = this->Internal->Polygons[i]->Vertices.size();
    vtkIdType *polyPts = new vtkIdType[numPoints];
    //std::cout << "Polygon[" << i << "] : \n";
    for ( j = 0; j < numPoints; j++ )
      {
      polyPts[j] = outPoints->InsertNextPoint(
        this->Internal->Polygons[i]->Vertices[j]->Point );
      //std::cout << "   { "
      //          << this->Internal->Polygons[i]->Vertices[j]->Point[0] << ", "
      //          << this->Internal->Polygons[i]->Vertices[j]->Point[1] << ", "
      //          << this->Internal->Polygons[i]->Vertices[j]->Point[2] << "\n";
      }
    outPolys->InsertNextCell(static_cast<vtkIdType>(numPoints), polyPts );
    delete [] polyPts;
    }
  //std::cout.flush();
  // Set the output vertices and polygons
  output->SetPoints(outPoints);
  output->SetPolys(outPolys);

  // Delete the temporary storage
  outPoints->Delete();
  outPolys->Delete();

  this->ClearInternals();
  
  return 1;
}

// ----------------------------------------------------------------------------
void vtkClipConvexPolyData::ClipWithPlane( vtkPlane *plane, double tolerance )
{
  double origin[3];
  double normal[4];

  plane->GetOrigin( origin );
  plane->GetNormal( normal );
  //std::cout << "orig: "
  //          << origin[0] << " " << origin[1] << " " << origin[2] << std::endl;
  //std::cout << "norm: "
  //          << normal[0] << " " << normal[1] << " " << normal[2] << std::endl;
  vtkMath::Normalize(normal);

  normal[3] = -(origin[0]*normal[0] +
                origin[1]*normal[1] +
                origin[2]*normal[2]);

  int numNewPoints = 0;
  unsigned int i;
  size_t j;

  // For each polygon
  for ( i = 0; i < this->Internal->Polygons.size(); i++ )
    {
    // the new polygon. We are clipping the existing polygon then
    // removing that old polygon and adding this clipped polygon
    vtkCCPDPolygon *newPoly = new vtkCCPDPolygon;

    int somePositive = 0;
    int someNegative = 0;
    if (this->Internal->Polygons[i]->NewVertices.size())
      {
      vtkErrorMacro( << "NewVertices should be cleared before use");
      this->RemoveEmptyPolygons();
      this->ClearNewVertices();
      return;
      }
    // Look around the polygon - we only want to process it if
    // there are some positive vertices when passed through the
    // plane equation. If they are all negative, then it is entirely
    // clipped. If they are either negative or 0, then this is a boundary
    // condition and we also don't want to consider it
    size_t numVertices = this->Internal->Polygons[i]->Vertices.size();
    for ( j = 0; j < numVertices; j++ )
      {
      double *p1=this->Internal->Polygons[i]->Vertices[j]->Point;
      double p1D=p1[0]*normal[0]+p1[1]*normal[1]+p1[2]*normal[2] + normal[3];
      if ( p1D < 2.0*tolerance && p1D > -2.0*tolerance )
        {
        p1D = 0;
        }
      if ( p1D > 0 )
        {
        somePositive = 1;
        // remove the comment
        //break;
        }
      if (p1D < 0)
        {
        someNegative = 1;
        }
      }
    if (!someNegative && !somePositive)
      {
      // A polygon fully matches the clipping plane (all the p1D were 0)
      // there is no need to truncate any plan. It's fine if some
      // planes have already been processed, they didn't loose any point.
      this->RemoveEmptyPolygons();
      this->ClearNewVertices();
      return;
      }

    if ( somePositive )
      {
      // For each vertex
      for ( j = 0; j < numVertices; j++ )
        {
        double *p1=this->Internal->Polygons[i]->Vertices[j]->Point;
        double *p2=
          this->Internal->Polygons[i]->Vertices[(j+1)%numVertices]->Point;

        double p1D=p1[0]*normal[0]+p1[1]*normal[1]+p1[2]*normal[2] + normal[3];
        double p2D=p2[0]*normal[0]+p2[1]*normal[1]+p2[2]*normal[2] + normal[3];
        // We want to avoid the case where we just barely clip a vertex. If we allow
        // that to happen then we wind up with too many candidate points all in
        // approximately the same place when we try to form a loop to close off
        // the cut. So if the point is within a 1/10th of the tolerance factor
        // we've set, we'll just consider it not clipped.
        if ( p1D < 2*tolerance && p1D > -2.0*tolerance )
          {
          p1D = 0;
          }

        if ( p2D < 2*tolerance && p2D > -2.0*tolerance )
          {
          p2D = 0;
          }
        // Add p1 in if it is not clipped. If the whole polygon is unclipped
        // then we'll just add in each vertex in turn. If the whole polygon
        // is clipped we won't add in any vertices. If the polygon is
        // clipped, we'll add in two new points corresponding the the
        // crossing location of the plane on two edges of the polygon
        if ( p1D >= 0 )
          {
          vtkCCPDVertex *v = new vtkCCPDVertex;
          v->Point[0] = p1[0];
          v->Point[1] = p1[1];
          v->Point[2] = p1[2];
          newPoly->Vertices.push_back(v);
          }

        // If the first point is exactly on the boundary, we need to also count it
        // as a new point. However, if the point is on the boundary and the next
        // point is not rejected, then there is no need to create a point
        if ( p1D == 0 && p2D <= 0)
          {
          vtkCCPDVertex *v = new vtkCCPDVertex;
          v->Point[0] = p1[0];
          v->Point[1] = p1[1];
          v->Point[2] = p1[2];
          this->Internal->Polygons[i]->NewVertices.push_back(v);
          numNewPoints++;
          }

        if ( p2D == 0 && p1D <= 0 )
          {
          vtkCCPDVertex *v = new vtkCCPDVertex;
          v->Point[0] = p2[0];
          v->Point[1] = p2[1];
          v->Point[2] = p2[2];
          this->Internal->Polygons[i]->NewVertices.push_back(v);
          numNewPoints++;
          }

        // If the plane clips this edge - find the crossing point. We'll need
        // to add this point to the new polygon
        if ( p1D*p2D < 0 )
          {
          double w = -p1D / (p2D - p1D);

          vtkCCPDVertex *v = new vtkCCPDVertex;
          v->Point[0] = p1[0] + w * (p2[0]-p1[0]);
          v->Point[1] = p1[1] + w * (p2[1]-p1[1]);
          v->Point[2] = p1[2] + w * (p2[2]-p1[2]);
          newPoly->Vertices.push_back(v);

          v = new vtkCCPDVertex;
          v->Point[0] = p1[0] + w * (p2[0]-p1[0]);
          v->Point[1] = p1[1] + w * (p2[1]-p1[1]);
          v->Point[2] = p1[2] + w * (p2[2]-p1[2]);
          this->Internal->Polygons[i]->NewVertices.push_back(v);
          numNewPoints++;
          }
        }
      }

    // Remove the current polygon
    for ( j = 0; j < numVertices; j++ )
      {
      delete this->Internal->Polygons[i]->Vertices[j];
      }
    this->Internal->Polygons[i]->Vertices.clear();

    // copy in the new polygon if it isn't entirely clipped
    numVertices = newPoly->Vertices.size();
    if ( numVertices > 0 )
      {
      for ( j = 0; j < numVertices; j++ )
        {
        this->Internal->Polygons[i]->Vertices.push_back(newPoly->Vertices[j]);
        }
      newPoly->Vertices.clear();
      }
    delete newPoly;
    }

  // If we've added any new points when clipping the polydata then
  // we must have added at least six. Otherwise something is wrong
  if ( numNewPoints )
    {
    if ( numNewPoints < 6 )
      {
      vtkErrorMacro(<< "Failure - not enough new points. Only "
                    << numNewPoints << " points.");
      this->RemoveEmptyPolygons();
      this->ClearNewVertices();
      return;
      }

    // Find the first polygon with a new point
    size_t idx = 0;
    bool idxFound=false;

    for ( i = 0; !idxFound && i < this->Internal->Polygons.size(); i++ )
      {
      idxFound=this->Internal->Polygons[i]->NewVertices.size() > 0;
      if(idxFound)
        {
        idx=i;
        }
      }

    if (!idxFound)
      {
      vtkErrorMacro( << "Couldn't find any new vertices!");
      return;
      }

    // the new polygon
    vtkCCPDPolygon *newPoly = new vtkCCPDPolygon;
    vtkCCPDVertex *v = new vtkCCPDVertex;
    v->Point[0] = this->Internal->Polygons[idx]->NewVertices[0]->Point[0];
    v->Point[1] = this->Internal->Polygons[idx]->NewVertices[0]->Point[1];
    v->Point[2] = this->Internal->Polygons[idx]->NewVertices[0]->Point[2];
    newPoly->Vertices.push_back(v);

    int count = this->Internal->Polygons[idx]->NewVertices.size();
    for (int k = 1 ; k < count; ++k)
      {
      v = new vtkCCPDVertex;
      v->Point[0] = this->Internal->Polygons[idx]->NewVertices[k]->Point[0];
      v->Point[1] = this->Internal->Polygons[idx]->NewVertices[k]->Point[1];
      v->Point[2] = this->Internal->Polygons[idx]->NewVertices[k]->Point[2];
      newPoly->Vertices.push_back(v);
      }

    double lastPoint[3];
    lastPoint[0] = this->Internal->Polygons[idx]->NewVertices[count - 1]->Point[0];
    lastPoint[1] = this->Internal->Polygons[idx]->NewVertices[count - 1]->Point[1];
    lastPoint[2] = this->Internal->Polygons[idx]->NewVertices[count - 1]->Point[2];

    size_t lastPointIdx = idx;
    size_t subIdx;

    while ( static_cast<int>(newPoly->Vertices.size()) < numNewPoints )
      {
      // Find the index of the closest new vertex that matches the
      // lastPoint but not the lastPointIdx.
      subIdx = 0;
      float closestDistance = VTK_FLOAT_MAX;
      bool foundSubIdx=false;
      const unsigned int size = this->Internal->Polygons.size();
      for ( i = 0; i < size; i++ )
        {
          if ( i != lastPointIdx &&
               this->Internal->Polygons[i]->NewVertices.size() > 0 )
          {
          const size_t newVerticesSize = this->Internal->Polygons[i]->NewVertices.size();
          for ( j = 0; j < newVerticesSize; j++ )
            {
            float testDistance =
              vtkMath::Distance2BetweenPoints(lastPoint,
                  this->Internal->Polygons[i]->NewVertices[j]->Point);
            if ( testDistance < tolerance && testDistance < closestDistance)
              {
              closestDistance = testDistance;
              idx = i;
              subIdx =j;
              foundSubIdx=true;
              }
            }
           }
        }

      if ( !foundSubIdx )
        {
        vtkErrorMacro("Could not find a match" << lastPoint[0] << "," << lastPoint[1] << " " << lastPoint[2] );
        }
      count = this->Internal->Polygons[idx]->NewVertices.size();
      for (unsigned int k = ((subIdx+1) %count);
           k != subIdx;
           k = ((k + 1) %count))
        {
        v = new vtkCCPDVertex;
        v->Point[0] =
          this->Internal->Polygons[idx]->NewVertices[k]->Point[0];
        v->Point[1] =
          this->Internal->Polygons[idx]->NewVertices[k]->Point[1];
        v->Point[2] =
          this->Internal->Polygons[idx]->NewVertices[k]->Point[2];
        newPoly->Vertices.push_back(v);
        delete this->Internal->Polygons[idx]->NewVertices[k];
        --numNewPoints;
        }
      this->Internal->Polygons[idx]->NewVertices.clear();

      lastPoint[0] = newPoly->Vertices[newPoly->Vertices.size() -1]->Point[0];
      //this->Internal->Polygons[idx]->NewVertices[(subIdx+1)%2]->Point[0];
      lastPoint[1] =newPoly->Vertices[newPoly->Vertices.size() -1]->Point[1];
      //this->Internal->Polygons[idx]->NewVertices[(subIdx+1)%2]->Point[1];
      lastPoint[2] =newPoly->Vertices[newPoly->Vertices.size() -1]->Point[2];
        //this->Internal->Polygons[idx]->NewVertices[(subIdx+1)%2]->Point[2];
      lastPointIdx = idx;
      }
    count = newPoly->Vertices.size();
    if ( vtkMath::Distance2BetweenPoints(newPoly->Vertices[0]->Point,
                                         newPoly->Vertices[count -1]->Point)
         < tolerance)
      {
      delete newPoly->Vertices[count-1];
      newPoly->Vertices.pop_back();
      }
    else
      {
      vtkErrorMacro( << "Last point should be equal to the first" );
      return;
      }

    // check to see that the polygon vertices are in the right order.
    // cross product of p1p2 and p3p2 should point in the same direction
    // as the plane normal. Otherwise, reverse the order
    int flipCount = 0;
    int checkCount = 0;
    unsigned int startV;
    for ( startV = 0; startV+2 < newPoly->Vertices.size(); startV++)
      {
      double *p1 = newPoly->Vertices[startV]->Point;
      double *p2 = newPoly->Vertices[startV+1]->Point;
      double *p3 = newPoly->Vertices[startV+2]->Point;
      double v1[3];
      double v2[3];
      double cross[3];
      v1[0] = p1[0] - p2[0];
      v1[1] = p1[1] - p2[1];
      v1[2] = p1[2] - p2[2];
      v2[0] = p3[0] - p2[0];
      v2[1] = p3[1] - p2[1];
      v2[2] = p3[2] - p2[2];
      vtkMath::Cross(v1,v2,cross);
      double nd = vtkMath::Normalize(cross);
      // only check if the length of the cross product is long
      // enough - otherwise we might be working with points that
      // are all too close together and we might wind up with
      // misleading results
      if ( nd > tolerance )
        {
        if ( vtkMath::Dot(cross,normal) < 0 )
          {
          flipCount++;
          }
        checkCount++;
        }
      }
    // As long as more than half the time we checked we got the answer
    // that we should flip, go ahead and flip it
    if ( flipCount > checkCount/2 )
      {
      vtkstd::reverse(newPoly->Vertices.begin(), newPoly->Vertices.end());
      }
    this->Internal->Polygons.push_back(newPoly);
    }
  this->RemoveEmptyPolygons();
  this->ClearNewVertices();
}

// ----------------------------------------------------------------------------
void vtkClipConvexPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Planes: " << this->Planes << endl;
}
