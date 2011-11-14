/*
 *
 *                 #####    #####   ######  ######  ###   ###
 *               ##   ##  ##   ##  ##      ##      ## ### ##
 *              ##   ##  ##   ##  ####    ####    ##  #  ##
 *             ##   ##  ##   ##  ##      ##      ##     ##
 *            ##   ##  ##   ##  ##      ##      ##     ##
 *            #####    #####   ##      ######  ##     ##
 *
 *
 *             OOFEM : Object Oriented Finite Element Code
 *
 *               Copyright (C) 1993 - 2011   Borek Patzak
 *
 *
 *
 *       Czech Technical University, Faculty of Civil Engineering,
 *   Department of Structural Mechanics, 166 29 Prague, Czech Republic
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "hydratingconcretemat.h"
#include "domain.h"
#include "flotmtrx.h"
#include "gausspnt.h"
#include "timestep.h"
#ifndef __MAKEDEPEND
 #include <stdlib.h>
#endif
#include "contextioerr.h"

namespace oofem {
HydratingConcreteMat :: HydratingConcreteMat(int n, Domain *d) : IsotropicHeatTransferMaterial(n,d)
{
   /// constructor
   maxModelIntegrationTime = 0.;
   minModelTimeStepIntegrations = 0;
}
  
HydratingConcreteMat :: ~HydratingConcreteMat()
{
     /// destructor
}

  
IRResultType
HydratingConcreteMat :: initializeFrom(InputRecord *ir)
{
    const char *__proc = "initializeFrom"; // Required by IR_GIVE_FIELD macro
    IRResultType result;                            // Required by IR_GIVE_FIELD macro

    // set conductivity k and capacity c
    IsotropicHeatTransferMaterial :: initializeFrom(ir);
    
    referenceTemperature = 25.;
    IR_GIVE_OPTIONAL_FIELD(ir, referenceTemperature, IFT_HydratingConcreteMat_referenceTemperature, "referencetemperature"); // Macro
    
    IR_GIVE_FIELD(ir, hydrationModelType, IFT_HydratingConcreteMat_hydrationModelType, "hydrationmodeltype");
    
    /*hydrationModelType==1:exponential hydration model, summarized in A.K. Schindler and K.J. Folliard: Heat of Hydration Models for Cementitious 
        Materials, ACI Materials Journal, 2005
    hydrationModelType==2: affinity hydration model inspired by Miguel Cervera and Javier Oliver and Tomas Prato:Thermo-chemo-mechanical model 
        for concrete. I: Hydration and aging, Journal of Engineering Mechanics ASCE, 125(9), 1018-1027, 1999
    */
    if (hydrationModelType == 1){
        IR_GIVE_FIELD(ir, tau, IFT_HydratingConcreteMat_tau, "tau");// [s]
        IR_GIVE_FIELD(ir, beta, IFT_HydratingConcreteMat_beta, "beta");// [-]
        IR_GIVE_FIELD(ir, DoHInf, IFT_HydratingConcreteMat_DoHInf, "dohinf");
    } else if (hydrationModelType == 2){
        IR_GIVE_FIELD(ir, B1, IFT_HydratingConcreteMat_B1, "b1");// [1/s]
        IR_GIVE_FIELD(ir, B2, IFT_HydratingConcreteMat_B2, "b2");
        IR_GIVE_FIELD(ir, eta, IFT_HydratingConcreteMat_eta, "eta");
        IR_GIVE_FIELD(ir, DoHInf, IFT_HydratingConcreteMat_DoHInf, "dohinf");
    } else {
        OOFEM_ERROR2("Unknown hdyration model type %d", hydrationModelType);
    }
    
    IR_GIVE_FIELD(ir, Qpot, IFT_HydratingConcreteMat_qpot, "qpot");// [1/s]
    
    maxModelIntegrationTime = 36000;
    IR_GIVE_OPTIONAL_FIELD(ir, maxModelIntegrationTime, IFT_maxModelIntegrationTime, "maxmodelintegrationtime"); // Macro
    
    minModelTimeStepIntegrations = 30.;
    IR_GIVE_OPTIONAL_FIELD(ir, minModelTimeStepIntegrations, IFT_minModelTimeStepIntegrations, "minmodeltimestepintegrations"); // Macro
    
    
    conductivityType=0;
    IR_GIVE_OPTIONAL_FIELD(ir, conductivityType, IFT_HydratingConcreteMat_conductivitytype, "conductivitytype"); // Macro
    capacityType=0;
    IR_GIVE_OPTIONAL_FIELD(ir, capacityType, IFT_HydratingConcreteMat_capacitytype, "capacitytype"); // Macro
    densityType=0;
    IR_GIVE_OPTIONAL_FIELD(ir, densityType, IFT_HydratingConcreteMat_densitytype, "densitytype"); // Macro
    
    activationEnergy=38400; //J/mol/K
    IR_GIVE_OPTIONAL_FIELD(ir, activationEnergy, IFT_HydratingConcreteMat_activationEnergy, "activationenergy"); // Macro

    IR_GIVE_FIELD(ir, massCement, IFT_HydratingConcreteMat_massCement, "masscement");
    
    reinforcementDegree = 0.;
    IR_GIVE_OPTIONAL_FIELD(ir, reinforcementDegree, IFT_HydratingConcreteMat_reinforcementDegree, "reinforcementdegree"); // Macro
    
    return IRRT_OK;
}

