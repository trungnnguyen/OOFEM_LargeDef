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

#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>

#include "matlabexportmodule.h"
#include "engngm.h"
#include "node.h"
#include "mathfem.h"
#include "gausspoint.h"
#include "weakperiodicbc.h"
#include "timestep.h"
#include "classfactory.h"

#ifdef __FM_MODULE
#include "tr21stokes.h"
#include "tet21stokes.h"
#include "stokesflow.h"
#endif

namespace oofem {

REGISTER_ExportModule( MatlabExportModule )

				MatlabExportModule :: MatlabExportModule(int n, EngngModel *e) : ExportModule(n, e), internalVarsToExport(), primaryVarsToExport()
{}


MatlabExportModule :: ~MatlabExportModule()
{}


IRResultType
MatlabExportModule :: initializeFrom(InputRecord *ir)
{
	exportMesh = ir->hasField(_IFT_MatlabExportModule_mesh);
	exportData = ir->hasField(_IFT_MatlabExportModule_data);
	exportArea = ir->hasField(_IFT_MatlabExportModule_area);
	exportSpecials = ir->hasField(_IFT_MatlabExportModule_specials);

	return IRRT_OK;
}


void
MatlabExportModule :: computeArea()
{
	Domain *domain = emodel->giveDomain(1);

	smax.clear();
	smin.clear();

	for (int i=1; i<=domain->giveNumberOfSpatialDimensions(); i++) {
		smax.push_back(domain->giveDofManager(1)->giveCoordinate(i));
		smin.push_back(domain->giveDofManager(1)->giveCoordinate(i));
	}

	for ( int i = 0; i < domain->giveNumberOfDofManagers(); i++ ) {
		for (int j=0; j < domain->giveNumberOfSpatialDimensions(); j++) {
			smax.at(j)=max(smax.at(j), domain->giveDofManager(i+1)->giveCoordinate(j+1));
			smin.at(j)=min(smin.at(j), domain->giveDofManager(i+1)->giveCoordinate(j+1));
		}
//		std :: copy(smax.begin(), smax.end(), std :: ostream_iterator<double>(std :: cout, " "));
//		std :: cout << std :: endl;
	}


	Area=0;
	Volume=0;

	if (domain->giveNumberOfSpatialDimensions()==2) {
		for ( int i = 1; i <= domain->giveNumberOfElements(); i++ ) {
			Area = Area + domain->giveElement(i)->computeArea();
		}
	} else {
		for ( int i = 1; i <= domain->giveNumberOfElements(); i++ ) {
			Volume = Volume + domain->giveElement(i)->computeVolume();
		}
	}

}


void
MatlabExportModule :: doOutput(TimeStep *tStep, bool forcedOutput)
{
	FILE *FID;
	FID = giveOutputStream(tStep);
	Domain *domain  = emodel->giveDomain(1);
	ndim=domain->giveNumberOfSpatialDimensions();

	fprintf( FID, "function [mesh area data specials]=%s\n\n", functionname.c_str() );

	if ( exportMesh ) {
		doOutputMesh(tStep, FID);
	} else {
		fprintf(FID, "\tmesh=[];\n");
	}

	if ( exportData ) {
		doOutputData(tStep, FID);
	} else {
		fprintf(FID, "\tdata=[];\n");
	}

	if ( exportArea ) {
		computeArea();
		fprintf(FID, "\tarea.xmax=%f;\n", smax.at(0));
		fprintf(FID, "\tarea.xmin=%f;\n", smin.at(0));
		fprintf(FID, "\tarea.ymax=%f;\n", smax.at(1));
		fprintf(FID, "\tarea.ymin=%f;\n", smin.at(1));
		if (ndim==2) {
			fprintf(FID, "\tarea.area=%f;\n", Area);
			fprintf(FID, "\tvolume=[];\n");
		} else {
			fprintf(FID, "\tarea.zmax=%f;\n", smax.at(2));
			fprintf(FID, "\tarea.zmin=%f;\n", smin.at(2));
			fprintf(FID, "\tarea.area=[];\n");
			fprintf(FID, "\tarea.volume=%f;\n", Volume);
		}
	} else {
		fprintf(FID, "\tarea.area=[];\n");
		fprintf(FID, "\tarea.volume=[];\n");
	}

	if ( exportSpecials ) {
		if ( !exportArea ) {
			computeArea();
		}

		doOutputSpecials(tStep, FID);
	} else {
		fprintf(FID, "\tspecials=[];\n");
	}

	fprintf(FID, "\nend\n");
	fclose(FID);
}


void
MatlabExportModule :: doOutputMesh(TimeStep *tStep, FILE *FID)
{
	Domain *domain  = emodel->giveDomain(1);

	fprintf(FID, "\tmesh.p=[");
	for ( int i = 1; i <= domain->giveNumberOfDofManagers(); i++ ) {
		double x = domain->giveDofManager(i)->giveCoordinate(1), y = domain->giveDofManager(i)->giveCoordinate(2);
		fprintf(FID, "%f,%f;", x, y);
	}

	fprintf(FID, "]';\n");

	int numberOfDofMans=domain->giveElement(1)->giveNumberOfDofManagers();

	fprintf(FID, "\tmesh.t=[");
	for ( int i = 1; i <= domain->giveNumberOfElements(); i++ ) {
		if (domain->giveElement(i)->giveNumberOfDofManagers()==numberOfDofMans) {
			for ( int j = 1; j <= domain->giveElement(i)->giveNumberOfDofManagers(); j++ ) {
				fprintf( FID, "%d,", domain->giveElement(i)->giveDofManagerNumber(j) );
			}
		}
		fprintf(FID, ";");
	}

	fprintf(FID, "]';\n");
}


void
MatlabExportModule :: doOutputData(TimeStep *tStep, FILE *FID)
{
	Domain *domain  = emodel->giveDomain(1);
	std :: vector< int >DofIDList;
	std :: vector< int > :: iterator it;
	std :: vector< std :: vector< double > * >valuesList;
	std :: vector< double > *values;

	for ( int i = 1; i <= domain->giveNumberOfDofManagers(); i++ ) {
		for ( int j = 1; j <= domain->giveDofManager(i)->giveNumberOfDofs(); j++ ) {
			Dof *thisDof;
			thisDof = domain->giveDofManager(i)->giveDof(j);
			it = std :: find( DofIDList.begin(), DofIDList.end(), thisDof->giveDofID() );

			if ( it == DofIDList.end() ) {
				DofIDList.push_back( thisDof->giveDofID() );
				values = new( std :: vector< double > );
				valuesList.push_back(values);
			} else {
				std::size_t pos = it - DofIDList.begin();
				values = valuesList.at(pos);
			}

			double value = thisDof->giveUnknown(VM_Total, tStep);
			values->push_back(value);
		}
	}

	fprintf(FID, "\tdata.DofIDs=[");
	for ( size_t i = 0; i < DofIDList.size(); i++ ) {
		fprintf( FID, "%d, ", DofIDList.at(i) );
	}

	fprintf(FID, "];\n");

	for ( size_t i = 0; i < valuesList.size(); i++ ) {
		fprintf(FID, "\tdata.a{%lu}=[", static_cast<long unsigned int>(i) + 1);
		for ( size_t j = 0; j < valuesList.at(i)->size(); j++ ) {
			fprintf( FID, "%f,", valuesList.at(i)->at(j) );
		}

		fprintf(FID, "];\n");
	}

}


void
MatlabExportModule :: doOutputSpecials(TimeStep *tStep,    FILE *FID)
{
	FloatMatrix v_hat, GradPTemp, v_hatTemp;

	Domain *domain  = emodel->giveDomain(1);

	v_hat.resize(ndim, 1);
	v_hat.zero();

	for ( int i = 1; i <= domain->giveNumberOfElements(); i++ ) {
#ifdef __FM_MODULE

		if ( Tr21Stokes *T = dynamic_cast< Tr21Stokes * >( domain->giveElement(i) ) ) {
			T->giveIntegratedVelocity(v_hatTemp, tStep);
			v_hat.add(v_hatTemp);
		} else if (Tet21Stokes *T = dynamic_cast< Tet21Stokes * >( domain->giveElement(i) )) {
			T->giveIntegratedVelocity(v_hatTemp, tStep);
			v_hat.add(v_hatTemp);
		}

#endif
	}

	// Compute intrinsic area/volume
	double intrinsicSize=1.0;

	std :: vector <double> V;

	for (int i=0; i<ndim; i++) {
		intrinsicSize=intrinsicSize*(smax.at(i)-smin.at(i));
	}

	for (int i=1; i<=ndim; i++) {
		V.push_back(v_hat.at(i,1)/intrinsicSize);
	}

	fprintf(FID, "\tspecials.velocitymean=[");
	for (int i=0; i<ndim; i++) {
		fprintf(FID, "%e", V.at(i));
		if (i!=(ndim-1)) fprintf (FID, ", ");
	}
	fprintf(FID, "];\n");

	// Output weak periodic boundary conditions
	unsigned int wpbccount = 1;

	for ( int i = 1; i <= domain->giveNumberOfBoundaryConditions(); i++ ) {
		WeakPeriodicBoundaryCondition *wpbc = dynamic_cast< WeakPeriodicBoundaryCondition * >( domain->giveBc(i) );
		if ( wpbc ) {
			for ( int j = 1; j <= wpbc->giveNumberOfInternalDofManagers(); j++ ) {
				fprintf( FID, "\tspecials.weakperiodic{%u}.descType=%u;\n", wpbccount, wpbc->giveBasisType() );
				fprintf(FID, "\tspecials.weakperiodic{%u}.coefficients=[", wpbccount);
				for ( int k = 1; k <= wpbc->giveInternalDofManager(j)->giveNumberOfDofs(); k++ ) {
					FloatArray unknowns;
					IntArray DofMask;
					double X = wpbc->giveInternalDofManager(j)->giveDof(k)->giveUnknown(VM_Total, tStep);
					fprintf(FID, "%e\t", X);
				}

				fprintf(FID, "];\n");
				wpbccount++;
			}
		}
	}
}


void
MatlabExportModule :: initialize()
{}


void
MatlabExportModule :: terminate()
{}


FILE *
MatlabExportModule :: giveOutputStream(TimeStep *tStep)
{
	FILE *answer;

	std :: ostringstream baseFileName;
	std :: string fileName;

	fileName = this->emodel->giveOutputBaseFileName();

	size_t foundDot;
	foundDot = fileName.rfind(".");
	fileName.erase(foundDot);

	functionname = fileName;

	std :: cout << baseFileName.str() << std :: endl;
	std :: cout << functionname << std :: endl;

	size_t foundSlash;

	foundSlash = functionname.find_last_of("/");
	functionname.erase(0, foundSlash + 1);

#ifdef __PARALLEL_MODE
	baseFileName << fileName << "_" << emodel->giveRank() << "_V_" << tStep->giveNumber() << "m";
#else
	baseFileName << fileName << ".m";
#endif

	if ( ( answer = fopen(baseFileName.str().c_str(), "w") ) == NULL ) {
		OOFEM_ERROR2( "MatlabExportModule::giveOutputStream: failed to open file %s", baseFileName.str().c_str() );
	}

	return answer;
}
} // end namespace oofem
