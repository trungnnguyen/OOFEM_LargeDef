/* $Header: /home/cvs/bp/oofem/oofemlib/src/fei3dhexalin.h,v 1.2 2003/05/19 13:03:57 bp Exp $ */
/*

                   *****    *****   ******  ******  ***   ***                            
                 **   **  **   **  **      **      ** *** **                             
                **   **  **   **  ****    ****    **  *  **                              
               **   **  **   **  **      **      **     **                               
              **   **  **   **  **      **      **     **                                
              *****    *****   **      ******  **     **         
            
                                                                   
               OOFEM : Object Oriented Finite Element Code                 
                    
                 Copyright (C) 1993 - 2002   Borek Patzak                                       



         Czech Technical University, Faculty of Civil Engineering,
     Department of Structural Mechanics, 166 29 Prague, Czech Republic
                                                                               
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                                                                              
*/

//   ***********************************************
//   *** CLASS 3d linear hexahedra interpolation ***
//   ***********************************************


#ifndef fei3dhexalin_h


#include "flotarry.h"
#include "intarray.h"
#include "domain.h"
#include "feinterpol3d.h"

/**
 Class representing implementation of linar hexahedra interpolation class.
*/
class FEI3dHexaLin : public FEInterpolation3d
{

protected:

public:
  FEI3dHexaLin() : FEInterpolation3d (1) {}
 /** 
  Evaluates the array of interpolation functions (shape functions) at given point.
  @param answer contains resulting array of evaluated interpolation functions
  @param lcoords array containing (local) coordinates 
  @param time time
  */
  virtual void evalN (FloatArray& answer, const FloatArray& lcoords, double time);
 /** 
  Evaluates the matrix of derivatives of interpolation functions (shape functions) at given point.
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param matrix contains resulting matrix of derivatives, the member at i,j position contains value of dNi/dxj
  @param coords coordinates of nodes defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void evaldNdx (FloatMatrix&answer, const FloatArray** coords, const FloatArray& lcoords, double time) ;
 /** 
  Evaluates the matrix of derivatives of interpolation functions (shape functions) at given point.
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param matrix contains resulting matrix of derivatives, the member at i,j position contains value of dNi/dxj
  @param nodes array of node numbers defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void evaldNdx (FloatMatrix& answer, Domain* d, IntArray& nodes, const FloatArray& lcoords, double time) {
   const FloatArray *c[8];
   nodes2coords (d, nodes, c, 8);
   evaldNdx (answer, c, lcoords, time);
 }
 /** 
  Evaluates global coordinates from given local ones
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting global coordinates
  @param coords coordinates of nodes defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void local2global (FloatArray& answer, const FloatArray** coords, const FloatArray& lcoords, double time) ;
 /** 
  Evaluates global coordinates from given local ones
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting global coordinates
  @param nodes array of node numbers defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
  virtual void local2global (FloatArray& answer, Domain* d, IntArray& nodes, const FloatArray& lcoords, double time) {
    const FloatArray *c[8];
    nodes2coords (d, nodes, c, 8);
    local2global (answer, c, lcoords, time);
  }
 /** 
  Evaluates local coordinates from given global ones. Returns nonzero if local coordinates are interpolating, 
  zero if extrapolating (nonzero is returned if point is within the element geometry, zero otherwise).
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains evaluated local coordinates
  @param coords coordinates of nodes defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  @return nonzero is returned if point is within the element geometry, zero otherwise
  */
 virtual int  global2local (FloatArray& answer, const FloatArray** coords, const FloatArray& lcoords, double time) ;
 /** 
  Evaluates local coordinates from given global ones. Returns nonzero if local coordinates are interpolating, 
  zero if extrapolating (nonzero is returned if point is within the element geometry, zero otherwise).
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains evaluated local coordinates
  @param nodes array of node numbers defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  @return nonzero is returned if point is within the element geometry, zero otherwise
  */
 virtual int  global2local (FloatArray& answer, Domain* d, IntArray& nodes, const FloatArray& lcoords, double time) {
   const FloatArray *c[8];
   nodes2coords (d, nodes, c, 8);
   return global2local (answer, c, lcoords, time);
 }
 /**
  Evaluates the jacobian of transformation between local and global coordinates.
  */
  virtual double giveTransformationJacobian (const FloatArray** coords, const FloatArray& lcoords, double time);
 /**
  Evaluates the jacobian of transformation between local and global coordinates.
  */
  virtual double giveTransformationJacobian (Domain *d, IntArray& nodes, const FloatArray& lcoords, double time) {
    const FloatArray *c[8];
    nodes2coords (d, nodes, c, 8);
    return giveTransformationJacobian (c, lcoords, time);
 }
    