///returns hydration power [W/m3 of concrete]
void
HydratingConcreteMat :: computeInternalSourceVector(FloatArray &val, GaussPoint *gp, TimeStep *atTime, ValueModeType mode)
{
  HydratingConcreteMatStatus *ms = (HydratingConcreteMatStatus *) this->giveStatus(gp);
  val.resize(1);
  if ( mode == VM_Total ) {
     //for nonlinear solver, return the last value even no time has elapsed
            if ( atTime->giveTargetTime() != ms->lastCallTime ) {
                val.at(1) = ms->GivePower( atTime );
            } else {
                val.at(1) = ms->power;
            }
        } else {
            OOFEM_ERROR2( "Undefined mode %s\n", __ValueModeTypeToString(mode) );
        }

}

void
HydratingConcreteMat :: updateInternalState(const FloatArray &vec, GaussPoint *gp, TimeStep *atTime)
{
    HydratingConcreteMatStatus *ms = ( HydratingConcreteMatStatus * ) this->giveStatus(gp);
    ms->letTempStateVectorBe(vec);
}

double
HydratingConcreteMat :: giveCharacteristicValue(MatResponseMode mode, GaussPoint *gp, TimeStep *atTime)
{
    if ( mode == Capacity ) {
        return ( giveConcreteCapacity(gp) * giveConcreteDensity(gp) );
    } else if ( mode == IntSource ) { //for nonlinear solver, return dHeat/dTemperature
        //it suffices to compute derivative of Arrhenius equation
        double lastEquilibratedTemperature = ( ( TransportMaterialStatus * ) giveStatus(gp) )->giveStateVector().at(1);
        double krate, EaOverR, val;
        HydratingConcreteMatStatus *ms = ( HydratingConcreteMatStatus * ) this->giveStatus(gp);
        
        EaOverR = 1000. * this->activationEnergy / 8.314;

        if ( !atTime->isTheFirstStep () ) {
            krate = exp( -EaOverR * ( 1. / ( ms->giveTempStateVector().at(1) + 273.15 ) -  1. / ( lastEquilibratedTemperature + 273.15 ) ) );
        } else {
            krate = 1.;
        }

        val = EaOverR * krate / ( ms->giveTempStateVector().at(1) + 273.15 ) / ( ms->giveTempStateVector().at(1) + 273.15 );

        return val;
    } else {
        OOFEM_ERROR2( "giveCharacteristicValue : unknown mode (%s)\n", __MatResponseModeToString(mode) );
    }

    return 0.;  
}

double HydratingConcreteMat :: giveConcreteConductivity(GaussPoint *gp) {
    HydratingConcreteMatStatus *ms = ( HydratingConcreteMatStatus * ) this->giveStatus(gp);
    double conduct;

    if ( conductivityType == 0 ) { //given from input file
        conduct = IsotropicHeatTransferMaterial :: give('k', gp);
    } else if ( conductivityType == 1 ) { //compute according to Ruiz, Schindler, Rasmussen. Kim, Chang: Concrete temperature modeling and strength prediction using maturity concepts in the FHWA HIPERPAV software, 7th international conference on concrete pavements, Orlando (FL), USA, 2001
        conduct = IsotropicHeatTransferMaterial :: give('k', gp) * ( 1.0 - 0.33/1.33 * ms->giveDoHActual() );
    } else {
        OOFEM_ERROR2("Unknown conductivityType %d\n", conductivityType);
    }

    //Parallel Voigt model, 20 W/m/K for steel
    conduct = conduct * ( 1. - this->reinforcementDegree ) + 20. * this->reinforcementDegree;

    if ( conduct < 0.3 || conduct > 5 ) {
        OOFEM_WARNING2("Weird concrete thermal conductivity %f W/m/K\n", conduct);
    }

    return conduct;
}

