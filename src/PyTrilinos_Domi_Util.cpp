// @HEADER
// ***********************************************************************
//
//              PyTrilinos: Python Interface to Trilinos
//                 Copyright (2005) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
// USA
// Questions? Contact Bill Spotz (wfspotz@sandia.gov)
//
// ***********************************************************************
// @HEADER

#include "PyTrilinos_Domi_Util.hpp"
#include "PyTrilinos_PythonException.h"

////////////////////////////////////////////////////////////////////////

namespace PyTrilinos
{

////////////////////////////////////////////////////////////////////////

Teuchos::RCP< const Domi::MDComm >
convertToMDComm(const Teuchos::RCP< const Teuchos::Comm< int > > teuchosComm,
                const DistArrayProtocol & distarray)
{
  // Get the number of dimensions and allocate the MDComm arrays
  int numDims = distarray.num_dims();
  Teuchos::Array< int > commDims(numDims);
  Teuchos::Array< int > periodic(numDims);

  // Fill in the MDComm arrays
  for (int axis = 0; axis < numDims; ++axis)
  {
    commDims[axis] = distarray.proc_grid_size(axis);
    periodic[axis] = distarray.periodic(axis) ? 1 : 0;
  }

  // Return the result
  return Teuchos::rcp(new Domi::MDComm(teuchosComm, commDims, periodic));
}

////////////////////////////////////////////////////////////////////////

Teuchos::RCP< const Domi::MDMap<> >
convertToMDMap(const Teuchos::RCP< const Teuchos::Comm< int > > teuchosComm,
               const DistArrayProtocol & distarray)
{
  // Get the equivalent MDComm.
  Teuchos::RCP< const Domi::MDComm > mdComm =
    convertToMDComm(teuchosComm, distarray);

  // Get the number of dimensions and check the types of the
  // distributions
  int numDims = mdComm->numDims();
  for (int axis = 0; axis < numDims; ++axis)
  {
    DistributionType distType = distarray.dist_type(axis);

    // Check the distribution type
    if (distType == NONE)
    {
      PyErr_Format(PyExc_ValueError,
                   "'dist_type' for axis %d = 'NONE' is invalid",
                   axis);
      throw PythonException();
    }
    if (distType == CYCLIC)
    {
      PyErr_Format(PyExc_ValueError,
                   "'dist_type' for axis %d = 'CYCLIC' is invalid",
                   axis);
      throw PythonException();
    }
    if (distType == UNSTRUCTURED)
    {
      PyErr_Format(PyExc_ValueError,
                   "'dist_type' for axis %d = 'UNSTRUCTURED' is invalid",
                   axis);
      throw PythonException();
    }
  }

  // Allocate the MDMap constructor arrays
  Teuchos::Array< Domi::Slice > myGlobalBounds(numDims);
  Teuchos::Array< PaddingType > padding(numDims);

  // Fill in the MDMap constructor arrays
  for (int axis = 0; axis < numDims; ++axis)
  {
    int start = distarray.start(axis);
    int stop  = distarray.stop( axis);
    myGlobalBounds[axis] = Domi::ConcreteSlice(start, stop);
    padding[axis]  = distarray.padding( axis);
  }

  // Get the buffer layout
  Domi::Layout layout =
    PyArray_IS_C_CONTIGUOUS((PyArrayObject*) distarray.buffer()) ?
      Domi::C_ORDER : Domi::FORTRAN_ORDER;

  // Return the result
  return Teuchos::rcp(new Domi::MDMap<>(mdComm,
                                        myGlobalBounds,
                                        padding,
                                        layout));
}

////////////////////////////////////////////////////////////////////////

PyObject *
convertToPySlice(const Domi::Slice domiSlice)
{
  PyObject * pyStart = NULL;
  PyObject * pyStop  = NULL;
  PyObject * pyStep  = NULL;
  PyObject * pySlice;
  if (domiSlice.start() != Domi::Slice::Default)
    pyStart = PyInt_FromLong((long)domiSlice.start());
  if (domiSlice.stop()  != Domi::Slice::Default)
    pyStop = PyInt_FromLong((long)domiSlice.stop());
  if (domiSlice.step()  != Domi::Slice::Default)
    pyStep = PyInt_FromLong((long)domiSlice.step());
  return PySlice_New(pyStart, pyStop, pyStep);
}

////////////////////////////////////////////////////////////////////////

Domi::Slice
convertToDomiSlice(PySliceObject * pySlice,
                   Py_ssize_t length)
{
  Py_ssize_t pyStart  = 0;
  Py_ssize_t pyStop   = 0;
  Py_ssize_t pyStep   = 0;
  Py_ssize_t sliceLen = 0;
  int rcode = PySlice_GetIndicesEx(pySlice, length , &pyStart ,
                                   &pyStop, &pyStep, &sliceLen);
  Domi::Slice domiSlice((Domi::dim_type) pyStart,
                        (Domi::dim_type) pyStop ,
                        (Domi::dim_type) pyStep );
  return domiSlice;
}

////////////////////////////////////////////////////////////////////////

}
