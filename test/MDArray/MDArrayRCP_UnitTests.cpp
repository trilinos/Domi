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

#include "Domi_MDArrayRCP.hpp"
#include "MDArray_UnitTest_helpers.hpp"

namespace
{

using Teuchos::tuple;
using Teuchos::Array;
using Domi::MDArrayRCP;
using Domi::MDArrayView;
using Domi::Slice;
using Teuchos::RangeError;
using MDArrayUnitTestHelpers::generateMDArrayRCP;

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, defaultConstructor, T )
{
  MDArrayRCP< T > mdar;
  TEST_EQUALITY_CONST(mdar.num_dims(), 1);
  TEST_EQUALITY_CONST(mdar.dimension(0), 0);
  TEST_EQUALITY_CONST(mdar.size(), 0);
  TEST_EQUALITY_CONST(mdar.strides()[0], 1);
  TEUCHOS_ASSERT(mdar.empty());
  TEUCHOS_ASSERT(mdar.is_null());
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, arrayViewDimsConstructor, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  Array< T > a(60);
  MDArrayRCP< T > mdar(a,tuple< size_type >(3,4,5));
  TEST_EQUALITY(mdar.num_dims(), 3);
  TEST_EQUALITY(mdar.dimension(0), 3);
  TEST_EQUALITY(mdar.dimension(1), 4);
  TEST_EQUALITY(mdar.dimension(2), 5);
  TEST_EQUALITY(mdar.size(), 60);
  TEST_EQUALITY(mdar.strides()[0],  1);
  TEST_EQUALITY(mdar.strides()[1],  3);
  TEST_EQUALITY(mdar.strides()[2], 12);
  TEST_EQUALITY(mdar.storage_order(), Domi::DEFAULT_ORDER);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, arrayViewDimsConstructorBad, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  Array< T > a(50);
  TEST_THROW(MDArrayRCP< T > mdar(a,tuple< size_type >(8,8)), RangeError);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, arrayViewDimsOrderConstructor, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  Array< T > a(60);
  MDArrayRCP< T > mdar(a,tuple< size_type >(3,4,5),Domi::C_ORDER);
  TEST_EQUALITY(mdar.num_dims(), 3);
  TEST_EQUALITY(mdar.dimension(0), 3);
  TEST_EQUALITY(mdar.dimension(1), 4);
  TEST_EQUALITY(mdar.dimension(2), 5);
  TEST_EQUALITY(mdar.size(), 60);
  TEST_EQUALITY(mdar.strides()[0], 20);
  TEST_EQUALITY(mdar.strides()[1],  5);
  TEST_EQUALITY(mdar.strides()[2],  1);
  TEST_EQUALITY(mdar.storage_order(), Domi::C_ORDER);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, dimsConstructor, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  MDArrayRCP< T > mdar(tuple< size_type >(3,4,5));
  TEST_EQUALITY(mdar.num_dims(), 3);
  TEST_EQUALITY(mdar.dimension(0), 3);
  TEST_EQUALITY(mdar.dimension(1), 4);
  TEST_EQUALITY(mdar.dimension(2), 5);
  TEST_EQUALITY(mdar.size(), 60);
  TEST_EQUALITY(mdar.strides()[0],  1);
  TEST_EQUALITY(mdar.strides()[1],  3);
  TEST_EQUALITY(mdar.strides()[2], 12);
  TEST_EQUALITY(mdar.storage_order(), Domi::DEFAULT_ORDER);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, dimsValConstructor, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  MDArrayRCP< T > mdar(tuple< size_type >(3,4), 12);
  TEST_EQUALITY(mdar.num_dims(), 2);
  TEST_EQUALITY(mdar.dimension(0), 3);
  TEST_EQUALITY(mdar.dimension(1), 4);
  TEST_EQUALITY(mdar.size(), 12);
  TEST_EQUALITY(mdar.strides()[0],  1);
  TEST_EQUALITY(mdar.strides()[1],  3);
  TEST_EQUALITY(mdar.storage_order(), Domi::DEFAULT_ORDER);
  TEST_EQUALITY(mdar(0,0), 12);
  TEST_EQUALITY(mdar(1,0), 12);
  TEST_EQUALITY(mdar(2,0), 12);
  TEST_EQUALITY(mdar(0,1), 12);
  TEST_EQUALITY(mdar(1,1), 12);
  TEST_EQUALITY(mdar(2,1), 12);
  TEST_EQUALITY(mdar(0,2), 12);
  TEST_EQUALITY(mdar(1,2), 12);
  TEST_EQUALITY(mdar(2,2), 12);
  TEST_EQUALITY(mdar(0,3), 12);
  TEST_EQUALITY(mdar(1,3), 12);
  TEST_EQUALITY(mdar(2,3), 12);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, copyConstructor, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  MDArrayRCP< T > mdar1(tuple< size_type >(2,3), 2);
  MDArrayRCP< T > mdar2(mdar1);
  TEST_EQUALITY(mdar1.num_dims(),      mdar2.num_dims()     );
  TEST_EQUALITY(mdar1.dimension(0),    mdar2.dimension(0)   );
  TEST_EQUALITY(mdar1.dimension(1),    mdar2.dimension(1)   );
  TEST_EQUALITY(mdar1.size(),          mdar2.size()         );
  TEST_EQUALITY(mdar1.strides()[0],    mdar2.strides()[0]   );
  TEST_EQUALITY(mdar1.strides()[1],    mdar2.strides()[1]   );
  TEST_EQUALITY(mdar1.storage_order(), mdar2.storage_order());
  TEST_EQUALITY(mdar1(0,0),            mdar2(0,0)           );
  TEST_EQUALITY(mdar1(1,0),            mdar2(1,0)           );
  TEST_EQUALITY(mdar1(0,1),            mdar2(0,1)           );
  TEST_EQUALITY(mdar1(1,1),            mdar2(1,1)           );
  TEST_EQUALITY(mdar1(0,2),            mdar2(0,2)           );
  TEST_EQUALITY(mdar1(1,2),            mdar2(1,2)           );
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, assignmentOperator, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  MDArrayRCP< T > mdar1(tuple< size_type >(2,2,2), 8);
  MDArrayRCP< T > mdar2 = mdar1;
  TEST_EQUALITY(mdar1.num_dims(),      mdar2.num_dims()     );
  TEST_EQUALITY(mdar1.dimension(0),    mdar2.dimension(0)   );
  TEST_EQUALITY(mdar1.dimension(1),    mdar2.dimension(1)   );
  TEST_EQUALITY(mdar1.dimension(2),    mdar2.dimension(2)   );
  TEST_EQUALITY(mdar1.size(),          mdar2.size()         );
  TEST_EQUALITY(mdar1.strides()[0],    mdar2.strides()[0]   );
  TEST_EQUALITY(mdar1.strides()[1],    mdar2.strides()[1]   );
  TEST_EQUALITY(mdar1.strides()[2],    mdar2.strides()[2]   );
  TEST_EQUALITY(mdar1.storage_order(), mdar2.storage_order());
  TEST_EQUALITY(mdar1(0,0,0),          mdar2(0,0,0)         );
  TEST_EQUALITY(mdar1(1,0,0),          mdar2(1,0,0)         );
  TEST_EQUALITY(mdar1(0,1,0),          mdar2(0,1,0)         );
  TEST_EQUALITY(mdar1(1,1,0),          mdar2(1,1,0)         );
  TEST_EQUALITY(mdar1(0,0,1),          mdar2(0,0,1)         );
  TEST_EQUALITY(mdar1(1,0,1),          mdar2(1,0,1)         );
  TEST_EQUALITY(mdar1(0,1,1),          mdar2(0,1,1)         );
  TEST_EQUALITY(mdar1(1,1,1),          mdar2(1,1,1)         );
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, indexing4D, T )
{
  typedef Domi::Ordinal ord;
  using Teuchos::tuple;
  ord ni = 2;
  ord nj = 2;
  ord nk = 3;
  ord nm = 3;
  MDArrayRCP< T > mdarcp(tuple(ni,nj,nk,nm));
  for (ord m=0; m < nm; m++)
    for (ord k=0; k < nk; k++)
      for (ord j=0; j < nj; j++)
        for (ord i=0; i < ni; i++)
        {
          mdarcp(i,j,k,m) = 3;
          TEST_EQUALITY(mdarcp(i,j,k,m), 3);
        }
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, indexing5D, T )
{
  typedef Domi::Ordinal ord;
  using Teuchos::tuple;
  ord ni = 2;
  ord nj = 2;
  ord nk = 3;
  ord nm = 3;
  ord nn = 4;
  MDArrayRCP< T > mdarcp(tuple(ni,nj,nk,nm,nn));
  for (ord n=0; n < nn; n++)
    for (ord m=0; m < nm; m++)
      for (ord k=0; k < nk; k++)
        for (ord j=0; j < nj; j++)
          for (ord i=0; i < ni; i++)
          {
            mdarcp(i,j,k,m,n) = 4;
            TEST_EQUALITY(mdarcp(i,j,k,m,n), 4);
          }
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, indexing6D, T )
{
  typedef Domi::Ordinal ord;
  using Teuchos::tuple;
  ord ni = 2;
  ord nj = 2;
  ord nk = 3;
  ord nm = 3;
  ord nn = 4;
  ord np = 4;
  MDArrayRCP< T > mdarcp(tuple(ni,nj,nk,nm,nn,np));
  for (ord p=0; p < np; p++)
    for (ord n=0; n < nn; n++)
      for (ord m=0; m < nm; m++)
        for (ord k=0; k < nk; k++)
          for (ord j=0; j < nj; j++)
            for (ord i=0; i < ni; i++)
            {
              mdarcp(i,j,k,m,n,p) = 5;
              TEST_EQUALITY(mdarcp(i,j,k,m,n,p), 5);
            }
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, indexing7D, T )
{
  // Note: this unit test exercises the variadic argument of the last
  // overloaded operator()
  typedef Domi::Ordinal ord;
  using Teuchos::tuple;
  ord ni = 2;
  ord nj = 2;
  ord nk = 3;
  ord nm = 3;
  ord nn = 4;
  ord np = 4;
  ord nq = 3;
  MDArrayRCP< T > mdarcp(tuple(ni,nj,nk,nm,nn,np,nq));
  for (ord q=0; q < nq; q++)
    for (ord p=0; p < np; p++)
      for (ord n=0; n < nn; n++)
        for (ord m=0; m < nm; m++)
          for (ord k=0; k < nk; k++)
            for (ord j=0; j < nj; j++)
              for (ord i=0; i < ni; i++)
              {
                mdarcp(i,j,k,m,n,p,q) = 6;
                TEST_EQUALITY(mdarcp(i,j,k,m,n,p,q), 6);
              }
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, squareBracketOrdinal, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(3,4);
  MDArrayView< T > view = a[0][Slice()];
  TEST_EQUALITY_CONST(view(0), 0);
  TEST_EQUALITY_CONST(view(1), 3);
  TEST_EQUALITY_CONST(view(2), 6);
  TEST_EQUALITY_CONST(view(3), 9);

  view = a[1][Slice()];
  TEST_EQUALITY_CONST(view(0),  1);
  TEST_EQUALITY_CONST(view(1),  4);
  TEST_EQUALITY_CONST(view(2),  7);
  TEST_EQUALITY_CONST(view(3), 10);

  view = a[2][Slice()];
  TEST_EQUALITY_CONST(view(0),  2);
  TEST_EQUALITY_CONST(view(1),  5);
  TEST_EQUALITY_CONST(view(2),  8);
  TEST_EQUALITY_CONST(view(3), 11);

  view = a[1][1];
  TEST_EQUALITY_CONST(view(0), 4);
}


TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, squareBracketOrdinalCOrder, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(3,4, Domi::C_ORDER);
  MDArrayView< T > view = a[0][Slice()];
  TEST_EQUALITY_CONST(view(0), 0);
  TEST_EQUALITY_CONST(view(1), 3);
  TEST_EQUALITY_CONST(view(2), 6);
  TEST_EQUALITY_CONST(view(3), 9);

  view = a[1][Slice()];
  TEST_EQUALITY_CONST(view(0),  1);
  TEST_EQUALITY_CONST(view(1),  4);
  TEST_EQUALITY_CONST(view(2),  7);
  TEST_EQUALITY_CONST(view(3), 10);

  view = a[2][Slice()];
  TEST_EQUALITY_CONST(view(0),  2);
  TEST_EQUALITY_CONST(view(1),  5);
  TEST_EQUALITY_CONST(view(2),  8);
  TEST_EQUALITY_CONST(view(3), 11);

  view = a[1][1];
  TEST_EQUALITY_CONST(view(0), 4);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, squareBracketSlice1, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(4,5);
  MDArrayView< T > view = a[Slice(1,-1)][Slice(1,-1)];
  TEST_EQUALITY_CONST(view(0,0),  5);
  TEST_EQUALITY_CONST(view(0,1),  9);
  TEST_EQUALITY_CONST(view(0,2), 13);
  TEST_EQUALITY_CONST(view(1,0),  6);
  TEST_EQUALITY_CONST(view(1,1), 10);
  TEST_EQUALITY_CONST(view(1,2), 14);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, squareBracketSlice2, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(4,3);
  MDArrayView< T > view = a[Slice(1,-1)][Slice()];
  TEST_EQUALITY_CONST(view(0,0),  1);
  TEST_EQUALITY_CONST(view(0,1),  5);
  TEST_EQUALITY_CONST(view(0,2),  9);
  TEST_EQUALITY_CONST(view(1,0),  2);
  TEST_EQUALITY_CONST(view(1,1),  6);
  TEST_EQUALITY_CONST(view(1,2), 10);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, rangeError, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(3,4);
