/*
// @HEADER
// ***********************************************************************
//
//            Domi: Multidimensional Datastructures Package
//                 Copyright (2013) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
// @HEADER
*/

// System include
#include <cstdlib>

// Teuchos includes
#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_DefaultComm.hpp"
#include "Teuchos_Tuple.hpp"

// Domi includes
#include "Domi_ConfigDefs.hpp"
#include "Domi_Utils.hpp"
#include "Domi_MDVector.hpp"

namespace
{

using std::string;
using Teuchos::Array;
using Teuchos::ArrayView;
using Teuchos::Tuple;
using Teuchos::rcp;
typedef Domi::Ordinal Ordinal;
typedef Domi::size_type size_type;
typedef Domi::dim_type dim_type;
using Domi::TeuchosCommRCP;
using Domi::MDArray;
using Domi::MDArrayView;
using Domi::Slice;
const Ordinal & Default = Domi::Slice::Default;
using Domi::MDComm;
using Domi::MDCommRCP;
using Domi::MDMap;
using Domi::MDVector;

int numDims = 2;
string commDimsStr = "-1";
Array< int > commDims;

////////////////////////////////////////////////////////////////////////

/** \brief Assign each element of an MDVector a unique global ID
 *
 * \param mdv [in] an MDVector, whose values will be re-assigned
 *
 */
template< typename Sca, class Node >
void assignGlobalIDsToMDVector(MDVector< Sca, Node > & mdv)
{
  // Give the iterator a simpler name
  typedef typename MDArrayView< Sca >::iterator iterator;
  // Get a pointer to the underlying MDMap
  const Teuchos::RCP< const MDMap< Node > > mdMap = mdv.getMDMap();
  // Loop over the underlying MDArrayView
  MDArrayView< Sca > mdav = mdv.getDataNonConst();
  for (iterator it = mdav.begin(); it != mdav.end(); ++it)
  {
    // Obtain the local index
    Array< dim_type > localIndex(mdv.numDims());
    for (int axis = 0; axis < mdv.numDims(); ++axis)
      localIndex[axis] = it.index(axis);
    // Convert the local index to a local ID, and then convert to a
    // global ID and store
    size_type localID = mdMap->getLocalID(localIndex);
    *it = (Sca) mdMap->getGlobalID(localID);
  }
}

////////////////////////////////////////////////////////////////////////

TEUCHOS_STATIC_SETUP()
{
  Teuchos::CommandLineProcessor &clp = Teuchos::UnitTestRepository::getCLP();
  clp.addOutputSetupOptions(true);
  clp.setOption("numDims" , &numDims    , "number of dimensions");
  clp.setOption("commDims", &commDimsStr, "comma-separated list of "
                "number of processors along each axis");
}

////////////////////////////////////////////////////////////////////////

//
// Templated Unit Tests
//

////////////////////////////////////////////////////////////////////////

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDVector, SliceLow, Sca )
{
  // Initialize the MDComm
  TeuchosCommRCP comm = Teuchos::DefaultComm< int >::getComm();
  commDims = Domi::splitStringOfIntsWithCommas(commDimsStr);
  MDCommRCP mdComm = Teuchos::rcp(new MDComm(comm, numDims, commDims));

  // Ensure that the commDims are completely specified
  commDims.resize(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    commDims[axis] = mdComm->getCommDim(axis);

  // Construct global dimensions and strides
  dim_type localDim = 10;
  Array< dim_type > dims(numDims);
  Array< dim_type > strides(numDims);
  for (int axis = 0; axis < numDims; ++axis)
  {
    dims[axis] = localDim * mdComm->getCommDim(axis);
    if (axis == 0) strides[0] = 1;
    else strides[axis] = strides[axis-1] * dims[axis-1];
  }

  // Construct an MDMap and MDVector
  typedef Teuchos::RCP< MDMap<> > MDMapRCP;
  MDMapRCP mdMap = rcp(new MDMap<>(mdComm, dims()));
  MDVector< Sca > mdVector(mdMap);
  assignGlobalIDsToMDVector(mdVector);

  // Perform tests along each axis
  dim_type width = 2 * localDim / 3;
  Slice slice(0, width);
  for (int axis = 0; axis < numDims; ++axis)
  {
    // Construct the sub-vector
    MDVector< Sca > subVector(mdVector, axis, slice);

    // Check whether on-processor or not
    if (commDims[axis] > 1 && mdComm->getCommIndex(axis) > 0)
    {
      TEST_ASSERT(!subVector.onSubcommunicator());
    }
    else
    {
      // Compute the sub-vector statistics
      bool contig = (axis == numDims-1);
      Sca  begin  = 0;
      Sca  end    = 0;
      for (int myAxis = 0; myAxis < numDims; ++myAxis)
      {
        dim_type min =  mdMap->getCommIndex(myAxis)    * localDim;
        dim_type max = (mdMap->getCommIndex(myAxis)+1) * localDim;
        begin += min * strides[myAxis];
        if (myAxis == axis) end += (width-1) * strides[myAxis];
        else end += (max-1) * strides[myAxis];
      }
      MDArrayView< const Sca > subArray = subVector.getData();

      // Perform the unit tests
      TEST_ASSERT(subVector.onSubcommunicator());
      TEST_EQUALITY(subVector.isContiguous()           , contig );
      TEST_EQUALITY(subVector.numDims()                , numDims);
      TEST_EQUALITY(subVector.getGlobalDim(axis)       , width  );
      TEST_EQUALITY(subVector.getGlobalBounds(axis)    , slice  );
      TEST_EQUALITY(subVector.getGlobalRankBounds(axis), slice  );
      TEST_EQUALITY(subVector.getLocalDim(axis)        , width  );
      TEST_EQUALITY(subVector.getLocalBounds(axis)     , slice  );
      TEST_EQUALITY(subVector.getLowerPadSize(axis)    , 0      );
      TEST_EQUALITY(subVector.getUpperPadSize(axis)    , 0      );
      TEST_EQUALITY(subVector.getCommPadSize(axis)     , 0      );
      TEST_EQUALITY(subVector.getLowerBndryPad(axis)   , 0      );
      TEST_EQUALITY(subVector.getUpperBndryPad(axis)   , 0      );
      TEST_EQUALITY(subVector.getBndryPadSize(axis)    , 0      );
      TEST_EQUALITY(*(subArray.begin() ), begin);
      TEST_EQUALITY(*(subArray.rbegin()), end  );
    }
  }
}

////////////////////////////////////////////////////////////////////////

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDVector, SliceMid, Sca )
{
  // Initialize the MDComm
  TeuchosCommRCP comm = Teuchos::DefaultComm< int >::getComm();
  commDims = Domi::splitStringOfIntsWithCommas(commDimsStr);
  MDCommRCP mdComm = Teuchos::rcp(new MDComm(comm, numDims, commDims));

  // Ensure that the commDims are completely specified
  commDims.resize(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    commDims[axis] = mdComm->getCommDim(axis);

  // Construct global dimensions and strides
  dim_type localDim = 10;
  Array< dim_type > dims(numDims);
  Array< dim_type > strides(numDims);
  for (int axis = 0; axis < numDims; ++axis)
  {
    dims[axis] = localDim * mdComm->getCommDim(axis);
    if (axis == 0) strides[0] = 1;
    else strides[axis] = strides[axis-1] * dims[axis-1];
  }

  // Construct an MDMap and MDVector
  typedef Teuchos::RCP< MDMap<> > MDMapRCP;
  MDMapRCP mdMap = rcp(new MDMap<>(mdComm, dims()));
  MDVector< Sca > mdVector(mdMap);
  assignGlobalIDsToMDVector(mdVector);

  // Perform tests along each axis
  dim_type start = localDim / 3;
  for (int axis = 0; axis < numDims; ++axis)
  {
    // Construct the sub-vector
    dim_type stop  = dims[axis] - localDim / 3;
    dim_type width = stop - start;
    Slice slice(start, stop);
    MDVector< Sca > subVector(mdVector, axis, slice);

    // Compute sub-vector stats
    bool contig = (axis == numDims-1);
    dim_type globalRankStart = start;
    dim_type globalRankStop  = stop;
    dim_type myStart = 0;
    dim_type myStop  = localDim;
    int commIndex = mdComm->getCommIndex(axis);
    if (commIndex > 0)                 globalRankStart = localDim*commIndex;
    if (commIndex < commDims[axis]-1)  globalRankStop  = localDim*(commIndex+1);
    if (commIndex == 0)                myStart = localDim / 3;
    if (commIndex == commDims[axis]-1) myStop  = localDim - localDim / 3;
    dim_type myWidth = myStop - myStart;
    Slice grs     = Slice(globalRankStart, globalRankStop);
    Slice mySlice = Slice(myWidth);
    Sca begin = 0;
    Sca end   = 0;
    for (int myAxis = 0; myAxis < numDims; ++myAxis)
    {
      if (myAxis == axis)
      {
        begin +=  globalRankStart   * strides[myAxis];
        end   += (globalRankStop-1) * strides[myAxis];
      }
      else
      {
        dim_type min =  mdMap->getCommIndex(myAxis)    * localDim;
        dim_type max = (mdMap->getCommIndex(myAxis)+1) * localDim;
        begin       +=  min    * strides[myAxis];
        end         += (max-1) * strides[myAxis];
      }
    }
    MDArrayView< const Sca > subArray = subVector.getData();

    // Perform the unit tests
    TEST_ASSERT(subVector.onSubcommunicator());
    TEST_EQUALITY(subVector.isContiguous()           , contig );
    TEST_EQUALITY(subVector.numDims()                , numDims);
    TEST_EQUALITY(subVector.getGlobalDim(axis)       , width  );
    TEST_EQUALITY(subVector.getGlobalBounds(axis)    , slice  );
    TEST_EQUALITY(subVector.getGlobalRankBounds(axis), grs    );
    TEST_EQUALITY(subVector.getLocalDim(axis)        , myWidth);
    TEST_EQUALITY(subVector.getLocalBounds(axis)     , mySlice);
    TEST_EQUALITY(subVector.getLowerPadSize(axis)    , 0      );
    TEST_EQUALITY(subVector.getUpperPadSize(axis)    , 0      );
    TEST_EQUALITY(subVector.getCommPadSize(axis)     , 0      );
    TEST_EQUALITY(subVector.getLowerBndryPad(axis)   , 0      );
    TEST_EQUALITY(subVector.getUpperBndryPad(axis)   , 0      );
    TEST_EQUALITY(subVector.getBndryPadSize(axis)    , 0      );
    TEST_EQUALITY(*(subArray.begin() ), begin);
    TEST_EQUALITY(*(subArray.rbegin()), end  );
  }
}

////////////////////////////////////////////////////////////////////////

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDVector, SliceHi, Sca )
{
  // Initialize the MDComm
  TeuchosCommRCP comm = Teuchos::DefaultComm< int >::getComm();
  commDims = Domi::splitStringOfIntsWithCommas(commDimsStr);
  MDCommRCP mdComm = Teuchos::rcp(new MDComm(comm, numDims, commDims));

  // Ensure that the commDims are completely specified
  commDims.resize(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    commDims[axis] = mdComm->getCommDim(axis);

  // Construct global dimensions and strides
  dim_type localDim = 10;
  Array< dim_type > dims(numDims);
  Array< dim_type > strides(numDims);
  for (int axis = 0; axis < numDims; ++axis)
  {
    dims[axis] = localDim * mdComm->getCommDim(axis);
    if (axis == 0) strides[0] = 1;
    else strides[axis] = strides[axis-1] * dims[axis-1];
  }

  // Construct an MDMap and MDVector
  typedef Teuchos::RCP< MDMap<> > MDMapRCP;
  MDMapRCP mdMap = rcp(new MDMap<>(mdComm, dims()));
  MDVector< Sca > mdVector(mdMap);
  assignGlobalIDsToMDVector(mdVector);

  // Perform tests along each axis
  dim_type width = 2 * localDim / 3;
  for (int axis = 0; axis < numDims; ++axis)
  {
    // Construct the sub-vector
    Slice slice(dims[axis]-width, dims[axis]);
    MDVector< Sca > subVector(mdVector, axis, slice);

    // Check whether on-processor or not
    if (commDims[axis] > 1 &&
        mdComm->getCommIndex(axis) < mdComm->getCommDim(axis)-1)
    {
      TEST_ASSERT(!subVector.onSubcommunicator());
    }
    else
    {
      // Compute the sub-vector statistics
      bool contig = (axis == numDims-1);
      Slice local(0, width);
      Sca  begin  = 0;
      Sca  end    = 0;
      for (int myAxis = 0; myAxis < numDims; ++myAxis)
      {
        if (myAxis == axis)
        {
          begin += (dims[axis]-width) * strides[axis];
          end   += (dims[axis]-1    ) * strides[myAxis];
        }
        else
        {
          dim_type min =  mdMap->getCommIndex(myAxis)    * localDim;
          dim_type max = (mdMap->getCommIndex(myAxis)+1) * localDim;
          begin +=  min    * strides[myAxis];
          end   += (max-1) * strides[myAxis];
        }
      }
      MDArrayView< const Sca > subArray = subVector.getData();

      // Perform the unit tests
      TEST_ASSERT(subVector.onSubcommunicator());
      TEST_EQUALITY(subVector.isContiguous()           , contig );
      TEST_EQUALITY(subVector.numDims()                , numDims);
      TEST_EQUALITY(subVector.getGlobalDim(axis)       , width  );
      TEST_EQUALITY(subVector.getGlobalBounds(axis)    , slice  );
      TEST_EQUALITY(subVector.getGlobalRankBounds(axis), slice  );
      TEST_EQUALITY(subVector.getLocalDim(axis)        , width  );
      TEST_EQUALITY(subVector.getLocalBounds(axis)     , local  );
      TEST_EQUALITY(subVector.getLowerPadSize(axis)    , 0      );
      TEST_EQUALITY(subVector.getUpperPadSize(axis)    , 0      );
      TEST_EQUALITY(subVector.getCommPadSize(axis)     , 0      );
      TEST_EQUALITY(subVector.getLowerBndryPad(axis)   , 0      );
      TEST_EQUALITY(subVector.getUpperBndryPad(axis)   , 0      );
      TEST_EQUALITY(subVector.getBndryPadSize(axis)    , 0      );
      TEST_EQUALITY(*(subArray.begin() ), begin);
      TEST_EQUALITY(*(subArray.rbegin()), end  );
    }
  }
}

////////////////////////////////////////////////////////////////////////

#define UNIT_TEST_GROUP( Sca ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDVector, SliceLow, Sca ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDVector, SliceMid, Sca ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDVector, SliceHi , Sca )

UNIT_TEST_GROUP(double)
#if 0
UNIT_TEST_GROUP(float)
UNIT_TEST_GROUP(int)
UNIT_TEST_GROUP(long)
#endif

}  // namespace
