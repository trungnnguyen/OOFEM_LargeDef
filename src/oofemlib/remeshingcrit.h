/* $Header: /home/cvs/bp/oofem/oofemlib/src/remeshingcrit.h,v 1.7 2003/04/06 14:08:25 bp Exp $ */
/*

                   *****    *****   ******  ******  ***   ***                            
                 **   **  **   **  **      **      ** *** **                             
                **   **  **   **  ****    ****    **  *  **                              
               **   **  **   **  **      **      **     **                               
              **   **  **   **  **      **      **     **                                
              *****    *****   **      ******  **     **         
            
                                                                   
               OOFEM : Object Oriented Finite Element Code                 
                    
                 Copyright (C) 1993 - 2000   Borek Patzak                                       



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


//   ********************************
//   *** CLASS REMESHING CRITERIA ***
//   ********************************

#ifndef remeshingcrit_h 

#include "femcmpnn.h"
#include "compiler.h"
#include "cltypes.h"
#include "interface.h"

class Domain;
class Element;
class TimeStep;
class ErrorEstimator;

/// Type representing the remeshing strategy
enum RemeshingStrategy {NoRemeshing_RS, RemeshingFromCurrentState_RS, RemeshingFromPreviousState_RS};

/**
 The base class for all remeshing criteria.
 The basic task is to evaluate the required mesh density (at nodes) on given domain,
 based on informations provided by the compatible error ertimator. 
 If this task requires the special element algorithms, these should be included using interface concept.

 The remeshing criteria is maintained by the corresponding error estimator. This is mainly due to fact, that is
 necessary for given EE to create compatible RC. In our concept, the EE is responsible. 
*/
class RemeshingCriteria : public FEMComponent {
protected:

 ErrorEstimator *ee;
public:
 /// Constructor
 RemeshingCriteria (int n, ErrorEstimator *e) ;
 /// Destructor
 virtual ~RemeshingCriteria() {}
 /** Returns the required mesh size n given dof manager.
  The mesh density is defined as a required element size 
  (in 1D the element length, in 2D the square from element area).
  @param num dofman  number
  @param tStep time step
  @param relative if zero, then actual density is returned, otherwise the relative density to current is returned.
  */
 virtual double giveRequiredDofManDensity (int num, TimeStep* tStep, int relative=0) = 0;
 /**
   Returns existing mesh size for given dof manager.
   @param num dofMan number
 */
 virtual double giveDofManDensity (int num) = 0;

  /**
  Determines, if the remeshing is needed, and if nedded, the type of strategy used
  */
  virtual RemeshingStrategy giveRemeshingStrategy (TimeStep* tStep) = 0;
 /**
  Estimates the nodal densities.
  @param tStep time step
  */
 virtual int estimateMeshDensities (TimeStep* tStep) = 0;


 /// Returns class name of the receiver.
 const char* giveClassName () const { return "ErrorEstimator" ;}
 /** Returns classType id of receiver.
  @see FEMComponent::giveClassID 
  */
 classType                giveClassID () const { return RemeshingCriteriaClass; }


#ifdef __OOFEG
#endif

protected:
};

#define remeshingcrit_h
#endif