#ifdef HAVE_DOMI_ARRAY_BOUNDSCHECK
  TEST_THROW(a(3,3), RangeError);
  TEST_THROW(a(0,4), RangeError);
#else
  a(0,0);  // Prevent unused variable warning
#endif
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, rangeErrorCOrder, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(3,4,Domi::C_ORDER);
#ifdef HAVE_DOMI_ARRAY_BOUNDSCHECK
  TEST_THROW(a(3,3), RangeError);
  TEST_THROW(a(0,4), RangeError);
#else
  a(0,0);  // Prevent unused variable warning
#endif
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, legalAt, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(2,3);
  TEST_EQUALITY_CONST(a.at(0,0), 0);
  TEST_EQUALITY_CONST(a.at(1,0), 1);
  TEST_EQUALITY_CONST(a.at(0,1), 2);
  TEST_EQUALITY_CONST(a.at(1,1), 3);
  TEST_EQUALITY_CONST(a.at(0,2), 4);
  TEST_EQUALITY_CONST(a.at(1,2), 5);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, illegalAt, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(2,3);
  TEST_THROW(a.at(2,0), RangeError);
  TEST_THROW(a.at(0,3), RangeError);
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, clearEmpty, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(10,10);
  TEST_EQUALITY_CONST(a.num_dims(), 2);
  TEST_EQUALITY_CONST(a.dimension(0), 10);
  TEST_EQUALITY_CONST(a.dimension(1), 10);
  TEUCHOS_ASSERT(!a.empty());
  a.clear();
  TEST_EQUALITY_CONST(a.num_dims(), 1);
  TEST_EQUALITY_CONST(a.dimension(0), 0);
  TEUCHOS_ASSERT(a.empty());
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, resize, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  MDArrayRCP< T > a = generateMDArrayRCP< T >(11,4);
  TEST_EQUALITY_CONST(a.num_dims(), 2);
  TEST_EQUALITY_CONST(a.dimension(0), 11);
  TEST_EQUALITY_CONST(a.dimension(1),  4);
  TEUCHOS_ASSERT(!a.empty());
  a.resize(tuple< size_type >(5,9,3));
  TEST_EQUALITY_CONST(a.num_dims(), 3);
  TEST_EQUALITY_CONST(a.dimension(0), 5);
  TEST_EQUALITY_CONST(a.dimension(1), 9);
  TEST_EQUALITY_CONST(a.dimension(2), 3);
  TEUCHOS_ASSERT(!a.empty());
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, toStringNull, T )
{
  MDArrayRCP< T > a;
  TEST_EQUALITY_CONST(a.toString(), "[]");
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, toString1D, T )
{
  typedef typename MDArrayRCP< T >::size_type size_type;
  T val = 3;
  MDArrayRCP< T > a(tuple< size_type >(3), val);
  TEST_EQUALITY_CONST(a.toString(), "[3, 3, 3]");
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, toString2D, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(2,3);
  TEST_EQUALITY_CONST(a.toString(), "[[0, 2, 4],\n [1, 3, 5]]");
}

TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( MDArrayRCP, toString3D, T )
{
  MDArrayRCP< T > a = generateMDArrayRCP< T >(2,3,2);
  TEST_EQUALITY_CONST(a.toString(), "[[[0, 1],\n  [2, 3],\n  [4, 5]],\n [[6, 7],\n  [8, 9],\n  [10, 11]]]");
}

//
// Instantiations
//
#ifdef HAVE_DOMI_ARRAY_BOUNDSCHECK
#  define DEBUG_UNIT_TEST_GROUP( T )
#else // HAVE_DOMI_ARRAY_BOUNDSCHECK
#  define DEBUG_UNIT_TEST_GROUP( T )
#endif // HAVE_DOMI_ARRAY_BOUNDSCHECK

#define UNIT_TEST_GROUP( T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, defaultConstructor, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, arrayViewDimsConstructor, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, arrayViewDimsConstructorBad, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, arrayViewDimsOrderConstructor, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, dimsConstructor, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, dimsValConstructor, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, copyConstructor, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, assignmentOperator, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, indexing4D, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, indexing5D, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, indexing6D, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, indexing7D, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, squareBracketOrdinal, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, squareBracketOrdinalCOrder, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, squareBracketSlice1, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, squareBracketSlice2, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, rangeError, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, rangeErrorCOrder, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, legalAt, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, illegalAt, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, clearEmpty, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, resize, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, toStringNull, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, toString1D, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, toString2D, T ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( MDArrayRCP, toString3D, T ) \
  DEBUG_UNIT_TEST_GROUP( T )

UNIT_TEST_GROUP(int)
#if 1
UNIT_TEST_GROUP(long)
UNIT_TEST_GROUP(float)
UNIT_TEST_GROUP(double)
#endif

}  // End anonymous namespace
