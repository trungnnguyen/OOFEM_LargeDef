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

#ifndef structuralinterfacematerialstatus_h
#define structuralinterfacematerialstatus_h

#include "matstatus.h"
#include "floatarray.h"

namespace oofem {
class GaussPoint;
class Dictionary;
class Domain;

/**
 * This class implements a structural interface material status information. It is attribute of
 * gaussPoint. This is only an abstract class, for every instance of this material class
 * there should be a specialized derived class, which handles are history variables.
 *
 * This is a base class for all material statuses corresponding to materials derived from
 * StructuralInterfaceMaterial class.
 * It defines the traction, jump (discontinuity), deformation gradient and their temporary counterparts.
 * Functions for accessing these components are defined.
 *
 * Tasks:
 * This is abstract class - only basic functionality is supported like:
 * - maintaining and providing access to stress and strain vectors
 *   (including their increments)
 * - storing and restoring status on tape
 * - printingYourself()
 * - updating Yourself after a new equilibrium state has been reached.
 */
class StructuralInterfaceMaterialStatus : public MaterialStatus
{
protected:
    /// Equilibrated jump (discontinuity)
    FloatArray jump;
    /// Equilibrated (engineering) traction vector
    FloatArray traction;
    /// Temporary (engineering) traction vector
    FloatArray tempTraction;
    /// Temporary jump (discontinuity)
    FloatArray tempJump;

    /// Equilibrated first Piola-Kirchhoff traction vector T
    FloatArray firstPKTraction;
    /// Temporary first Piola-Kirchhoff traction vector (to find balanced state)
    FloatArray tempFirstPKTraction;
    /// Equilibrated deformation gradient in reduced form
    FloatMatrix F;
    /// Temporary deformation gradient in reduced form (to find balanced state)
    FloatMatrix tempF;

public:
    /// Constructor. Creates new StructuralInterfaceMaterialStatus with number n, belonging to domain d and IntegrationPoint g.
    StructuralInterfaceMaterialStatus(int n, Domain *d, GaussPoint *g);
    /// Destructor
    virtual ~StructuralInterfaceMaterialStatus();

    virtual void printOutputAt(FILE *file, TimeStep *tStep);

    virtual void initTempStatus();
    virtual void updateYourself(TimeStep *tStep);

    virtual contextIOResultType saveContext(DataStream *stream, ContextMode mode, void *obj = NULL);
    virtual contextIOResultType restoreContext(DataStream *stream, ContextMode mode, void *obj = NULL);

    /// Returns the const pointer to receiver's jump.
    const FloatArray &giveJump() { return jump; }
    /// Returns the const pointer to receiver's traction vector.
    const FloatArray &giveTraction() { return traction; }
    /// Returns the const pointer to receiver's first Piola-Kirchhoff traction vector.
    const FloatArray &giveFirstPKTraction() { return firstPKTraction; }
    /// Returns the const pointer to receiver's deformation gradient vector.
    const FloatMatrix &giveF() { return F; }
    /// Returns the const pointer to receiver's temporary jump.
    const FloatArray &giveTempJump() { return tempJump; }
    /// Returns the const pointer to receiver's temporary traction vector.
    const FloatArray &giveTempTraction() { return tempTraction; }
    /// Returns the const pointer to receiver's temporary first Piola-Kirchhoff traction vector.
    const FloatArray &giveTempFirstPKTraction() { return tempFirstPKTraction; }
    /// Returns the const pointer to receiver's temporary deformation gradient vector.
    const FloatMatrix &giveTempF() { return tempF; }
    /// Assigns jump to given vector v.
    void letJumpBe(const FloatArray &v) { jump = v; }
    /// Assigns traction to given vector v.
    void letTractionBe(const FloatArray &v) { traction = v; }
    /// Assigns firstPKTraction to given vector v.
    void letFirstPKTractionBe(const FloatArray &v) { firstPKTraction = v; }
    /// Assigns FVector to given vector v.
    void letFBe(const FloatMatrix &v) { F = v; }
    /// Assigns tempTraction to given vector v.
    void letTempTractionBe(const FloatArray &v) { tempTraction = v; }
    /// Assigns tempJump to given vector v
    void letTempJumpBe(const FloatArray &v) { tempJump = v; }
    /// Assigns tempFirstPKTraction to given vector v
    void letTempFirstPKTractionBe(const FloatArray &v) { tempFirstPKTraction = v; }
    /// Assigns tempFVector to given vector v
    void letTempFBe(const FloatMatrix &v) { tempF = v; }

    virtual const char *giveClassName() const { return "StructuralInterfaceMaterialStatus"; }
    //virtual classType giveClassID() const { return StructuralInterfaceMaterialStatus; }
};
} // end namespace oofem
#endif // structuralinterfacematerialstatus_h
