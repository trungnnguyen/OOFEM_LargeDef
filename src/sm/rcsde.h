/* $Header: /home/cvs/bp/oofem/sm/src/rcsde.h,v 1.5 2003/04/06 14:08:31 bp Exp $ */
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


//   ***************************************************************************************************************
//   *** CLASS ROTATING SMEARED CRACK MODEL WITH TRANSITION TO SCALAR DAMAGE WITH EXPONENTIAL SOFTENING ************
//   ***************************************************************************************************************

#ifndef rcsde_h 

#include "rcm2.h"
#include "cltypes.h"

#define rcsd_Omega 300
#define pscm_SDTransitionCoeff 306
#define RCSDE_DAMAGE_EPS 1.e-4


class GaussPoint ;


//class PlasticSmearedCrackingMaterialStatus : public PerfectlyPlasticMaterialStatus
class RCSDEMaterialStatus : public RCM2MaterialStatus
{
/*
 This class implements associated Material Status to SmearedCrackingMaterail.
 It is atribute of matStatusDictionary at every GaussPoint, for which this material 
 is active.
 DESCRIPTION:
   Idea used there is that we have variables
   describing:
     1) state at previous equilibrium state (variables without temp)
   2) state during searching new equilibrium (variables with temp)
   when we start search new state from previous equilibrium one we copy 
   non-tem variables into temp ones. And after we reach new equilibrium 
   (now decribed by temp variables) we copy tem-var into non-tepm ones
   (see function updateYourself).
   
   variables description:

  TASK:

*/

public:
  enum __rcsdModeType {rcMode, sdMode};

protected:

 double maxEquivStrain, tempMaxEquivStrain;
  double damageCoeff, tempDamageCoeff;
  FloatMatrix Ds0;
  double transitionEps, epsF2;
  __rcsdModeType rcsdMode, tempRcsdMode;
public:

 RCSDEMaterialStatus (int n, Domain*d, GaussPoint* g) ;
 ~RCSDEMaterialStatus ();

 void   printOutputAt (FILE *file, TimeStep* tStep) ;

  double giveTempMaxEquivStrain () {return tempMaxEquivStrain;}
  void   setTempMaxEquivStrain (double val) {tempMaxEquivStrain = val;}
//  double giveDamageStiffCoeff () {return damageStiffCoeff;}
//  void   setDamageStiffCoeff (double val) {damageStiffCoeff = val;}
  double giveTempDamageCoeff () {return tempDamageCoeff;}
  void   setTempDamageCoeff (double val) {tempDamageCoeff = val;}
  const FloatMatrix* giveDs0Matrix () {return &Ds0;}
  void   setDs0Matrix (FloatMatrix& mtrx) {Ds0 = mtrx;}

  double giveTransitionEpsCoeff () {return transitionEps;}
  void   setTransitionEpsCoeff (double val) {transitionEps = val;}
  double giveEpsF2Coeff () {return epsF2;}
  void   setEpsF2Coeff (double val) {epsF2 = val;}

  __rcsdModeType giveTempMode () {return tempRcsdMode;}
  void     setTempMode (__rcsdModeType mode) {tempRcsdMode = mode;}

 // query for non-tem variables (usefull for postprocessing)
  double giveMaxEquivStrain () {return maxEquivStrain;}
  double giveDamageCoeff () {return damageCoeff;}

  __rcsdModeType giveMode () {return rcsdMode;}
 // definition
 const char* giveClassName () const { return "RCSDEMaterialStatus" ;}
  classType             giveClassID () const { return RCSDEMaterialStatusClass; }
 
 virtual void initTempStatus ();
 virtual void updateYourself(TimeStep*); // update after new equilibrium state reached

 // saves current context(state) into stream
 contextIOResultType    saveContext (DataStream* stream, ContextMode mode, void *obj = NULL);
 contextIOResultType    restoreContext(DataStream* stream, ContextMode mode, void *obj = NULL);
 
}; 



class RCSDEMaterial : public RCM2Material
{
/*

 DESCRIPTION
   This class implements a Rotating Crack Model with transition to scalar damage
  for fracture in smeared fashion
 ( only material stiffness modification is required, no changes in 
   mesh topology).
  Model according to Milan Jirasek RC-SD model.

*/

protected:
  double SDTransitionCoeff;
public:
 
 RCSDEMaterial (int n,Domain* d) ;
 ~RCSDEMaterial () ;
 
 // identification and auxiliary functions
 const char*    giveClassName () const { return "RCSDEMaterial" ;}
 classType giveClassID ()         const { return RCSDEMaterialClass;}
 
 // contextIOResultType    saveContext (FILE* stream, void *obj = NULL);
 // contextIOResultType    restoreContext(FILE* stream, void *obj = NULL);
IRResultType initializeFrom (InputRecord* ir);
contextIOResultType    saveContext (DataStream* stream, ContextMode mode, void *obj = NULL);
contextIOResultType    restoreContext(DataStream* stream, ContextMode mode, void *obj = NULL);

 // non-standart - returns time independent material constant
 double   give (int) ; 

 void giveRealStressVector (FloatArray& answer,  MatResponseForm, GaussPoint*, 
               const FloatArray&,TimeStep* );


#ifdef __OOFEG
#endif

 MaterialStatus* CreateStatus (GaussPoint *gp) const {return new RCSDEMaterialStatus (1,domain,gp);}
 
protected: 
  double          computeCurrEquivStrain (GaussPoint*, const FloatArray&, double, TimeStep* );
 // two functions used to initialize and updating temporary variables in
 // gp's status. These variables are used to controll process, when
        // we try to find equlibrium state.

 virtual void giveEffectiveMaterialStiffnessMatrix (FloatMatrix& answer, MatResponseForm form, 
                           MatResponseMode rMode,
                           GaussPoint* gp, TimeStep* atTime);

////
 double          computeDamageCoeff (double, double, double);
 virtual     double giveCrackingModulus (MatResponseMode  rMode, GaussPoint* gp, 
               double crackStrain, int i);
 //virtual     double giveShearRetentionFactor(GaussPoint* gp, double eps_cr, int i);
 virtual     double giveNormalCrackingStress (GaussPoint* gp, double eps_cr, int i);
 virtual     double giveMinCrackStrainsForFullyOpenCrack (GaussPoint* gp, int i);
  //virtual     void   updateStatusForNewCrack( GaussPoint*, int, double);
 virtual     double computeStrength (GaussPoint*, double);
 virtual     int    checkSizeLimit(GaussPoint *gp, double );
////

} ;


#define rcsde_h
#endif



