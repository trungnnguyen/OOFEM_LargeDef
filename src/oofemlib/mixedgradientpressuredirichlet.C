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


#include "mixedgradientpressuredirichlet.h"
#include "dofiditem.h"
#include "dofmanager.h"
#include "dof.h"
#include "valuemodetype.h"
#include "floatarray.h"
#include "floatmatrix.h"
#include "engngm.h"
#include "node.h"
#include "activedof.h"
#include "masterdof.h"
#include "classfactory.h"
#include "sparsemtrxtype.h"
#include "sparsemtrx.h"
#include "sparselinsystemnm.h"
#include "dynamicinputrecord.h"

namespace oofem {

REGISTER_BoundaryCondition( MixedGradientPressureDirichlet );

MixedGradientPressureDirichlet :: MixedGradientPressureDirichlet(int n, Domain *d) : MixedGradientPressureBC(n,d)
{
    // The unknown volumetric strain
    voldman = new Node(1, d);
    voldman->appendDof(new MasterDof(1, voldman, (DofIDItem)d->giveNextFreeDofID()));

    int nsd = d->giveNumberOfSpatialDimensions();
    int components = (nsd+1)*nsd/2;
    // The prescribed strains.
    devdman = new Node(2, d);
    for (int i = 0; i < components; i++) {
        // Just putting in X_i id-items since they don't matter.
        // These don't actually need to be active, they are masterdofs with prescribed values, its
        // easier to just have them here rather than trying to make another Dirichlet boundary condition.
        devdman->appendDof(new ActiveDof(i+1, devdman, this->giveNumber(), (DofIDItem)d->giveNextFreeDofID() ));
    }
}


MixedGradientPressureDirichlet :: ~MixedGradientPressureDirichlet()
{
    delete voldman;
    delete devdman;
}


Dof *MixedGradientPressureDirichlet :: giveVolDof()
{
    return voldman->giveDof(1);
}


int MixedGradientPressureDirichlet :: giveNumberOfInternalDofManagers()
{
    return 2;
}


DofManager *MixedGradientPressureDirichlet :: giveInternalDofManager(int i)
{
    if (i == 1) {
        return this->voldman;
    } else {
        return this->devdman;
    }
}


int MixedGradientPressureDirichlet :: giveNumberOfMasterDofs(ActiveDof *dof)
{
    if (this->isDevDof(dof))
        return 1;
    return devdman->giveNumberOfDofs() + 1;
}


Dof *MixedGradientPressureDirichlet :: giveMasterDof(ActiveDof *dof, int mdof)
{
    if (this->isDevDof(dof))
        return NULL;
    if (mdof == 1) {
        return voldman->giveDof(1);
    } else {
        return devdman->giveDof(mdof-1);
    }
}


void MixedGradientPressureDirichlet :: computeDofTransformation(ActiveDof *dof, FloatArray &masterContribs)
{
    DofIDItem id = dof->giveDofID();
    FloatArray *coords = dof->giveDofManager()->giveCoordinates();

    FloatArray dx;
    dx.beDifferenceOf(*coords, this->centerCoord);

    int nsd = dx.giveSize(); // Number of spatial dimensions

    masterContribs.resize(devdman->giveNumberOfDofs()+1);

    if ( id == D_u || id == V_u ) {
        masterContribs.at(1) = dx.at(1)/3.0;      // d_vol
        if (nsd == 2) {
            masterContribs.at(2) = dx.at(1);      // d_dev_11
            masterContribs.at(3) = 0.0;           // d_dev_22
            masterContribs.at(4) = dx.at(2)/2.0;  // gamma_12
        } else if (nsd == 3) {
            masterContribs.at(2) = dx.at(1);      // d_dev_11
            masterContribs.at(3) = 0.0;           // d_dev_22
            masterContribs.at(3) = 0.0;           // d_dev_33
            masterContribs.at(4) = 0.0;           // gamma_23
            masterContribs.at(5) = dx.at(3)/2.0;  // gamma_13
            masterContribs.at(6) = dx.at(2)/2.0;  // gamma_12
        }
    } else if ( id == D_v || id == V_v ) {
        masterContribs.at(1) = dx.at(2)/3.0;      // d_vol
        if (nsd == 2) {
            masterContribs.at(2) = 0.0;           // d_dev_11
            masterContribs.at(3) = dx.at(2);      // d_dev_22
            masterContribs.at(4) = dx.at(1)/2.0;  // gamma_12
        } else if (nsd == 3) {
            masterContribs.at(2) = 0.0;           // d_dev_11
            masterContribs.at(3) = dx.at(2);      // d_dev_22
            masterContribs.at(4) = 0.0;           // d_dev_33
            masterContribs.at(5) = dx.at(3)/2.0;  // gamma_23
            masterContribs.at(6) = 0.0;           // gamma_13
            masterContribs.at(7) = dx.at(1)/2.0;  // gamma_12
        }
    } else if ( id == D_w || id == V_w ) { // 3D only:
        masterContribs.at(1) = dx.at(3)/3.0;  // d_vol
        masterContribs.at(2) = 0.0;           // d_dev_11
        masterContribs.at(3) = 0.0;           // d_dev_22
        masterContribs.at(3) = dx.at(3);      // d_dev_33
        masterContribs.at(4) = dx.at(2)/2.0;  // gamma_23
        masterContribs.at(4) = dx.at(1)/2.0;  // gamma_13
        masterContribs.at(4) = 0.0;           // gamma_12
    } else {
        OOFEM_ERROR("MixedGradientPressureDirichlet :: computeDofTransformation - Incompatible id on subjected dof\n");
    }
}


double MixedGradientPressureDirichlet :: giveUnknown(double vol, const FloatArray &dev, ValueModeType mode, TimeStep *tStep, ActiveDof *dof)
{
    DofIDItem id = dof->giveDofID();
    FloatArray *coords = dof->giveDofManager()->giveCoordinates();

    if ( coords == NULL || coords->giveSize() != this->centerCoord.giveSize() ) {
        OOFEM_ERROR2("MixedGradientPressureDirichlet :: give - Size of coordinate system different from center coordinate (%d) in b.c.", this->centerCoord.giveSize());
    }

    FloatArray dx;
    dx.beDifferenceOf(*coords, this->centerCoord);

    int nsd = dx.giveSize(); // Number of spatial dimensions

    double dev11, dev22, dev33 = 0., dev12, dev23 = 0., dev13 = 0.;
    if (nsd == 2) {
        dev11 = dev.at(1);
        dev22 = dev.at(2);
        dev12 = dev.at(3)*0.5;
    } else /*if (nsd == 3)*/ {
        dev11 = dev.at(1);
        dev22 = dev.at(2);
        dev33 = dev.at(3);
        dev23 = dev.at(4)*0.5;
        dev13 = dev.at(5)*0.5;
        dev12 = dev.at(6)*0.5;
    }

    double val;
    if ( id == D_u || id == V_u ) {
        val = dx.at(1)/3.0*vol;
        if (nsd >= 2) val += dx.at(1)*dev11 + dx.at(2)*dev12;
        if (nsd == 3) val += dx.at(3)*dev13;
    } else if ( id == D_v || id == V_v ) {
        val = dx.at(2)/3.0*vol + dx.at(1)*dev12 + dx.at(2)*dev22;
        if (nsd == 3) val += dx.at(3)*dev23;
    } else /*if ( id == D_w || id == V_w )*/ { // 3D only:
        val = dx.at(3)/3.0*vol + dx.at(1)*dev13 + dx.at(2)*dev23 + dx.at(3)*dev33;
    }
    return val;
}


void MixedGradientPressureDirichlet :: computeFields(FloatArray &sigmaDev, double &vol, EquationID eid, TimeStep *tStep)
{
    EngngModel *emodel = this->giveDomain()->giveEngngModel();
    FloatArray tmp;
    vol = this->giveVolDof()->giveUnknown(VM_Total, tStep);
    int npeq = emodel->giveNumberOfDomainEquations(this->giveDomain()->giveNumber(), EModelDefaultPrescribedEquationNumbering());
    // sigma = residual (since we use the slave dofs) = f_ext - f_int
    sigmaDev.resize(npeq);
    sigmaDev.zero();
    emodel->assembleVector(sigmaDev, tStep, eid, InternalForcesVector, VM_Total, EModelDefaultPrescribedEquationNumbering(), this->domain);
    tmp.resize(npeq);
    tmp.zero();
    emodel->assembleVector(sigmaDev, tStep, eid, ExternalForcesVector, VM_Total, EModelDefaultPrescribedEquationNumbering(), this->domain);
    sigmaDev.subtract(tmp);
    // Divide by the RVE-volume
    sigmaDev.times(1.0/this->domainSize());
    // Above, full sigma = sigmaDev - p*I is assembled, so to obtain the deviatoric part we add p to the diagonal part.
    int nsd = this->domain->giveNumberOfSpatialDimensions();
    for (int i = 1; i <= nsd; ++i ) {
        sigmaDev.at(i) += this->pressure;
    }
}


void MixedGradientPressureDirichlet :: computeTangents(
    FloatMatrix &Ed, FloatArray &Ep, FloatArray &Cd, double &Cp, EquationID eid, TimeStep *tStep)
{
    double size = this->domainSize();
    // Fetch some information from the engineering model
    EngngModel *rve = this->giveDomain()->giveEngngModel();
    ///@todo Get this from engineering model
    SparseLinearSystemNM *solver = classFactory.createSparseLinSolver(ST_Petsc, this->domain, this->domain->giveEngngModel());// = rve->giveLinearSolver();
    SparseMtrx *Kff, *Kfp, *Kpf, *Kpp;
    SparseMtrxType stype = SMT_PetscMtrx;// = rve->giveSparseMatrixType();
    EModelDefaultEquationNumbering fnum;
    EModelDefaultPrescribedEquationNumbering pnum;

    // Set up and assemble tangent FE-matrix which will make up the sensitivity analysis for the macroscopic material tangent.
    Kff = classFactory.createSparseMtrx(stype);
    Kfp = classFactory.createSparseMtrx(stype);
    Kpf = classFactory.createSparseMtrx(stype);
    Kpp = classFactory.createSparseMtrx(stype);
    if ( !Kff ) {
        OOFEM_ERROR2("MixedGradientPressureDirichlet :: computeTangents - Couldn't create sparse matrix of type %d\n", stype);
    }
    Kff->buildInternalStructure( rve, 1, eid, fnum, fnum );
    Kfp->buildInternalStructure( rve, 1, eid, fnum, pnum );
    Kpf->buildInternalStructure( rve, 1, eid, pnum, fnum );
    Kpp->buildInternalStructure( rve, 1, eid, pnum, pnum );
    rve->assemble(Kff, tStep, eid, StiffnessMatrix, fnum, fnum, this->domain );
    rve->assemble(Kfp, tStep, eid, StiffnessMatrix, fnum, pnum, this->domain );
    rve->assemble(Kpf, tStep, eid, StiffnessMatrix, pnum, fnum, this->domain );
    rve->assemble(Kpp, tStep, eid, StiffnessMatrix, pnum, pnum, this->domain );

    // Setup up indices and locations
    int neq = Kff->giveNumberOfRows();
    int npeq = Kpf->giveNumberOfRows();

    // Indices and such of internal dofs
    int dvol_eq = this->giveVolDof()->giveEqn();
    int ndev = this->devGradient.giveSize();
    // Matrices and arrays for sensitivities
    FloatMatrix ddev_pert(ndev,npeq); // In fact, npeq should most likely equal ndev
    FloatMatrix rhs_d(neq,npeq); // RHS for d_dev [d_dev11, d_dev22, d_dev12] in 2D
    FloatMatrix s_d(neq,npeq); // Sensitivity fields for d_dev
    FloatArray rhs_p(neq); // RHS for pressure
    FloatArray s_p(neq); // Sensitivity fields for p

    // Unit pertubations for d_dev
    ddev_pert.zero();
    for (int i = 1; i <= ndev; ++i) {
        int eqn = this->devdman->giveDof(i)->__givePrescribedEquationNumber();
        ddev_pert.at(eqn, i) = -1.0; // Minus sign for moving it to the RHS
    }
    Kfp->times(ddev_pert, rhs_d);

    // Sensitivity analysis for p (already in rhs, just set value directly)
    rhs_p.zero();
    rhs_p.at(dvol_eq) = -1.0; // dp = -1.0 (unit size)

    // Solve all sensitivities
    solver->solve(Kff,&rhs_p,&s_p);
    solver->solve(Kff,rhs_d,s_d);

    // Sensitivities for d_vol is solved for directly;
    Cp = s_p.at(dvol_eq);
    Cd.resize(ndev);
    for (int i = 1; i <= ndev; ++i) {
        Cd.at(i) = s_d.at(dvol_eq, i); // Copy over relevant row from solution
    }

    // Sensitivities for d_dev is obtained as reactions forces;
    Kpf->times(s_p, Ep);

    FloatMatrix tmpMat;
    Kpp->times(ddev_pert, tmpMat);
    Kpf->times(s_d, Ed);

    Ed.subtract(tmpMat);

    Ed.times(1.0/size);

    // Not sure if i actually need to do this part, but the obtained tangents are to dsigma/dp, not dsigma_dev/dp, so they need to be corrected;
    int nsd = this->domain->giveNumberOfSpatialDimensions();
    for (int i = 1; i <= nsd; ++i) {
        Ep.at(i) += 1.0;
        Cd.at(i) += 1.0;
    }
#if 0
    // Makes Ed 4th order deviatoric, in Voigt form (!)
    for (int i = 1; i <= Ed.giveNumberOfColumns(); ++i) {
        // Take the mean of the "diagonal" part of each column
        double mean = 0.0;
        for (int j = 1; j <= nsd; ++j) {
            mean += Ed.at(i,j);
        }
        mean /= (double)nsd;
        // And subtract it from that column
        for (int j = 1; j <= nsd; ++j) {
            Ed.at(i,j) -= mean;
        }
    }
#endif

    delete Kff;
    delete Kfp;
    delete Kpf;
    delete Kpp;
    
    delete solver; ///@todo Remove this when solver is taken from engngmodel
}


double MixedGradientPressureDirichlet :: giveUnknown(PrimaryField &field, ValueModeType mode, TimeStep *tStep, ActiveDof *dof)
{
    if (this->isDevDof(dof)) {
        return this->devGradient(dof->giveNumber());
    }
    return this->giveUnknown(this->giveVolDof()->giveUnknown(field, mode, tStep), this->devGradient, mode, tStep, dof);
}


double MixedGradientPressureDirichlet :: giveUnknown(ValueModeType mode, TimeStep *tStep, ActiveDof *dof)
{
    if (this->isDevDof(dof)) {
        return this->devGradient(dof->giveNumber());
    }
    return this->giveUnknown(this->giveVolDof()->giveUnknown(mode, tStep), this->devGradient, mode, tStep, dof);
}


void MixedGradientPressureDirichlet :: setPrescribedDeviatoricGradientFromVoigt(const FloatArray &t)
{
    devGradient = t;
}


void MixedGradientPressureDirichlet :: assembleVector(FloatArray &answer, TimeStep *tStep, EquationID eid,
                    CharType type, ValueModeType mode, const UnknownNumberingScheme &s, FloatArray *eNorms)
{
    if (type != ExternalForcesVector)
        return;

    if (eid != EID_MomentumBalance_ConservationEquation && eid != EID_MomentumBalance)
        return;

    Dof *vol = this->giveVolDof();
    int vol_loc = vol->giveEquationNumber(s);
    if (vol_loc) {
        double rve_size = this->domainSize();
        answer.at(vol_loc) -= rve_size*pressure; // Note the negative sign (pressure as opposed to mean stress)
        if ( eNorms ) eNorms->at(vol->giveDofID()) = rve_size*pressure*rve_size*pressure;
    }
}


bool MixedGradientPressureDirichlet :: isPrimaryDof(ActiveDof *dof)
{
    return this->isDevDof(dof);
}


double MixedGradientPressureDirichlet :: giveBcValue(ActiveDof *dof, ValueModeType mode, TimeStep *tStep)
{
    if (this->isDevDof(dof)) {
        return this->devGradient(dof->giveNumber());
    }
    OOFEM_ERROR("MixedGradientPressureDirichlet :: giveBcValue - Has no prescribed value from bc.");
    return 0.0;
}


bool MixedGradientPressureDirichlet :: hasBc(ActiveDof *dof, TimeStep *tStep)
{
    return this->isDevDof(dof);
}


bool MixedGradientPressureDirichlet :: isDevDof(ActiveDof *dof)
{
    return devdman == dof->giveDofManager();
}


IRResultType MixedGradientPressureDirichlet :: initializeFrom(InputRecord *ir)
{
    const char *__proc = "initializeFrom";
    IRResultType result;

    MixedGradientPressureBC :: initializeFrom(ir);

    this->centerCoord.resize( domain->giveNumberOfSpatialDimensions() );
    this->centerCoord.zero();
    IR_GIVE_OPTIONAL_FIELD(ir, this->centerCoord, _IFT_MixedGradientPressure_centerCoords)

    return IRRT_OK;
}


void MixedGradientPressureDirichlet :: giveInputRecord(DynamicInputRecord &input)
{
    MixedGradientPressureBC :: giveInputRecord(input);
    input.setField( this->centerCoord, _IFT_MixedGradientPressure_centerCoords );
    input.setField( this->devGradient, _IFT_MixedGradientPressure_devGradient );
    input.setField( this->pressure, _IFT_MixedGradientPressure_pressure );
}


} // end namespace oofem

