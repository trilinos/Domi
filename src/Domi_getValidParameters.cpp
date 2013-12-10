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
// Questions? Contact William F. Spotz (wfspotz@sandia.gov)
//
// ***********************************************************************
// @HEADER


#include "Domi_getValidParameters.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_StandardParameterEntryValidators.hpp"
#include "Domi_Utils.hpp"

using std::string;

namespace Domi
{

  Teuchos::RCP< const Teuchos::ParameterList > getValidParameters() 
  {
    static Teuchos::RCP< const Teuchos::ParameterList > result;

    if ( result.is_null() )
    {

      Teuchos::ParameterList* plist = new Teuchos::ParameterList;

      /* "axis comm sizes" parameter applies to MDComm */
      Teuchos::Array< int > axisCommSizes(1);
      axisCommSizes[0] = -1;
      plist->set("axis comm sizes",
                 axisCommSizes, 
                 "An array of ints whose length is the number of dimensions "
                 "of the problem and whose entries specify the size of the "
                 "MDComm along each axis. A negative value tells Domi to "
                 "fill in a logical value based on the total number of "
                 "processors");

      /* "periodic" parameter applies to MDComm */
      Teuchos::Array< int > periodic(1);
      periodic[0] = 0;
      plist->set("periodic",
                 periodic,
                 "An array of int flags specifying whether each axis is a "
                 "periodic axis. If the periodic array is shorter than the "
                 "length of axisCommSizes array, then the unspecified "
                 "entries are given a default value of zero (not "
                 "periodic).");

      /* "dimensions" parameter applies to MDMap */
      Teuchos::Array< long long > dimensions(1);
      dimensions[0] = 0;
      plist->set("dimensions",
                 dimensions,
                 "An array of ordinals specifying the global dimensions of "
                 "the MDMap. The length of this array should be the same as "
                 "the length of the axisCommSizes array. Note that this is "
                 "an array of long long and it will need to be copied to an "
                 "array of type GlobalOrd.");

      Teuchos::Array< int > pad;
      /* "boundary pad" parameter applies to MDMap */
      plist->set("boundary pad",
                 pad,
                 "An array of ints specifying the size of the boundary "
                 "padding along each axis. All unspecified entries are "
                 "assumed to be zero.");

      /* "communication pad" parameter applies to MDMap */
      plist->set("communication pad",
                 pad,
                 "An array of ints specifying the size of the commmunication "
                 "padding along each axis. All unspecified entries are "
                 "assumed to be zero.");

      /* "layout" parameter applies to MDMap */
      string layout = "Default";
      Teuchos::ArrayView< const string > layoutOpts =
        Teuchos::tuple(string("C Order"),
                       string("Fortran Order"),
                       string("Row Major"),
                       string("Column Major"),
                       string("Last Index Fastest"),
                       string("First Index Fastest"),
                       string("Default"));
      Teuchos::ArrayView< const string > layoutDocs =
        Teuchos::tuple(string("C storage order (last index varies fastest)"),
                       string("Fortran storage order (first index varies "
                              "fastest)"),
                       string("Row major storage order (last index varies "
                              "fastest)"),
                       string("Column major storage order (first index "
                              "varies fastest)"),
                       string("Last index varies fastest"),
                       string("First index varies fastest"),
                       string("Fortran storage order"));
      Teuchos::ArrayView< const int > layoutVals =
        Teuchos::tuple(0, 1, 0, 1, 0, 1, 1);
      Teuchos::RCP< const Teuchos::ParameterEntryValidator > layoutValidator =
        Teuchos::rcp(new Teuchos::StringToIntegralParameterEntryValidator< int >
                     (layoutOpts,
                      layoutDocs,
                      layoutVals,
                      string("Default"),
                      false));
      plist->set("layout",
                 layout,
                 "A string indicating how the data is laid out in memory. "
                 "Default is currently set to Fortran order.",
                 layoutValidator);

      result.reset(plist);
    }

    return result; 
  }

}