//normally it returns J/kg/K of concrete
double HydratingConcreteMat :: giveConcreteCapacity(GaussPoint *gp) {
    double capacityConcrete;

    if ( capacityType == 0 ) { //given from OOFEM input file
        capacityConcrete = IsotropicHeatTransferMaterial :: give('c', gp);
    } else if ( capacityType == 1 ) { ////calculate from 5-component model
        capacityConcrete = 1111.;
    } else {
        OOFEM_ERROR2("Unknown capacityType %d\n", capacityType);
    }

    //Parallel Voigt model, 500 J/kg/K for steel
    capacityConcrete = capacityConcrete * ( 1. - this->reinforcementDegree ) + 500. * this->reinforcementDegree;

    if ( capacityConcrete < 500 || capacityConcrete > 2000 ) {
        OOFEM_WARNING2("Weird concrete heat capacity %f J/kg/K\n", capacityConcrete);
    }

    return capacityConcrete;
}

double HydratingConcreteMat :: giveConcreteDensity(GaussPoint *gp) {
    double concreteBulkDensity;

    if ( densityType == 0 ) { //get from input file
        concreteBulkDensity = IsotropicHeatTransferMaterial :: give('d', gp);
    } else if ( densityType == 1 ) { //calculate from 5-component model
        concreteBulkDensity = 1111.;
    } else {
        OOFEM_ERROR2("Unknown densityType %d\n", densityType);
    }

    //Parallel Voigt model, 7850 kg/m3 for steel
    concreteBulkDensity = concreteBulkDensity * ( 1. - this->reinforcementDegree ) + 7850. * this->reinforcementDegree;

    if ( concreteBulkDensity < 1000 || concreteBulkDensity > 4000 ) {
        OOFEM_WARNING2("Weird concrete density %f kg/m3\n", concreteBulkDensity);
    }

    return concreteBulkDensity;
}

contextIOResultType
HydratingConcreteMat :: saveContext(DataStream *stream, ContextMode mode, void *obj)
// saves full status for this material, also invokes saving
// for sub-objects of this.
{
    contextIOResultType iores;

    // write parent data
    if ( ( iores = TransportMaterial :: saveContext(stream, mode, obj) ) != CIO_OK ) {
        THROW_CIOERR(iores);
    }

    // save hydration model data - maybe should check hydration option?
//     if ( ( iores = HydrationModelInterface :: saveContext(stream, mode, obj) ) != CIO_OK ) {
//         THROW_CIOERR(iores);
//     }

    return CIO_OK;
}

contextIOResultType
HydratingConcreteMat :: restoreContext(DataStream *stream, ContextMode mode, void *obj)
// restores full status for this material, also invokes restoring for sub-objects of this.
{
    contextIOResultType iores;

    // read parent data
    if ( ( iores = TransportMaterial :: restoreContext(stream, mode, obj) ) != CIO_OK ) {
        THROW_CIOERR(iores);
    }

    // read hydration model data - maybe should check hydration option?
//     if ( ( iores = HydrationModelInterface :: restoreContext(stream, mode, obj) ) != CIO_OK ) {
//         THROW_CIOERR(iores);
//     }

    return CIO_OK;
}

int
HydratingConcreteMat :: giveIPValue(FloatArray &answer, GaussPoint *gp, InternalStateType type, TimeStep *atTime)
{
    // printf ("IP %d::giveIPValue, IST %d", giveNumber(), type);
    if ( type == IST_HydrationDegree ) {
        HydratingConcreteMatStatus* status = (HydratingConcreteMatStatus*) this -> giveStatus (gp);
        answer.resize(1);
        answer.at(1) = status->giveDoHActual();
        //else answer.at(1) = 0;
        return 1;
    } else {
        return TransportMaterial :: giveIPValue(answer, gp, type, atTime);
    }
}

InternalStateValueType
HydratingConcreteMat :: giveIPValueType(InternalStateType type)
{
    if ( type == IST_HydrationDegree ) {
        return ISVT_SCALAR;
    } else {
        return TransportMaterial :: giveIPValueType(type);
    }
}

int
HydratingConcreteMat :: giveIntVarCompFullIndx(IntArray &answer, InternalStateType type, MaterialMode mmode)
{
    if ( ( type == IST_HydrationDegree ) ) {
        answer.resize(1);
        answer.at(1) = 1;
        return 1;
    } else {
        return TransportMaterial :: giveIntVarCompFullIndx(answer, type, mmode);
    }
}

int
HydratingConcreteMat :: giveIPValueSize(InternalStateType type, GaussPoint *aGaussPoint)
{
    if ( type == IST_HydrationDegree ) {
        return 1;
    } else {
        return TransportMaterial :: giveIPValueSize(type, aGaussPoint);
    }
}

MaterialStatus *
HydratingConcreteMat :: CreateStatus(GaussPoint *gp) const
{
    return new HydratingConcreteMatStatus(1, domain, gp);
}


