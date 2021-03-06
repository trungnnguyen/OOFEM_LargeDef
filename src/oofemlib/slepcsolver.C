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

#include "slepcsolver.h"

#define TIME_REPORT
#include "petscsparsemtrx.h"
#include "engngm.h"
#include "floatarray.h"
#include "verbose.h"

#ifdef TIME_REPORT
 #include "timer.h"
#endif

namespace oofem {
SLEPcSolver :: SLEPcSolver(Domain *d, EngngModel *m) : SparseGeneralEigenValueSystemNM(d, m)
{
    A = B = NULL;
    epsInit = false;
}


SLEPcSolver :: ~SLEPcSolver()
{
    if ( epsInit ) {
        EPSDestroy(eps);
    }
}

NM_Status
SLEPcSolver :: solve(SparseMtrx *a, SparseMtrx *b, FloatArray *_eigv, FloatMatrix *_r, double rtol, int nroot)
{
    FILE *outStream;
    PetscErrorCode ierr;
    int size;
    ST st;

    outStream = domain->giveEngngModel()->giveOutputStream();

    // first check whether Lhs is defined
    if ( ( !a ) || ( !b ) ) {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt : matrices are not defined\n");
    }

    if ( a->giveNumberOfRows() != a->giveNumberOfColumns() ||
        b->giveNumberOfRows() != b->giveNumberOfRows() ||
        a->giveNumberOfColumns() != b->giveNumberOfColumns() ) {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt : matrices size mismatch\n");
    }

    if ( a->giveType() != SMT_PetscMtrx || b->giveType() != SMT_PetscMtrx ) {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt: PetscSparseMtrx Expected");
    }

    A = ( PetscSparseMtrx * ) a;
    B = ( PetscSparseMtrx * ) b;
    size = engngModel->givePetscContext( A->giveDomainIndex() )->giveNumberOfNaturalEqs(); // A->giveLeqs();

    // check array for storing eigenvalues
    if ( _eigv == NULL ) {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt: unknown eigenvalue array");
    }

    if ( _eigv->giveSize() != nroot ) {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt: eigv size mismatch");
    }

    // check matrix for storing resulted eigen vectors at the end
    if ( _r == NULL ) {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt: unknown eigen vectors mtrx");
    }

    if ( ( _r->giveNumberOfRows() != size ) || ( _r->giveNumberOfColumns() != nroot ) ) {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt: _r size mismatch");
    }


    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    *             Create the eigensolver and set various options
    *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    int nconv, nite;
    EPSConvergedReason reason;

 #ifdef TIME_REPORT
    Timer timer;
    timer.startTimer();
 #endif

    if ( !epsInit ) {
        /*
         * Create eigensolver context
         */
 #ifdef __PARALLEL_MODE
        ierr = EPSCreate(PETSC_COMM_WORLD, & eps);
        CHKERRQ(ierr);
 #else
        ierr = EPSCreate(PETSC_COMM_SELF, & eps);
        CHKERRQ(ierr);
 #endif
        epsInit = true;
    }

    /*
     * Set operators. In this case, it is a generalized eigenvalue problem
     */

    ierr = EPSSetOperators( eps, * A->giveMtrx(), * B->giveMtrx() );
    CHKERRQ(ierr);
    ierr = EPSSetProblemType(eps, EPS_GHEP);
    CHKERRQ(ierr);
    ierr = EPSGetST(eps, & st);
    CHKERRQ(ierr);
    ierr = STSetType(st, STSINVERT);
    CHKERRQ(ierr);
    ierr = STSetMatStructure(st, SAME_NONZERO_PATTERN);
    CHKERRQ(ierr);
    ierr = EPSSetTolerances(eps, ( PetscReal ) rtol, PETSC_DECIDE);
    CHKERRQ(ierr);
    ierr = EPSSetDimensions(eps, ( PetscInt ) nroot, PETSC_DECIDE, PETSC_DECIDE);
    CHKERRQ(ierr);
    ierr = EPSSetWhichEigenpairs(eps, EPS_SMALLEST_MAGNITUDE);
    CHKERRQ(ierr);

    /*
     * Set solver parameters at runtime
     */

    ierr = EPSSetFromOptions(eps);
    CHKERRQ(ierr);

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    *                   Solve the eigensystem
    *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    ierr = EPSSolve(eps);
    CHKERRQ(ierr);

    ierr = EPSGetConvergedReason(eps, & reason);
    CHKERRQ(ierr);
    ierr = EPSGetIterationNumber(eps, & nite);
    CHKERRQ(ierr);
    OOFEM_LOG_INFO("SLEPcSolver::solve EPSConvergedReason: %d, number of iterations: %d\n", reason, nite);

    ierr = EPSGetConverged(eps, & nconv);
    CHKERRQ(ierr);

    if ( nconv > 0 ) {
        fprintf(outStream, "SLEPcSolver :: solveYourselfAt: Convergence reached for RTOL=%20.15f", rtol);
        PetscScalar kr, *array;
        //PetscInt vSize;
        Vec Vr;

        ierr = MatGetVecs(* B->giveMtrx(), PETSC_NULL, & Vr);
        CHKERRQ(ierr);

 #ifdef __PARALLEL_MODE
        Vec Vr2;
        ierr = VecCreateSeq(PETSC_COMM_SELF, size, & Vr2);
        CHKERRQ(ierr);
 #endif

        for ( int i = 0; i < nconv && i < nroot; i++ ) {
            ierr = EPSGetEigenpair(eps, nconv - i - 1, & kr, PETSC_NULL, Vr, PETSC_NULL);
            CHKERRQ(ierr);

            //Store the eigenvalue
            _eigv->at(i + 1) = kr;

            //Store the eigenvector
 #ifdef __PARALLEL_MODE
            engngModel->givePetscContext( A->giveDomainIndex() )->scatterG2N(Vr, Vr2, INSERT_VALUES);
            ierr = VecGetArray(Vr2, & array);
            CHKERRQ(ierr);
            for ( int j = 0; j < size; j++ ) {
                _r->at(j + 1, i + 1) = array [ j ];
            }

            ierr = VecRestoreArray(Vr2, & array);
            CHKERRQ(ierr);
 #else
            ierr = VecGetArray(Vr, & array);
            CHKERRQ(ierr);
            for ( int j = 0; j < size; j++ ) {
                _r->at(j + 1, i + 1) = array [ j ];
            }

            ierr = VecRestoreArray(Vr, & array);
            CHKERRQ(ierr);

 #endif
        }

        ierr = VecDestroy(Vr);
        CHKERRQ(ierr);
 #ifdef __PARALLEL_MODE
        ierr = VecDestroy(Vr2);
        CHKERRQ(ierr);
 #endif
    } else {
        OOFEM_ERROR("SLEPcSolver :: solveYourselfAt: No converged eigenpairs\n");
    }

 #ifdef TIME_REPORT
    timer.stopTimer();
    OOFEM_LOG_INFO( "SLEPcSolver info: user time consumed by solution: %.2fs\n", timer.getUtime() );
 #endif

    return NM_Success;
}
} // end namespace oofem

