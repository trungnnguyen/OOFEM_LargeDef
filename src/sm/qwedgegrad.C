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

#include "qwedgegrad.h"
#include "node.h"
#include "material.h"
#include "gausspoint.h"
#include "gaussintegrationrule.h"
#include "floatmatrix.h"
#include "floatarray.h"
#include "intarray.h"
#include "domain.h"
#include "cltypes.h"
#include "structuralms.h"
#include "mathfem.h"
#include "structuralcrosssection.h"
#include "classfactory.h"

#include <cstdio>

namespace oofem {
REGISTER_Element(QWedgeGrad);

FEI3dWedgeLin QWedgeGrad :: interpolation;

QWedgeGrad :: QWedgeGrad(int n, Domain *aDomain) :  QWedge(n, aDomain), GradDpElement()
    // Constructor.
{
    nPrimNodes = 15;
    nPrimVars = 3;
    nSecNodes = 6;
    nSecVars = 1;
    totalSize = nPrimVars * nPrimNodes + nSecVars * nSecNodes;
    locSize   = nPrimVars * nPrimNodes;
    nlSize    = nSecVars * nSecNodes;
}


IRResultType
QWedgeGrad :: initializeFrom(InputRecord *ir)
{
    IRResultType result = this->NLStructuralElement :: initializeFrom(ir);
	if(result != IRRT_OK) {
		return result;
	}

    if ( ( numberOfGaussPoints != 2 ) && ( numberOfGaussPoints != 9 ) ) {
        numberOfGaussPoints = 9;
    }

    return IRRT_OK;
}


void
QWedgeGrad :: giveDofManDofIDMask(int inode, EquationID ut, IntArray &answer) const
// returns DofId mask array for inode element node.
// DofId mask array determines the dof ordering requsted from node.
// DofId mask array contains the DofID constants (defined in cltypes.h)
// describing physical meaning of particular DOFs.
{
    if ( inode <= nSecNodes ) {
        answer.resize(4);
        answer.at(1) = D_u;
        answer.at(2) = D_v;
        answer.at(3) = D_w;
        answer.at(4) = G_0;
    } else {
        answer.resize(3);
        answer.at(1) = D_u;
        answer.at(2) = D_v;
        answer.at(3) = D_w;
    }
}

void
QWedgeGrad :: computeGaussPoints()
// Sets up the array containing the four Gauss points of the receiver.
{
    numberOfIntegrationRules = 1;
    integrationRulesArray = new IntegrationRule * [ numberOfIntegrationRules ];
    integrationRulesArray [ 0 ] = new GaussIntegrationRule(1, this, 1, 7);
    this->giveCrossSection()->setupIntegrationPoints(* integrationRulesArray [ 0 ], numberOfGaussPoints, this);
}



void
QWedgeGrad :: computeNkappaMatrixAt(GaussPoint *aGaussPoint, FloatMatrix &answer)
// Returns the displacement interpolation matrix {N} of the receiver, eva-
// luated at aGaussPoint.
{
    FloatArray n(9);
    this->interpolation.evalN( n, * aGaussPoint->giveCoordinates(), FEIElementGeometryWrapper(this) );
    answer.resize(1, 9);
    answer.zero();

    for ( int i = 1; i <= 9; i++ ) {
        answer.at(1, i) = n.at(i);
    }
}

void
QWedgeGrad :: computeBkappaMatrixAt(GaussPoint *aGaussPoint, FloatMatrix &answer)
{
    FloatMatrix dnx;
    IntArray a(9);

    answer.resize(3, 9);
    answer.zero();

    this->interpolation.evaldNdx( dnx, * aGaussPoint->giveCoordinates(), FEIElementGeometryWrapper(this) );
    for ( int i = 1; i <= 9; i++ ) {
        answer.at(1, i) = dnx.at(i, 1);
        answer.at(2, i) = dnx.at(i, 2);
        answer.at(3, i) = dnx.at(i, 3);
    }
}


void
QWedgeGrad :: computeNLBMatrixAt(FloatMatrix &answer, GaussPoint *aGaussPoint, int i)
// Returns the [45x45] nonlinear part of strain-displacement matrix {B} of the receiver,
// evaluated at aGaussPoint

{
    FloatMatrix dnx;

    // compute the derivatives of shape functions
    this->interpolation.evaldNdx( dnx, * aGaussPoint->giveCoordinates(), FEIElementGeometryWrapper(this) );

    answer.resize(45, 45);
    answer.zero();

    // put the products of derivatives of shape functions into the "nonlinear B matrix",
    // depending on parameter i, which is the number of the strain component
    if ( i <= 3 ) {
        for ( int k = 0; k < 15; k++ ) {
            for ( int l = 0; l < 3; l++ ) {
                for ( int j = 1; j <= 45; j += 3 ) {
                    answer.at(k * 3 + l + 1, l + j) = dnx.at(k + 1, i) * dnx.at( ( j - 1 ) / 3 + 1, i );
                }
            }
        }
    } else if ( i == 4 ) {
        for ( int k = 0; k < 15; k++ ) {
            for ( int l = 0; l < 3; l++ ) {
                for ( int j = 1; j <= 45; j += 3 ) {
                    answer.at(k * 3 + l + 1, l + j) = dnx.at(k + 1, 2) * dnx.at( ( j - 1 ) / 3 + 1, 3 ) + dnx.at(k + 1, 3) * dnx.at( ( j - 1 ) / 3 + 1, 2 );
                }
            }
        }
    } else if ( i == 5 ) {
        for ( int k = 0; k < 15; k++ ) {
            for ( int l = 0; l < 3; l++ ) {
                for ( int j = 1; j <= 45; j += 3 ) {
                    answer.at(k * 3 + l + 1, l + j) = dnx.at(k + 1, 1) * dnx.at( ( j - 1 ) / 3 + 1, 3 ) + dnx.at(k + 1, 3) * dnx.at( ( j - 1 ) / 3 + 1, 1 );
                }
            }
        }
    } else if ( i == 6 ) {
        for ( int k = 0; k < 15; k++ ) {
            for ( int l = 0; l < 3; l++ ) {
                for ( int j = 1; j <= 45; j += 3 ) {
                    answer.at(k * 3 + l + 1, l + j) = dnx.at(k + 1, 1) * dnx.at( ( j - 1 ) / 3 + 1, 2 ) + dnx.at(k + 1, 2) * dnx.at( ( j - 1 ) / 3 + 1, 1 );
                }
            }
        }
    }
}
}
