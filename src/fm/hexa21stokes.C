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

#include "hexa21stokes.h"
#include "fmelement.h"
#include "node.h"
#include "domain.h"
#include "equationid.h"
#include "gaussintegrationrule.h"
#include "gausspoint.h"
#include "bcgeomtype.h"
#include "generalboundarycondition.h"
#include "load.h"
#include "boundaryload.h"
#include "mathfem.h"
#include "fluiddynamicmaterial.h"
#include "fei3dhexalin.h"
#include "fei3dhexatriquad.h"
#include "crosssection.h"
#include "classfactory.h"

namespace oofem {

REGISTER_Element( Hexa21Stokes );

// Set up interpolation coordinates
FEI3dHexaLin Hexa21Stokes :: interpolation_lin;
FEI3dHexaTriQuad Hexa21Stokes :: interpolation_quad;
// Set up ordering vectors (for assembling)
IntArray Hexa21Stokes :: momentum_ordering(81);
IntArray Hexa21Stokes :: conservation_ordering(8);
IntArray Hexa21Stokes :: surf_ordering [ 6 ] = { IntArray(), IntArray(), IntArray(), IntArray(), IntArray(), IntArray() };
bool Hexa21Stokes :: __initialized = Hexa21Stokes :: initOrdering();

Hexa21Stokes :: Hexa21Stokes(int n, Domain *aDomain) : FMElement(n, aDomain)
{
    this->numberOfDofMans = 27;
    this->numberOfGaussPoints = 27;
}

Hexa21Stokes :: ~Hexa21Stokes()
{}

IRResultType Hexa21Stokes :: initializeFrom(InputRecord *ir)
{
    this->FMElement :: initializeFrom(ir);
    return IRRT_OK;
}

void Hexa21Stokes :: computeGaussPoints()
{
    if ( !integrationRulesArray ) {
        numberOfIntegrationRules = 1;
        integrationRulesArray = new IntegrationRule * [ 2 ];
        integrationRulesArray [ 0 ] = new GaussIntegrationRule(1, this, 1, 3);
        this->giveCrossSection()->setupIntegrationPoints( *integrationRulesArray[0], numberOfGaussPoints, this );
    }
}

int Hexa21Stokes :: computeNumberOfDofs(EquationID ut)
{
    if ( ut == EID_MomentumBalance_ConservationEquation ) {
        return 89;
    } else if ( ut == EID_MomentumBalance ) {
        return 81;
    } else if ( ut == EID_ConservationEquation ) {
        return 8;
    } else {
        OOFEM_ERROR("Hexa21Stokes :: computeNumberOfDofs: Unknown equation id encountered");
    }

    return 0;
}

void Hexa21Stokes :: giveDofManDofIDMask(int inode, EquationID ut, IntArray &answer) const
{
    // Returns the mask for node number inode of this element. The mask tells what quantities
    // are held by each node. Since this element holds velocities (both in x and y direction),
    // in six nodes and pressure in three nodes the answer depends on which node is requested.

    if ( inode <= 8 ) {
        if ( ut == EID_MomentumBalance ) {
            answer.setValues(3, V_u, V_v, V_w);
        } else if ( ut == EID_ConservationEquation ) {
            answer.setValues(1, P_f);
        } else if ( ut == EID_MomentumBalance_ConservationEquation ) {
            answer.setValues(4, V_u, V_v, V_w, P_f);
        } else {
            OOFEM_ERROR("Hexa21Stokes :: giveDofManDofIDMask: Unknown equation id encountered");
        }
    } else {
        if ( ut == EID_MomentumBalance || ut == EID_MomentumBalance_ConservationEquation ) {
            answer.setValues(3, V_u, V_v, V_w);
        } else if ( ut == EID_ConservationEquation ) {
            answer.resize(0);
        } else {
            OOFEM_ERROR("Hexa21Stokes :: giveDofManDofIDMask: Unknown equation id encountered");
        }
    }
}

void Hexa21Stokes :: giveCharacteristicVector(FloatArray &answer, CharType mtrx, ValueModeType mode,
                                            TimeStep *tStep)
{
    // Compute characteristic vector for this element. I.e the load vector(s)
    if ( mtrx == ExternalForcesVector ) {
        this->computeExternalForcesVector(answer, tStep);
    } else if ( mtrx == InternalForcesVector ) {
        this->computeInternalForcesVector(answer, tStep);
    } else {
        OOFEM_ERROR("Hexa21Stokes :: giveCharacteristicVector: Unknown Type of characteristic mtrx.");
    }
}

void Hexa21Stokes :: giveCharacteristicMatrix(FloatMatrix &answer,
                                            CharType mtrx, TimeStep *tStep)
{
    // Compute characteristic matrix for this element. The only option is the stiffness matrix...
    if ( mtrx == StiffnessMatrix ) {
        this->computeStiffnessMatrix(answer, tStep);
    } else {
        OOFEM_ERROR("Hexa21Stokes :: giveCharacteristicMatrix: Unknown Type of characteristic mtrx.");
    }
}

void Hexa21Stokes :: computeInternalForcesVector(FloatArray &answer, TimeStep *tStep)
{
    IntegrationRule *iRule = integrationRulesArray [ 0 ];
    FluidDynamicMaterial *mat = static_cast<FluidDynamicMaterial * >( this->giveMaterial() );
    FloatArray a_pressure, a_velocity, devStress, epsp, Nh, dN_V(81);
    FloatMatrix dN, B(6, 81);
    double r_vol, pressure;
    this->computeVectorOf(EID_MomentumBalance, VM_Total, tStep, a_velocity);
    this->computeVectorOf(EID_ConservationEquation, VM_Total, tStep, a_pressure);
    FloatArray momentum, conservation;

    B.zero();
    for ( int i = 0; i < iRule->giveNumberOfIntegrationPoints(); i++ ) {
        GaussPoint *gp = iRule->getIntegrationPoint(i);
        const FloatArray &lcoords = * gp->giveCoordinates();

        double detJ = fabs( this->interpolation_quad.evaldNdx(dN, lcoords, FEIElementGeometryWrapper(this)) );
        this->interpolation_lin.evalN(Nh, lcoords, FEIElementGeometryWrapper(this));
        double dV = detJ * gp->giveWeight();

        for ( int j = 0, k = 0; j < dN.giveNumberOfRows(); j++, k+=3 ) {
            dN_V(k + 0) = B(0, k + 0) = B(5, k + 1) = B(4, k + 2) = dN(j, 0);
            dN_V(k + 1) = B(1, k + 1) = B(5, k + 0) = B(3, k + 2) = dN(j, 1);
            dN_V(k + 2) = B(2, k + 2) = B(4, k + 0) = B(3, k + 1) = dN(j, 2);
        }

        epsp.beProductOf(B, a_velocity);
        pressure = Nh.dotProduct(a_pressure);
        mat->computeDeviatoricStressVector(devStress, r_vol, gp, epsp, pressure, tStep);

        momentum.plusProduct(B, devStress, dV);
        momentum.add(-pressure*dV, dN_V);
        conservation.add(r_vol*dV, Nh);
    }

    answer.resize(89);
    answer.zero();
    answer.assemble(momentum, this->momentum_ordering);
    answer.assemble(conservation, this->conservation_ordering);
}

void Hexa21Stokes :: computeExternalForcesVector(FloatArray &answer, TimeStep *tStep)
{
    FloatArray vec;

    int nLoads = this->boundaryLoadArray.giveSize() / 2;
    answer.resize(0);

    for ( int i = 1; i <= nLoads; i++ ) {  // For each Neumann boundary condition
        int load_number = this->boundaryLoadArray.at(2 * i - 1);
        int load_id = this->boundaryLoadArray.at(2 * i);
        Load *load = this->domain->giveLoad(load_number);

        if ( load->giveBCGeoType() == SurfaceLoadBGT ) {
            this->computeBoundaryLoadVector(vec, static_cast<BoundaryLoad*>(load), load_id, ExternalForcesVector, VM_Total, tStep);
            answer.add(vec);
        }
    }

    nLoads = this->giveBodyLoadArray()->giveSize();
    for ( int i = 1; i <= nLoads; i++ ) {
        Load *load = domain->giveLoad( bodyLoadArray.at(i) );
        if ( load->giveBCGeoType() == BodyLoadBGT && load->giveBCValType() == ForceLoadBVT ) {
            this->computeLoadVector(vec, load, ExternalForcesVector, VM_Total, tStep);
            answer.add(vec);
        }
    }
}

void Hexa21Stokes :: computeLoadVector(FloatArray &answer, Load *load, CharType type, ValueModeType mode, TimeStep *tStep)
{
    if ( type != ExternalForcesVector ) {
        answer.resize(0);
        return;
    }

    IntegrationRule *iRule = this->integrationRulesArray [ 0 ];
    FloatArray N, gVector, temparray(81);

    load->computeComponentArrayAt(gVector, tStep, VM_Total);
    temparray.zero();
    if ( gVector.giveSize() ) {
        for ( int k = 0; k < iRule->giveNumberOfIntegrationPoints(); k++ ) {
            GaussPoint *gp = iRule->getIntegrationPoint(k);
            FloatArray *lcoords = gp->giveCoordinates();

            double rho = this->giveMaterial()->give('d', gp);
            double detJ = fabs( this->interpolation_quad.giveTransformationJacobian(* lcoords, FEIElementGeometryWrapper(this)) );
            double dA = detJ * gp->giveWeight();

            this->interpolation_quad.evalN(N, * lcoords, FEIElementGeometryWrapper(this));
            for ( int j = 0; j < N.giveSize(); j++ ) {
                temparray(3 * j + 0) += N(j) * rho * gVector(0) * dA;
                temparray(3 * j + 1) += N(j) * rho * gVector(1) * dA;
                temparray(3 * j + 2) += N(j) * rho * gVector(2) * dA;
            }
        }
    }

    answer.resize(89);
    answer.zero();
    answer.assemble( temparray, this->momentum_ordering );
}

void Hexa21Stokes :: computeBoundaryLoadVector(FloatArray &answer, BoundaryLoad *load, int iSurf, CharType type, ValueModeType mode, TimeStep *tStep)
{
    if ( type != ExternalForcesVector ) {
        answer.resize(0);
        return;
    }

    if ( load->giveType() == TransmissionBC ) { // Neumann boundary conditions (traction)
        BoundaryLoad *boundaryLoad = static_cast< BoundaryLoad * >( load );

        int numberOfSurfaceIPs = ( int ) ceil( ( boundaryLoad->giveApproxOrder() + 1. ) / 2. ) * 2; ///@todo Check this.

        GaussIntegrationRule iRule(1, this, 1, 1);
        GaussPoint *gp;
        FloatArray N, t, f(27);

        f.zero();
        iRule.SetUpPointsOnTriangle(numberOfSurfaceIPs, _Unknown);

        for ( int i = 0; i < iRule.giveNumberOfIntegrationPoints(); i++ ) {
            gp = iRule.getIntegrationPoint(i);
            FloatArray *lcoords = gp->giveCoordinates();

            this->interpolation_quad.surfaceEvalN(N, iSurf, * lcoords, FEIElementGeometryWrapper(this));
            double dA = gp->giveWeight() * this->interpolation_quad.surfaceGiveTransformationJacobian(iSurf, * lcoords, FEIElementGeometryWrapper(this));

            if ( boundaryLoad->giveFormulationType() == Load :: FT_Entity ) { // load in xi-eta system
                boundaryLoad->computeValueAt(t, tStep, * lcoords, VM_Total);
            } else { // Edge load in x-y system
                FloatArray gcoords;
                this->interpolation_quad.surfaceLocal2global(gcoords, iSurf, * lcoords, FEIElementGeometryWrapper(this));
                boundaryLoad->computeValueAt(t, tStep, gcoords, VM_Total);
            }

            // Reshape the vector
            for ( int j = 0; j < N.giveSize(); j++ ) {
                f(3 * j + 0) += N(j) * t(0) * dA;
                f(3 * j + 1) += N(j) * t(1) * dA;
                f(3 * j + 2) += N(j) * t(2) * dA;
            }
        }

        answer.resize(89);
        answer.zero();
        answer.assemble(f, this->surf_ordering [ iSurf - 1 ]);
    } else {
        OOFEM_ERROR("Hexa21Stokes :: Strange boundary condition type");
    }
}

void Hexa21Stokes :: computeStiffnessMatrix(FloatMatrix &answer, TimeStep *tStep)
{
    FluidDynamicMaterial *mat = static_cast< FluidDynamicMaterial * >( this->giveMaterial() );
    IntegrationRule *iRule = this->integrationRulesArray [ 0 ];
    FloatMatrix B(6, 81), EdB, K, G, Dp, DvT, C, Ed, dN;
    FloatArray dN_V(81), Nlin, Ep, Cd, tmpA, tmpB;
    double Cp;

    B.zero();

    for ( int i = 0; i < iRule->giveNumberOfIntegrationPoints(); i++ ) {
        // Compute Gauss point and determinant at current element
        GaussPoint *gp = iRule->getIntegrationPoint(i);
        const FloatArray &lcoords = *gp->giveCoordinates();

        double detJ = fabs( this->interpolation_quad.evaldNdx(dN, lcoords, FEIElementGeometryWrapper(this)) );
        double dV = detJ * gp->giveWeight();
        this->interpolation_lin.evalN(Nlin, lcoords, FEIElementGeometryWrapper(this));

        for ( int j = 0, k = 0; j < dN.giveNumberOfRows(); j++, k+=3 ) {
            dN_V(k + 0) = B(0, k + 0) = B(5, k + 1) = B(4, k + 2) = dN(j, 0);
            dN_V(k + 1) = B(1, k + 1) = B(5, k + 0) = B(3, k + 2) = dN(j, 1);
            dN_V(k + 2) = B(2, k + 2) = B(4, k + 0) = B(3, k + 1) = dN(j, 2);
        }

        // Computing the internal forces should have been done first.
        mat->giveDeviatoricStiffnessMatrix(Ed, TangentStiffness, gp, tStep); // dsigma_dev/deps_dev
        mat->giveDeviatoricPressureStiffness(Ep, TangentStiffness, gp, tStep); // dsigma_dev/dp
        mat->giveVolumetricDeviatoricStiffness(Cd, TangentStiffness, gp, tStep); // deps_vol/deps_dev
        mat->giveVolumetricPressureStiffness(Cp, TangentStiffness, gp, tStep); // deps_vol/dp

        EdB.beProductOf(Ed,B);
        K.plusProductSymmUpper(B, EdB, dV);
        G.plusDyadUnsym(dN_V, Nlin, -dV);
        C.plusDyadSymmUpper(Nlin, Cp*dV);

        tmpA.beTProductOf(B, Ep);
        Dp.plusDyadUnsym(tmpA, Nlin, dV);

        tmpB.beTProductOf(B, Cd);
        DvT.plusDyadUnsym(Nlin, tmpB, dV);
    }
    K.symmetrized();
    C.symmetrized();
    FloatMatrix GTDvT, GDp;
    GTDvT.beTranspositionOf(G);
    GTDvT.add(DvT);
    GDp = G;
    GDp.add(Dp);

    answer.resize(89, 89);
    answer.zero();
    answer.assemble(K, this->momentum_ordering);
    answer.assemble(GDp, this->momentum_ordering, this->conservation_ordering);
    answer.assemble(GTDvT, this->conservation_ordering, this->momentum_ordering);
    answer.assemble(C, this->conservation_ordering);
}

FEInterpolation *Hexa21Stokes :: giveInterpolation() const
{
    return &interpolation_quad;
}

FEInterpolation *Hexa21Stokes :: giveInterpolation(DofIDItem id) const
{
    if (id == P_f) return &interpolation_lin;
    else return &interpolation_quad;
}

void Hexa21Stokes :: updateYourself(TimeStep *tStep)
{
    Element :: updateYourself(tStep);
}

// Some extension Interfaces to follow:

Interface *Hexa21Stokes :: giveInterface(InterfaceType it)
{
    switch ( it ) {
        case NodalAveragingRecoveryModelInterfaceType:
            return static_cast< NodalAveragingRecoveryModelInterface * >(this);
        case SpatialLocalizerInterfaceType:
            return static_cast< SpatialLocalizerInterface * >(this);
        case EIPrimaryUnknownMapperInterfaceType:
            return static_cast< EIPrimaryUnknownMapperInterface * >(this);
        default:
            return FMElement :: giveInterface(it);
    }
}

int Hexa21Stokes :: SpatialLocalizerI_containsPoint(const FloatArray &coords)
{
    FloatArray lcoords;
    return this->computeLocalCoordinates(lcoords, coords);
}

void Hexa21Stokes :: EIPrimaryUnknownMI_computePrimaryUnknownVectorAtLocal(ValueModeType mode,
        TimeStep *tStep, const FloatArray &lcoords, FloatArray &answer)
{
    FloatArray n, n_lin;
    this->interpolation_quad.evalN(n, lcoords, FEIElementGeometryWrapper(this));
    this->interpolation_lin.evalN(n_lin, lcoords, FEIElementGeometryWrapper(this));
    answer.resize(4);
    answer.zero();
    for ( int i = 1; i <= n.giveSize(); i++ ) {
        answer(0) += n.at(i)*this->giveNode(i)->giveDofWithID(V_u)->giveUnknown(mode, tStep);
        answer(1) += n.at(i)*this->giveNode(i)->giveDofWithID(V_v)->giveUnknown(mode, tStep);
        answer(2) += n.at(i)*this->giveNode(i)->giveDofWithID(V_w)->giveUnknown(mode, tStep);
    }
    for ( int i = 1; i <= n_lin.giveSize(); i++ ) {
        answer(3) += n_lin.at(i)*this->giveNode(i)->giveDofWithID(P_f)->giveUnknown(mode, tStep);
    }
}

int Hexa21Stokes :: EIPrimaryUnknownMI_computePrimaryUnknownVectorAt(ValueModeType mode, TimeStep *tStep, const FloatArray &gcoords, FloatArray &answer)
{
    bool ok;
    FloatArray lcoords, n, n_lin;
    ok = this->computeLocalCoordinates(lcoords, gcoords);
    if ( !ok ) {
        answer.resize(0);
        return false;
    }
    this->EIPrimaryUnknownMI_computePrimaryUnknownVectorAtLocal(mode, tStep, lcoords, answer);
    return true;
}

void Hexa21Stokes :: EIPrimaryUnknownMI_givePrimaryUnknownVectorDofID(IntArray &answer)
{
    answer.setValues(4, V_u, V_v, V_w, P_f);
}

double Hexa21Stokes :: SpatialLocalizerI_giveDistanceFromParametricCenter(const FloatArray &coords)
{
    FloatArray center;
    FloatArray lcoords;
    lcoords.setValues(3, 0.25, 0.25, 0.25);
    this->computeGlobalCoordinates(center, lcoords);
    return center.distance(coords);
}

void Hexa21Stokes :: NodalAveragingRecoveryMI_computeSideValue(FloatArray &answer, int side, InternalStateType type, TimeStep *tStep)
{
    answer.resize(0);
}

void Hexa21Stokes :: NodalAveragingRecoveryMI_computeNodalValue(FloatArray &answer, int node, InternalStateType type, TimeStep *tStep)
{
    answer.resize(1);
    if ( type == IST_Pressure ) {
        answer.resize(1);
        if ( node <= 8 ) { // Corner nodes
            answer.at(1) = this->giveNode(node)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep);
        } else if ( node <= 20 ) { // Edge nodes
            // Edges are numbered consistently with edge nodes, so node number - 8 = edge number
            IntArray eNodes;
            this->interpolation_quad.computeLocalEdgeMapping(eNodes, node - 8);
            answer.at(1) = 0.5 * (
                this->giveNode(eNodes.at(1))->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(eNodes.at(2))->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep));
        } else if ( node <= 26 ) { // Face nodes
            // Faces are numbered consistently with edge nodes, so node number - 12 = face number
            IntArray fNodes;
            this->interpolation_quad.computeLocalSurfaceMapping(fNodes, node - 20);
            answer.at(1) = 0.25 * (
                this->giveNode(fNodes.at(1))->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(fNodes.at(2))->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) + 
                this->giveNode(fNodes.at(3))->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) + 
                this->giveNode(fNodes.at(4))->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep));
        } else { // Middle node
            answer.at(1) = 0.125 * (
                this->giveNode(1)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(2)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(3)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(4)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(5)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(6)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(7)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep) +
                this->giveNode(8)->giveDofWithID(P_f)->giveUnknown(VM_Total, tStep));
        }
    } else {
        answer.resize(0);
    }
}
} // end namespace oofem
