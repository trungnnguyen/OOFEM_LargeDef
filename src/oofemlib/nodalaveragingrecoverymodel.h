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

#ifndef nodalaveragingrecoverymodel_h
#define nodalaveragingrecoverymodel_h

#include "nodalrecoverymodel.h"
#include "interface.h"

namespace oofem {
class GaussPoint;
class ProcessCommunicator;

/**
 * The nodal recovery model based on nodal averaging. The recovery is based
 * on nodal averaging or projection process in which element contributions are averaged
 * at node with the same weight.
 */
class NodalAveragingRecoveryModel : public NodalRecoveryModel
{
protected:
    /**
     * Helper structure to pass required arguments to packing/unpacking functions
     * needed in parallel mode
     */
    struct parallelStruct {
        FloatArray *lhs;
        IntArray *regionDofMansConnectivity;
        IntArray *regionNodalNumbers;
        int regionValSize;
        parallelStruct(FloatArray *a, IntArray *b, IntArray *c, int d):
            lhs(a), regionDofMansConnectivity(b), regionNodalNumbers(c), regionValSize(d) { }
    };

public:
    /// Constructor.
    NodalAveragingRecoveryModel(Domain *d);
    /// Destructor.
    ~NodalAveragingRecoveryModel();

    int recoverValues(InternalStateType type, TimeStep *tStep);

private:
#ifdef __PARALLEL_MODE
    void initCommMaps();
    void exchangeDofManValues(int ireg, FloatArray &lhs, IntArray &, IntArray &, int);
    int  packSharedDofManData(parallelStruct *s, ProcessCommunicator &processComm);
    int  unpackSharedDofManData(parallelStruct *s, ProcessCommunicator &processComm);
#endif
};

/**
 * The element interface required by NodalAvergagingRecoveryModel.
 */
class NodalAveragingRecoveryModelInterface : public Interface
{
public:
    /// Constructor
    NodalAveragingRecoveryModelInterface() { }

    /// @name The element interface required by NodalAveragingRecoveryModel
    //@{
    /**
     * Computes the element value in given node.
     * @param answer Contains the result.
     * @param node Element node number.
     * @param type Determines the type of internal variable to be recovered.
     * @param tStep Time step.
     */
    virtual void NodalAveragingRecoveryMI_computeNodalValue(FloatArray &answer, int node,
                                                            InternalStateType type, TimeStep *tStep) = 0;
    /**
     * Computes the element value in given side.
     * @param answer Contains the result.
     * @param side Element side number.
     * @param type Determines the type of internal variable to be recovered.
     * @param tStep Time step.
     */
    virtual void NodalAveragingRecoveryMI_computeSideValue(FloatArray &answer, int side,
                                                           InternalStateType type, TimeStep *tStep) = 0;
    //@}
};
} // end namespace oofem
#endif // nodalaveragingrecoverymodel_h
