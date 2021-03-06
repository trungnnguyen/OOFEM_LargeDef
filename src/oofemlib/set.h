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

#ifndef set_h
#define set_h

#include "femcmpnn.h"
#include "intarray.h"

namespace oofem {
    
///@name Input fields for Set
//@{
#define _IFT_Set_Name "set"
#define _IFT_Set_nodes "nodes" ///< List of specific node indices.
#define _IFT_Set_nodeRanges "noderanges" ///< List of node index ranges.
#define _IFT_Set_elements "elements" ///< List of specific element indices.
#define _IFT_Set_elementRanges "elementranges" ///< List of element index ranges.
#define _IFT_Set_elementBoundaries "elementboundaries" ///< Interleaved array of element index + boundary number
#define _IFT_Set_elementEdges "elementedges" ///< Interleaved array of element index + edge number
//@}
    
class EntityRenumberingFunction;

/**
 * Set of elements, boundaries, edges and/or nodes.
 * Describes a collection of components which are given easy access to for example boundary conditions.
 * @author Mikael Öhman
 */
class Set : public FEMComponent
{
protected:
    IntArray elements; ///< Element numbers.
    IntArray elementBoundaries; /// Element numbers + boundary numbers (interleaved).
    IntArray elementEdges; /// Element numbers + edge numbers (interleaved).
    IntArray nodes; ///< Node numbers.
    IntArray totalNodes; ///< Unique set of nodes (computed).

public:
    /**
     * Creates a empty set with given number and belonging to given domain.
     * @param n Set number.
     * @param d Domain to which component belongs to.
     */
    Set(int n, Domain *d): FEMComponent(n,d) { }
    virtual ~Set() {}

    virtual IRResultType initializeFrom(InputRecord *ir);

    /**
     * Returns list of elements within set. 
     * @return List of element numbers.
     */
    const IntArray &giveElementList();
    /**
     * Returns list of element boundaries within set.
     * Boundaries are either surfaces (3d), edges (2d), or corners (1d).
     * @return List of element boundaries.
     */
    const IntArray &giveBoundaryList();
    /**
     * Returns list of element edges within set (must be edges of 3D elements).
     * @return List of element edges.
     */
    const IntArray &giveEdgeList();
    /**
     * Returns list of all nodes within set.
     * This list is computed automatically, based on all elements, boundaries, edges, and specified nodes within the set.
     * @return List of node numbers.
     */
    const IntArray &giveNodeList();
    
    /**
     * Sets list of elements within set. 
     */
    void setElementList(const IntArray &newElements);
    /**
     * Sets list of element boundaries within set.
     */
    void setBoundaryList(const IntArray &newBoundaries);
    /**
     * Sets list of element edges within set (must be edges of 3D elements).
     */
    void setEdgeList(const IntArray &newEdges);
    /**
     * Sets list of nodes within set.
     */
    void setNodeList(const IntArray &newNodes);

    /**
     * Clears the entire set.
     */
    void clear();
    virtual void updateLocalNumbering(EntityRenumberingFunctor &f);
    /**
     * Renumbering of nodes (could change due to load balancing).
     */
    void updateLocalNodeNumbering(EntityRenumberingFunctor &f);
    /**
     * Renumbering of nodes (could change due to load balancing).
     */
    void updateLocalElementNumbering(EntityRenumberingFunctor &f);
    
    virtual contextIOResultType saveContext(DataStream *stream, ContextMode mode, void *obj = NULL);
    virtual contextIOResultType restoreContext(DataStream *stream, ContextMode mode, void *obj = NULL);
    
    virtual const char* giveClassName() const { return "Set"; }
    virtual const char *giveInputRecordName() const { return _IFT_Set_Name; }
    
protected:
    /**
     * Converts list ranges to list of individual values + individually specified values.
     */
    void computeIntArray(IntArray &answer, const IntArray &specified, std::list< Range >ranges);
};

}

#endif // set_h