 /**@name Edge interpolation servises */
 //@{
 /** 
  Evaluates the array of edge interpolation functions (shape functions) at given point.
  @param answer contains resulting array of evaluated interpolation functions
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void edgeEvalN (FloatArray& answer, const FloatArray& lcoords, double time);
 /** 
  Evaluates the matrix of derivatives of edge interpolation functions (shape functions) at given point.
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting matrix of derivatives, the member at i,j position contains value of dNj/dxi
  @param iedge determines the edge number
  @param coords coordinates of nodes defining the interpolation geometry (for the whole element)
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void edgeEvaldNdx (FloatMatrix&answer, int iedge,
			    const FloatArray** coords, const FloatArray& lcoords, double time) ;
 /** 
  Evaluates the matrix of derivatives of edge interpolation functions (shape functions) at given point.
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting matrix of derivatives, the member at i,j position contains value of dNj/dxi
  @param iedge determines the edge number
  @param nodes array of node numbers (for the whole element) defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void edgeEvaldNdx (FloatMatrix&answer, int iedge,
			    Domain* d, IntArray& nodes, const FloatArray& lcoords, double time) {
   const FloatArray *c[8];
   nodes2coords (d, nodes, c, 8);
   edgeEvaldNdx (answer, iedge, c, lcoords, time);
 }
   
 /** 
  Evaluates edge global coordinates from given local ones
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting global coordinates
  @param iedge determines edge number
  @param coords coordinates of nodes defining the interpolation geometry (for the whole element)
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void edgeLocal2global (FloatArray& answer, int iedge,
				const FloatArray** coords, const FloatArray& lcoords, double time);
 /** 
  Evaluates edge global coordinates from given local ones
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting global coordinates
  @param iedge determines edge number
  @param nodes array of node numbers (for the whole element) defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void edgeLocal2global (FloatArray& answer, int iedge,
				Domain* d, IntArray& nodes, const FloatArray& lcoords, double time) {
   const FloatArray *c[8];
   nodes2coords (d, nodes, c, 8);
   edgeLocal2global (answer, iedge, c, lcoords, time);
 }

 /**
  Evaluates the edge jacobian of transformation between local and global coordinates.
  */
 virtual double edgeGiveTransformationJacobian (int iedge, const FloatArray** coords, const FloatArray& lcoords, 
						double time);
  /**
     Evaluates the edge jacobian of transformation between local and global coordinates.
  */
 virtual double edgeGiveTransformationJacobian (int iedge, Domain* d, IntArray& nodes, const FloatArray& lcoords, 
                                                double time) {
   const FloatArray *c[8];
   nodes2coords (d, nodes, c, 8);
   return edgeGiveTransformationJacobian (iedge, c, lcoords, time);
 }

 virtual void computeLocalEdgeMapping (IntArray& edgeNodes, int iedge);
 //@}

 /**@name Surface interpolation services */
 //@{
 /** 
  Evaluates the array of edge interpolation functions (shape functions) at given point.
  @param answer contains resulting array of evaluated interpolation functions
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void surfaceEvalN (FloatArray& answer, const FloatArray& lcoords, double time);
 /** 
  Evaluates the matrix of derivatives of edge interpolation functions (shape functions) at given point.
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting matrix of derivatives, the member at i,j position contains value of dNj/dxi
  @param iedge determines the edge number
  @param nodes array of node numbers (for the whole element) defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 //virtual void surfaceEvaldNdx (FloatMatrix&answer, int iedge, 
 //               Domain* d, IntArray& nodes, const FloatArray& lcoords, double time);
 /** 
  Evaluates edge global coordinates from given local ones
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting global coordinates
  @param iedge determines edge number
  @param nodes array of node numbers (for the whole element) defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void surfaceLocal2global (FloatArray& answer, int iedge,
				   const FloatArray** coords, const FloatArray& lcoords, double time);
 /** 
  Evaluates edge global coordinates from given local ones
  These derivatives are in global coordinate system (where the nodal coordinates are defined) 
  @param answer contains resulting global coordinates
  @param iedge determines edge number
  @param nodes array of node numbers (for the whole element) defining the interpolation geometry
  @param lcoords array containing (local) coordinates 
  @param time time
  */
 virtual void surfaceLocal2global (FloatArray& answer, int iedge,
				   Domain* d, IntArray& nodes, const FloatArray& lcoords, double time) {
   const FloatArray *c[8];
   nodes2coords (d, nodes, c, 8);
   surfaceLocal2global (answer, iedge, c, lcoords, time);
 }   
 /**
  Evaluates the edge jacobian of transformation between local and global coordinates.
  */
 virtual double surfaceGiveTransformationJacobian (int isurf, const FloatArray** coords, const FloatArray& lcoords,
						   double time);
 /**
  Evaluates the edge jacobian of transformation between local and global coordinates.
  */
 virtual double surfaceGiveTransformationJacobian (int isurf, Domain* d, IntArray& nodes, const FloatArray& lcoords, 
                                                   double time) {
   const FloatArray *c[8];
   nodes2coords (d, nodes, c, 8);
   return surfaceGiveTransformationJacobian (isurf, c, lcoords, time);
 }
 virtual void computeLocalSurfaceMapping (IntArray& edgeNodes, int iedge);
 //@}

protected:
  double edgeComputeLength (IntArray& edgeNodes, const FloatArray** coords);
  void   giveDerivativeKsi (FloatArray& dx, double v, double w);
  void   giveDerivativeEta (FloatArray& dy, double u, double w);
  void   giveDerivativeDzeta (FloatArray& dz, double u, double v);
public:
  void   giveJacobianMatrixAt (FloatMatrix& jacobianMatrix, const FloatArray** coords, const FloatArray& lcoords) ;
} ;




#define fei3dhexalin_h
#endif





