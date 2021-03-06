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
 *               Copyright (C) 1993 - 2013   Borek Patzak
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

#ifndef RankineMatGrad_h

#include "rankinemat.h"
#include "structuralnonlocalmaterialext.h"
#include "nonlocmatstiffinterface.h"
#include "graddpmaterialextensioninterface.h"
#include "cltypes.h"

#ifdef __OOFEG
 #include "oofeggraphiccontext.h"
 #include "connectivitytable.h"
#endif

///@name Input fields for RankineMatGrad
//@{
#define _IFT_RankineMatGrad_Name "rankmatgrad"
#define _IFT_RankineMatGrad_L "l"
#define _IFT_RankineMatGrad_m "m"
#define _IFT_RankineMatGrad_negligibleDamage "negligible_damage"
//@}

namespace oofem {
/**
 * Gradient rankine material status.
 */
class RankineMatGradStatus : public RankineMatStatus, public GradDpMaterialStatusExtensionInterface
{
protected:
    double kappa_nl;
    double kappa_hat;

public:
    RankineMatGradStatus(int n, Domain *d, GaussPoint *g);
    virtual ~RankineMatGradStatus() { }

    virtual void printOutputAt(FILE *file, TimeStep *tStep);

    // definition
    virtual const char *giveClassName() const { return "RankineMatGradStatus"; }
    virtual classType giveClassID() const { return RankineMatClass; }

    virtual void initTempStatus();
    virtual void updateYourself(TimeStep *tStep);

    void setKappa_nl(double kap) { kappa_nl = kap; }
    void setKappa_hat(double kap) { kappa_hat = kap; }
    double giveKappa_nl() { return kappa_nl; }
    double giveKappa_hat() { return kappa_hat; }
    virtual double giveNonlocalCumulatedStrain() { return nonlocalCumulatedStrain; }
    virtual void setNonlocalCumulatedStrain(double nonlocalCumulatedStrain) { this->nonlocalCumulatedStrain = nonlocalCumulatedStrain; }
};


/**
 * Gradient Rankine material.
 */
class RankineMatGrad : public RankineMat, GradDpMaterialExtensionInterface
{
protected:
    double L;
    double mParam;
    double negligible_damage;

public:
    RankineMatGrad(int n, Domain *d);
    virtual ~RankineMatGrad() {; }

    virtual const char *giveClassName() const { return "RankineMatGrad"; }
    virtual classType giveClassID() const { return RankineMatGradClass; }
    virtual const char *giveInputRecordName() const { return _IFT_RankineMatGrad_Name; }

    virtual IRResultType initializeFrom(InputRecord *ir);
    virtual int hasMaterialModeCapability(MaterialMode mode);
    virtual Interface *giveInterface(InterfaceType t) { if ( t == GradDpMaterialExtensionInterfaceType ) { return static_cast< GradDpMaterialExtensionInterface * >( this ); } else { return NULL; } }

    virtual void giveStiffnessMatrix(FloatMatrix &answer, MatResponseMode rMode, GaussPoint *gp, TimeStep *tStep);

    virtual void givePDGradMatrix_uu(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);
    virtual void givePDGradMatrix_uk(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);
    virtual void givePDGradMatrix_ku(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);
    virtual void givePDGradMatrix_kk(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);
    virtual void givePDGradMatrix_LD(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);
    virtual void giveRealStressVectorGrad(FloatArray &answer1, double &answer2, GaussPoint *gp, const FloatArray &totalStrain, double nonlocalCumulatedStrain, TimeStep *atTime);

    virtual void givePlaneStressStiffMtrx(FloatMatrix & answer, MatResponseMode, GaussPoint * gp,  TimeStep * tStep);
    void givePlaneStressGprime(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);
    void givePlaneStressKappaMatrix(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);
    void giveInternalLength(FloatMatrix &answer, MatResponseMode mode, GaussPoint *gp, TimeStep *tStep);

    virtual void computeCumPlastStrain(double &kappa, GaussPoint *gp, TimeStep *tStep);
    double giveNonlocalCumPlasticStrain(GaussPoint *gp);
    void performPlasticityReturn(GaussPoint *gp, const FloatArray &totalStrain);

    LinearElasticMaterial *giveLinearElasticMaterial() { return linearElasticMaterial; }

    virtual int giveIPValue(FloatArray &answer, GaussPoint *aGaussPoint, InternalStateType type, TimeStep *tStep);
    virtual InternalStateValueType giveIPValueType(InternalStateType type);

protected:
    virtual MaterialStatus *CreateStatus(GaussPoint *gp) const { return new RankineMatGradStatus(1, RankineMat :: domain, gp); }
};
} // end namespace oofem
#define RankineMatGrad_h
#endif