HydratingConcreteMatStatus :: HydratingConcreteMatStatus(int n, Domain *d, GaussPoint *g) : TransportMaterialStatus(n, d, g)
{
  //constructor 
  power = 0.;
  lastCallTime = -1.e6; //start from begining (the LastCall in the beginning is -1.e6)
  degreeOfHydration = 0.;
  lastDegreeOfHydration = 0;
}

HydratingConcreteMatStatus :: ~HydratingConcreteMatStatus()
{
  //destructor
}


double HydratingConcreteMatStatus :: GivePower( TimeStep *atTime) 
{
  double castingTime = this->gp->giveMaterial()->giveCastingTime();
  HydratingConcreteMat *mat = (HydratingConcreteMat *) this->gp->giveMaterial();
  double targTime = atTime->giveTargetTime();
  double intrinsicTime = atTime->giveIntrinsicTime(); //where heat power is evaluated
  double time = lastCallTime;
  double timeStep;
  double affinity;
  double temp;
  
  
  //do not calculate anything before casting time
  if ( targTime - castingTime <= 0 ) {
        power = 0.;
        lastCallTime = castingTime;
        lastIntrinsicTime = intrinsicTime;
        return 0.;
    }
    
    lastTargTime = targTime;
  
    if ( targTime < lastCallTime ) {
        printf("Cannot go backwards in hydration, TargTime = %f s < LastCallTime = %f s\n", targTime, lastCallTime);
        exit(0);
    }
    
    if (atTime->giveNumber() == 0){
        lastCallTime = intrinsicTime;
        lastIntrinsicTime = intrinsicTime;
        return 0;
    }
    
    //determine timeStep for integration
    timeStep = (intrinsicTime - time) / mat->minModelTimeStepIntegrations;
    if(timeStep > mat->maxModelIntegrationTime){
        timeStep = mat->maxModelIntegrationTime;
    }
    
    //integration loop through hydration model at a given TimeStep
    while (time<intrinsicTime) {
        if(time+timeStep > intrinsicTime){
            timeStep = intrinsicTime - time;
        } else {
            time += timeStep;
        }
        if (mat->hydrationModelType == 1){//exponential affinity hydration model
            temp = pow(mat->tau/time, mat->beta);
            if(time == 0.){
                affinity = 0.;
            } else {
                affinity = mat->DoHInf / time * mat->beta * temp * exp(-temp);
            }
            degreeOfHydration += affinity*timeStep*scaleTemperature();
        } else if (mat->hydrationModelType == 2){//affinity hydration model inspired by Miguel Cervera et al.
            affinity = mat->B1*(mat->B2/mat->DoHInf + degreeOfHydration)*(mat->DoHInf-degreeOfHydration)*exp(-mat->eta*degreeOfHydration/mat->DoHInf);
            //scale from 25C to arbitrary temperature
            affinity *= scaleTemperature();
            degreeOfHydration += affinity*timeStep;
        } else {
            OOFEM_ERROR2("Unknown hydration model type %d", mat->hydrationModelType);
        }
    
    lastCallTime = time;
    }
    
    power = mat->Qpot * (degreeOfHydration - lastDegreeOfHydration) / (intrinsicTime - lastIntrinsicTime );
    power *= 1000*mat->massCement; // W/m3 of concrete
    
    lastDegreeOfHydration = degreeOfHydration;
    lastIntrinsicTime = intrinsicTime;
 
    return power;
}

double HydratingConcreteMatStatus :: scaleTemperature(void){
    HydratingConcreteMat *mat = (HydratingConcreteMat *) this->gp->giveMaterial();
    return exp( mat->activationEnergy/8.314* (1./(273.15+mat->referenceTemperature)-1./(273.15+this->giveStateVector().at(1))) );
}

double HydratingConcreteMatStatus :: giveDoHActual(void)
{
 return degreeOfHydration; 
}


void
HydratingConcreteMatStatus :: updateYourself(TimeStep *atTime) {
    TransportMaterialStatus :: updateYourself(atTime);
};

void
HydratingConcreteMatStatus :: printOutputAt(FILE *file, TimeStep *atTime)
{
    HydratingConcreteMat *mat = ( HydratingConcreteMat * ) this->gp->giveMaterial();
    TransportMaterialStatus :: printOutputAt(file, atTime);
    fprintf(file, "   status {");
    fprintf( file, "IntrinsicTime %e  DoH %f HeatPower %f [W/m3 of concrete] conductivity %f  capacity %f  density %f", atTime->giveIntrinsicTime(), this->giveDoHActual(), this->power, mat->giveConcreteConductivity(this->gp), mat->giveConcreteCapacity(this->gp), mat->giveConcreteDensity(this->gp) );
    fprintf(file, "}\n");
}

} // end namespace oofem