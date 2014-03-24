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

#ifndef DOMI_MDVECTOR_HPP
#define DOMI_MDVECTOR_HPP

// Standard includes
#include <ctime>

// Domi includes
#include "Domi_ConfigDefs.hpp"
#include "Domi_MDMap.hpp"
#include "Domi_MDArrayRCP.hpp"

// Teuchos includes
#include "Teuchos_Describable.hpp"
#include "Teuchos_ScalarTraitsDecl.hpp"
#include "Teuchos_Comm.hpp"
#include "Teuchos_CommHelpers.hpp"

#ifdef HAVE_EPETRA
#include "Epetra_Vector.h"
#endif

#ifdef HAVE_TPETRA
#include "Tpetra_Vector.hpp"
#endif


#ifdef OPEN_MPI
// I provide dummy definitions for MPI structs so that
// typeid(MPI_Datatype) and typeid(MPI_Request) will compile.
// Defining empty structs like this should be fine since only the guts
// of OpenMPI will ever see the real definitions.  This is a silly game
// we are playing with the C++ type system here but it should work
// just fine.
struct ompi_datatype_t {};
//struct ompi_request_t {};
#endif

using std::cout;
using std::endl;

namespace Domi
{

/** \brief Multi-dimensional distributed vector
 *
 * The <tt>MDVector</tt> class represents a multi-dimensional vector
 * distributed according to a multi-dimensional <tt>MDMap</tt>.
 */
template< class Scalar,
          class Node = Kokkos::DefaultNode::DefaultNodeType >
class MDVector : Teuchos::Describable
{
public:

  /** \name Constructors and destructor */
  //@{

  /** \brief Main constructor
   *
   * \param mdMap [in] MDMap that describes the domain decomposition
   *        of this MDVector
   *
   * \param zeroOut [in] flag to initialize all data to zero
   */
  MDVector(const Teuchos::RCP< const MDMap< Node > > & mdMap,
           bool zeroOut = true);

  /** \brief Augmented constructor
   *
   * \param mdMap [in] MDMap that describes the domain decomposition
   *        of this MDVector
   *
   * \param leadingDim [in] leading augmentation dimension, typically
   *        used to provide additional degrees of freedom at each
   *        index defined by the MDMap
   *
   * \param trailingDim [in] trailing augmentation dimension, typically
   *        used to provide additional degrees of freedom at each
   *        index defined by the MDMap
   *
   * \param zeroOut [in] flag to initialize all data to zero
   *
   * If leadingDim or trailingDim is less than 2, then the MDMap will
   * not be augmented with a leading dimension or trailing dimension,
   * respectively.  Note that this constructor takes the given MDMap
   * and computes a new, augmented MDComm and a new MDMap for this
   * MDVector.
   */
  MDVector(const Teuchos::RCP< const MDMap< Node > > & mdMap,
           const dim_type leadingDim,
           const dim_type trailingDim = 0,
           bool zeroOut = true);

  /** \brief Constructor with initialization values (copy)
   *
   * \param mdMap [in] MDMap that describes the domain decomposition
   *        of this MDVector
   *
   * \param source [in] initialization values
   */
  MDVector(const Teuchos::RCP< const MDMap< Node > > & mdMap,
           const MDArrayView< Scalar > & source);

  /** \brief Copy constructor
   *
   * \param source [in] source MDVector
   */
  MDVector(const MDVector< Scalar, Node > & source);

  /** \brief Constructor with Teuchos::Comm and ParameterList
   *
   * \param teuchosComm [in] The Teuchos Communicator.  Note that an
   *        MDComm and MDMap will be constructed from the information
   *        in plist.
   *
   * \param plist [in] ParameterList with construction information
   *        \htmlonly
   *        <iframe src="domi.xml" width="90%"height="400px"></iframe>
   *        <hr />
   *        \endhtmlonly
   *
   * \param node [in] the Kokkos node of the map
   */
  MDVector(const TeuchosCommRCP teuchosComm,
           Teuchos::ParameterList & plist,
           const Teuchos::RCP< Node > & node =
             Kokkos::DefaultNode::getDefaultNode());

  /** \brief Constructor with MDComm and ParameterList
   *
   * \param mdComm [in] an RCP of an MDComm (multi-dimensional
   *        communicator), on which this MDMap will be built.
   *
   * \param plist [in] ParameterList with construction information
   *        \htmlonly
   *        <iframe src="domi.xml" width="90%"height="400px"></iframe>
   *        <hr />
   *        \endhtmlonly
   *
   * \param node [in] the Kokkos node of the map
   */
  MDVector(const MDCommRCP mdComm,
           Teuchos::ParameterList & plist,
           const Teuchos::RCP< Node > & node =
             Kokkos::DefaultNode::getDefaultNode());

  /** \brief Parent/single global ordinal sub-vector constructor
   *
   * \param parent [in] an MDVector, from which this MDVector will be
   *        derived
   *
   * \param axis [in] the axis to which this index ordinal applies
   *
   * \param index [in] the global ordinal that defines the sub-vector
   */
  MDVector(const MDVector< Scalar, Node > & parent,
           int axis,
           dim_type index);

  /** \brief Parent/single slice sub-vector constructor
   *
   * \param parent [in] an MDVector, from which this MDVector will be
   *        derived
   *
   * \param axis [in] the axis to which this slice applies
   *
   * \param slice [in] the slice describing the sub-vector
   *
   * \param bndryPad [in] the boundary padding along the altered axis
   *        of the new sub-vector.  This may include indexes from the
   *        boundary padding of the parent MDVector, but it does not
   *        have to.
   */
  MDVector(const MDVector< Scalar, Node > & parent,
           int axis,
           const Slice & slice,
           int bndryPad = 0);

  /** \brief Create a cloned MDVector for a different node type
   *
   * \param node2 [in] the new node
   */
  template< class Node2 >
  Teuchos::RCP< MDVector< Scalar, Node2 > >
  clone(const Teuchos::RCP< Node2 > & node2) const;

  /** \brief Destructor 
   */
  virtual ~MDVector();

  //@}

  /** \name MDMap methods */
  //@{

  /** \brief MDMap accessor method
   */
  const Teuchos::RCP< const MDMap< Node > >
  getMDMap() const;

  /** \brief Query whether this processor is on the sub-communicator
   *
   * Sub-communicators are formed when a parent MDVector is sliced by
   * using the (parent,ordinal) or (parent,slice) constructors.  For a
   * full communicator, this method will always return true.
   */
  bool onSubcommunicator() const;

  /** \brief Get the Teuchos communicator
   *
   * Note that if the communicator is not a full communicator, i.e. a
   * sub-communicator, that the underlying Comm pointer may be NULL,
   * depending on this processor's rank.
   */
  TeuchosCommRCP getTeuchosComm() const;

  /** \brief Get the number of dimensions
   *
   * This method will return 0 if the communicator is a
   * sub-communicator and this processor does not belong to the
   * sub-communicator.
   */
  int numDims() const;

  /** \brief Get the communicator size along the given axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * This method will throw a Domi::SubcommunicatorError if the
   * communicator is a sub-communicator and this processor does not
   * belong to the sub-communicator.
   */
  int getCommDim(int axis) const;

  /** \brief Return the periodic flag for the given axis.
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * This method will throw a Domi::SubcommunicatorError if the
   * communicator is a sub-communicator and this processor does not
   * belong to the sub-communicator.
   */
  bool isPeriodic(int axis) const;

  /** \brief Get the axis rank of this processor
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * This method will throw a Domi::SubcommunicatorError if the
   * communicator is a sub-communicator and this processor does not
   * belong to the sub-communicator.
   */
  int getCommIndex(int axis) const;

  /** \brief Get the rank of the lower neighbor
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * This method will throw a Domi::SubcommunicatorError if the
   * communicator is a sub-communicator and this processor does not
   * belong to the sub-communicator.
   *
   * If the periodic flag for the given axis is off, and the axis rank
   * of the calling processor is zero, then this method returns -1.
   *
   * If the periodic flag for the given axis is on, and the axis rank
   * of the calling processor is the highest axis rank processor along
   * this axis, then the returned lower neighbor will be zero.
   */
  int getLowerNeighbor(int axis) const;

  /** \brief Get the rank of the upper neighbor
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * This method will throw a Domi::SubcommunicatorError if the
   * communicator is a sub-communicator and this processor does not
   * belong to the sub-communicator.
   *
   * If the periodic flag for the given axis is off, and the axis rank
   * of the calling processor is the highest axis rank processor along
   * this axis, then this method returns -1.
   *
   * If the periodic flag for the given axis is on, and the axis rank
   * of the calling processor is zero, then the returned lower
   * neighbor will be the highest axis rank processor along this axis.
   */
  int getUpperNeighbor(int axis) const;

  /** \brief Get the global dimension along the specified axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * \param withBndryPad [in] specify whether the dimension should
   *        include boundary padding or not
   */
  dim_type getGlobalDim(int axis, bool withBndryPad=false) const;

  /** \brief Get the bounds of the global problem
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * \param withBndryPad [in] specify whether the bounds should
   *        include boundary padding or not
   */
  Slice getGlobalBounds(int axis, bool withBndryPadding=false) const;

  /** \brief Get the global loop bounds along the specified axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * \param withBndryPadding [in] specify whether the dimension should
   *        include boundary padding or not
   *
   * The loop bounds are returned in the form of a <tt>Slice</tt>, in
   * which the <tt>start()</tt> method returns the loop begin value,
   * and the <tt>stop()</tt> method returns the non-inclusive end
   * value.
   */
  Slice getGlobalRankBounds(int axis, bool withBndryPad=false) const;

  /** \brief Get the local dimension along the specified axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * \param withCommPad [in] specify whether the dimension should
   *        include communication padding or not
   */
  dim_type getLocalDim(int axis, bool withCommPad=false) const;

  /** \brief Get the local dimension along the specified axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * \param withCommPad [in] specify whether the dimension should
   *        include communication padding or not
   *
   * The loop bounds are returned in the form of a <tt>Slice</tt>, in
   * which the <tt>start()</tt> method returns the loop begin value,
   * and the <tt>stop()</tt> method returns the non-inclusive end
   * value.
   */
  Slice getLocalBounds(int axis, bool withCommPad=false) const;

  /** \brief Return true if there is any padding stored locally
   *
   * Note that it is not as simple as whether there were communication
   * padding specified in the constructor.  Suppose non-zero
   * communication padding was specified, but the problem is on one
   * processor and no boundary padding was specified: this method will
   * return false.  Similarly, if no communication padding was
   * specified but boundary padding was, then boundary processors will
   * have padding and this method will return true.
   */
  bool hasPadding() const;

  /** \brief Get the size of the lower padding along the given axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * Note that the returned padding can be either communication
   * padding or boundary padding as appropriate.
   */
  int getLowerPadSize(int axis) const;

  /** \brief Get the size of the upper padding along the given axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * Note that the returned padding can be either communication
   * padding or boundary padding as appropriate.
   */
  int getUpperPadSize(int axis) const;

  /** \brief Get the communication padding size along the given axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * This returns the value of the communication padding along the
   * given axis at the time of construction, regardless of the
   * processor's position relative to the domain boundary.
   */
  int getCommPadSize(int axis) const;

  /** \brief Get the size of the lower boundary padding along the
   *         given axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   */
  int getLowerBndryPad(int axis) const;

  /** \brief Get the size of the upper boundary padding along the
   *         given axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   */
  int getUpperBndryPad(int axis) const;

  /** \brief Get the boundary padding size along the given axis
   *
   * \param axis [in] the index of the axis (from zero to the number
   *        of dimensions - 1)
   *
   * This returns the value of the boundary padding along the given
   * axis at the time of construction, regardless of whether a sub-map
   * reduced these values.
   */
  int getBndryPadSize(int axis) const;

  /** \brief Get the storage order
   */
  Layout getLayout() const;

  /** \brief True if there are no stride gaps on any processor
   *
   * An MDVector constructed from a communicator and dimensions will
   * always be contiguous.  An MDVector that is a slice of a parent
   * MDMVector will generally be non-contiguous, with some exceptions.
   * There are cases where some local data is contiguous and some is
   * not, but this method returns True only if all processes' local
   * data is contiguous.
   */
  bool isContiguous() const;

  //@}

  /** \name Conversion to other types of Vectors */
  //@{

#ifdef HAVE_EPETRA

  /** \brief Return this MDVector as an Epetra_Vector
   *
   * The multiple dimensions of the MDVector will be flattened in
   * order to be expressed as an Epetra_Vector.  Currently, if the
   * MDVector is non-contiguous, a Domi::MDMapNoncontiguous exception
   * will be thrown, as Epetra_Vectors are contiguous, and this is
   * intended as a view transform.  Non-contiguous data is the result
   * of slicing a parent MDVector, and getEpetraVector() should be
   * called on the parent instead.
   *
   * The MDVector Scalar template must be the same type as an
   * Epetra_Vector, i.e. double, or a Domi::TypeError exception will
   * be thrown.
   */
  Teuchos::RCP< Epetra_Vector > getEpetraVector() const;

  /** \brief Return this MDVector as an Epetra_MultiVector
   *
   * This method will attempt to utilize either the leading dimension
   * (if the layout is C-order) or the trailing dimension (if the
   * layout is Fortran-order) to serve as the Epetra_MultiVector
   * vector index.  This dimension must be non-distributed (i.e., the
   * corresponding commDim must be 1) in order for this mapping to
   * work.  There must also be no padding associated with this axis.
   * If these requirements are not met, then the entire MDVector will
   * be treated as a single vector, that is the number of vectors will
   * equal 1.
   *
   * Currently, if the MDVector is non-contiguous, a
   * Domi::MDMapNoncontiguous exception will be thrown, as
   * Epetra_MultiVectors are contiguous, and this is intended as a
   * view transform.
   *
   * The MDVector Scalar template must be the same type as an
   * Epetra_MultiVector, i.e. double, or a Domi::TypeError exception
   * will be thrown.
   */
  Teuchos::RCP< Epetra_MultiVector > getEpetraMultiVector() const;

#endif

#ifdef HAVE_TPETRA

  /** \brief Return this MDVector as an Tpetra::Vector
   *
   * The multiple dimensions of the MDVector will be flattened in
   * order to be expressed as an Tpetra::Vector.  Currently, if the
   * MDVector is non-contiguous, a Domi::MDMapNoncontiguous exception
   * will be thrown, as Tpetra::Vectors are contiguous, and this is
   * intended as a view transform.  Non-contiguous data is the result
   * of slicing a parent MDVector, and getTpetraVector() should be
   * called on the parent instead.
   */
  template< class LocalOrdinal >
  Teuchos::RCP< Tpetra::Vector< Scalar, LocalOrdinal, LocalOrdinal, Node > >
  getTpetraVector() const;

  /** \brief Return this MDVector as an Tpetra::Vector
   *
   * The multiple dimensions of the MDVector will be flattened in
   * order to be expressed as an Tpetra::Vector.  Currently, if the
   * MDVector is non-contiguous, a Domi::MDMapNoncontiguous exception
   * will be thrown, as Tpetra::Vectors are contiguous, and this is
   * intended as a view transform.  Non-contiguous data is the result
   * of slicing a parent MDVector, and getTpetraVector() should be
   * called on the parent instead.
   */
  template< class LocalOrdinal,
            class GlobalOrdinal >
  Teuchos::RCP< Tpetra::Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node > >
  getTpetraVector() const;

  /** \brief Return this MDVector as an Tpetra::Vector
   *
   * The multiple dimensions of the MDVector will be flattened in
   * order to be expressed as an Tpetra::Vector.  Currently, if the
   * MDVector is non-contiguous, a Domi::MDMapNoncontiguous exception
   * will be thrown, as Tpetra::Vectors are contiguous, and this is
   * intended as a view transform.  Non-contiguous data is the result
   * of slicing a parent MDVector, and getTpetraVector() should be
   * called on the parent instead.
   */
  template< class LocalOrdinal,
            class GlobalOrdinal,
            class Node2 >
  Teuchos::RCP< Tpetra::Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node2 > >
  getTpetraVector() const;

  /** \brief Return this MDVector as an Tpetra::MultiVector
   *
   * This method will attempt to utilize either the leading dimension
   * (if the layout is C-order) or the trailing dimension (if the
   * layout is Fortran-order) to serve as the Tpetra::MultiVector
   * vector index.  This dimension must be non-distributed (i.e., the
   * corresponding commDim must be 1) in order for this mapping to
   * work.  There must also be no padding associated with this axis.
   * If these requirements are not met, then the entire MDVector will
   * be treated as a single vector, that is the number of vectors will
   * equal 1.
   *
   * Currently, if the MDVector is non-contiguous, a
   * Domi::MDMapNoncontiguous exception will be thrown, as
   * Tpetra::MultiVectors are contiguous, and this is intended as a
   * view transform.
   */
  template < class LocalOrdinal >
  Teuchos::RCP< Tpetra::MultiVector< Scalar,
                                     LocalOrdinal,
                                     LocalOrdinal,
                                     Node > >
  getTpetraMultiVector() const;

  /** \brief Return this MDVector as an Tpetra::MultiVector
   *
   * This method will attempt to utilize either the leading dimension
   * (if the layout is C-order) or the trailing dimension (if the
   * layout is Fortran-order) to serve as the Tpetra::MultiVector
   * vector index.  This dimension must be non-distributed (i.e., the
   * corresponding commDim must be 1) in order for this mapping to
   * work.  There must also be no padding associated with this axis.
   * If these requirements are not met, then the entire MDVector will
   * be treated as a single vector, that is the number of vectors will
   * equal 1.
   *
   * Currently, if the MDVector is non-contiguous, a
   * Domi::MDMapNoncontiguous exception will be thrown, as
   * Tpetra::MultiVectors are contiguous, and this is intended as a
   * view transform.
   */
  template < class LocalOrdinal,
             class GlobalOrdinal >
  Teuchos::RCP< Tpetra::MultiVector< Scalar,
                                     LocalOrdinal,
                                     GlobalOrdinal,
                                     Node > >
  getTpetraMultiVector() const;

  /** \brief Return this MDVector as an Tpetra::MultiVector
   *
   * This method will attempt to utilize either the leading dimension
   * (if the layout is C-order) or the trailing dimension (if the
   * layout is Fortran-order) to serve as the Tpetra::MultiVector
   * vector index.  This dimension must be non-distributed (i.e., the
   * corresponding commDim must be 1) in order for this mapping to
   * work.  There must also be no padding associated with this axis.
   * If these requirements are not met, then the entire MDVector will
   * be treated as a single vector, that is the number of vectors will
   * equal 1.
   *
   * Currently, if the MDVector is non-contiguous, a
   * Domi::MDMapNoncontiguous exception will be thrown, as
   * Tpetra::MultiVectors are contiguous, and this is intended as a
   * view transform.
   */
  template < class LocalOrdinal,
             class GlobalOrdinal,
             class Node2 >
  Teuchos::RCP< Tpetra::MultiVector< Scalar,
                                     LocalOrdinal,
                                     GlobalOrdinal,
                                     Node2 > >
  getTpetraMultiVector() const;

#endif

  //@}

  /** \name Data extraction methods */
  //@{

  /** \brief Get a non-const view of the data as an MDArrayView
   */
  MDArrayView< Scalar > getDataNonConst(bool includePadding = true);

  /** \brief Get a const view of the data as an MDArrayView
   */
  MDArrayView< const Scalar > getData(bool includePadding = true) const;

  /** \brief Get a non-const view of the lower padding data along the
             given axis as an MDArrayView
   */
  MDArrayView< Scalar > getLowerPadDataNonConst(int axis);

  /** \brief Get a const view of the lower padding data along the
             given axis as an MDArrayView
   */
  MDArrayView< const Scalar > getLowerPadData(int axis) const;

  /** \brief Get a non-const view of the upper padding data along the
             given axis as an MDArrayView
   */
  MDArrayView< Scalar > getUpperPadDataNonConst(int axis);

  /** \brief Get a const view of the upper padding data along the
             given axis as an MDArrayView
   */
  MDArrayView< const Scalar > getUpperPadData(int axis) const;

  //@}

  /** \name Mathematical methods */
  //@{

  /** \brief Compute the dot product of this MDVector and MDVector a
   */
  Scalar
  dot(const MDVector< Scalar, Node > & a) const;

  /** \brief Compute the 1-norm of this MDVector
   */
  typename Teuchos::ScalarTraits< Scalar >::magnitudeType norm1() const;

  /** \brief Compute the 2-norm of this MDVector
   */
  typename Teuchos::ScalarTraits< Scalar >::magnitudeType norm2() const;

  /** \brief Compute the infinity-norm of this MDVector
   */
  typename Teuchos::ScalarTraits< Scalar >::magnitudeType normInf() const;

  /** \brief Compute the weighted norm of this 
   */
  typename Teuchos::ScalarTraits< Scalar >::magnitudeType
  normWeighted(const MDVector< Scalar, Node > & weights) const;

  /** \brief Compute the mean (average) value of this MDVector
   */
  Scalar meanValue() const;

  //@}

  /** \name Implementation of the Teuchos::Describable interface */
  //@{

  /** \brief A simple one-line description of this MDVector
   */
  virtual std::string description() const;

  /** \brief Print the object with some verbosity level to a
             FancyOStream
   */
  virtual void
  describe(Teuchos::FancyOStream &out,
           const Teuchos::EVerbosityLevel verbLevel =
             Teuchos::Describable::verbLevel_default) const;

  //@}

  /** \name Global assignment methods */
  //@{

  /// Set all values in the multivector with the given value. 
  void putScalar(const Scalar & value,
                 bool includePadding = true);

  /// Set all values in the multivector to pseudorandom numbers. 
  void randomize();

  //@}

  /** \name Global communication methods */
  //@{

  /// Sum values of a locally replicated multivector across all processes.
  void reduce();

  /// The simplest method for updating the communication padding.
  /// This method will update the communication padding along all
  /// axes.
  void updateCommPad();

  /// Update the data in the communication padding along the specified
  /// axis
  void updateCommPad(int axis);

  /// Send the non-blocking updates for the communication padding
  /// along the given axis
  void startUpdateCommPad(int axis);

  /// Receive non-blocking updates for the communication padding along
  /// the given axis
  void endUpdateCommPad(int axis);

  //@}

  /** \name Sub-MDVector operators */
  //@{

  /** \brief Sub-vector access operator.  The returned
   *  <tt>MDVector</tt> will have one fewer dimensions than the
   *  calling <tt>MDVector</tt>.
   *
   * \param index [in] index of the desired sub-vector.  Note that to
   *        obtain expected behavior, you should always chain together
   *        <tt>n</tt> square bracket operators when referencing an
   *        <tt>n</tt>-dimensional <tt>MDVector</tt>.
   */
  MDVector< Scalar, Node >
  operator[](dim_type index) const;

  /** \brief Sub-vector access operator.  The returned
   *  <tt>MDVector</tt> will have the same number of dimensions as the
   *  calling <tt>MDVector</tt>.
   *
   * \param slice [in] index of the desired sub-vector.  Note that to
   *        obtain expected behavior, you should always chain together
   *        <tt>n</tt> square bracket operators when referencing an
   *        <tt>n</tt>-dimensional <tt>MDVector</tt>.
   */
  MDVector< Scalar, Node >
  operator[](Slice slice) const;

  //@}

private:

  // The Teuchos communicator.  Note that this is always a reference
  // to the communicator of the _mdMap, and is stored only for
  // convenience
  const TeuchosCommRCP _teuchosComm;

  // The MDMap that describes the domain decomposition of this
  // MDVector
  Teuchos::RCP< const MDMap< Node > > _mdMap;

  // The MDArrayRCP that stores the data of this MDVector
  MDArrayRCP< Scalar > _mdArrayRcp;

  // The MDArrayView of the (possibly whole) sub-view into this
  // MDVector's MDArrayRCP
  MDArrayView< Scalar > _mdArrayView;

  // The operator[](int) and operator[](Slice) methods are applied to
  // a specific axis, namely this internally stored and updated next
  // axis
  int _nextAxis;

  // The operator[](Slice) method calls a constructor that can take a
  // boundary pad size, which we store internally here
  Teuchos::Array< int > _sliceBndryPad;

  // Define a struct for storing all the information needed for a
  // single message: a pointer to the buffer, a reference to the
  // message's MPI data type, the rank of the communication partner,
  // and the axis alng which we are communicating
  struct MessageInfo
  {
    // Pointer to first element of data buffer
    void *buffer;
#ifdef HAVE_MPI
    // MPI data type (strided vector)
    Teuchos::RCP< MPI_Datatype > datatype;
#endif
    // Processor rank for communication partner
    int proc;
    // Communication is along this axis
    int axis;
  };

  // An array of MessageInfo objects representing all active send
  // buffers.  The outer array represents the axis and the inner
  // 2-Tuple represents the lower and upper boundaries.
  Teuchos::Array< Teuchos::Tuple< MessageInfo, 2 > > _sendMessages;

  // An array of MessageInfo objects representing all active receive
  // buffers.  The outer array represents the axis and the inner
  // 2-Tuple represents the lower and upper boundaries.
  Teuchos::Array< Teuchos::Tuple< MessageInfo, 2 > > _recvMessages;

#ifdef HAVE_MPI
  // An array of MPI_Request objects for supporting non-blocking sends
  // and receives
  Teuchos::Array< MPI_Request > _requests;
#endif

  // A private method to initialize the _sendMessages and
  // _recvMessages arrays.
  void initializeMessages();
};

/////////////////////////////
// *** Implementations *** //
/////////////////////////////

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const Teuchos::RCP< const MDMap< Node > > & mdMap,
         bool zeroOut) :
  _teuchosComm(mdMap->getTeuchosComm()),
  _mdMap(mdMap),
  _mdArrayRcp(),
  _mdArrayView(),
  _nextAxis(0),
  _sliceBndryPad(mdMap->numDims()),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");

  // Obtain the array of dimensions
  int numDims = _mdMap->numDims();
  Teuchos::Array< dim_type > dims(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    dims[axis] = _mdMap->getLocalDim(axis,true);

  // Resize the MDArrayRCP and set the MDArrayView
  _mdArrayRcp.resize(dims);
  _mdArrayView = _mdArrayRcp();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const Teuchos::RCP< const MDMap< Node > > & mdMap,
         const dim_type leadingDim,
         const dim_type trailingDim,
         bool zeroOut) :
  _teuchosComm(mdMap->getTeuchosComm()),
  _mdMap(mdMap->getAugmentedMDMap(leadingDim, trailingDim)),
  _mdArrayRcp(),
  _mdArrayView(),
  _nextAxis(0),
  _sliceBndryPad(_mdMap->numDims()),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");

  // Obtain the array of dimensions
  int numDims = _mdMap->numDims();
  Teuchos::Array< dim_type > dims(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    dims[axis] = _mdMap->getLocalDim(axis,true);

  // Resize the MDArrayRCP and set the MDArrayView
  _mdArrayRcp.resize(dims);
  _mdArrayView = _mdArrayRcp();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const Teuchos::RCP< const MDMap< Node > > & mdMap,
         const MDArrayView< Scalar > & source) :
  _mdMap(mdMap),
  _mdArrayRcp(source),
  _mdArrayView(_mdArrayRcp()),
  _nextAxis(0),
  _sliceBndryPad(mdMap->numDims()),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");
  int numDims = _mdMap->numDims();
  TEUCHOS_TEST_FOR_EXCEPTION(
    numDims != _mdArrayRcp.numDims(),
    InvalidArgument,
    "MDMap and source array do not have the same number of dimensions");

  for (int axis = 0; axis < numDims; ++axis)
  {
    TEUCHOS_TEST_FOR_EXCEPTION(
      _mdMap->getLocalDim(axis) != _mdArrayRcp.dimension(axis),
      InvalidArgument,
      "Axis " << axis << ": MDMap dimension = " << _mdMap->getLocalDim(axis)
      << ", MDArray dimension = " << _mdArrayRcp.dimension(axis));
  }
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const MDVector< Scalar, Node > & source) :
  _teuchosComm(source.getMDMap()->getTeuchosComm()),
  _mdMap(source.getMDMap()),
  _mdArrayRcp(source._mdArrayRcp),
  _mdArrayView(source._mdArrayView),
  _nextAxis(0),
  _sliceBndryPad(source._sliceBndryPad),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const TeuchosCommRCP teuchosComm,
         Teuchos::ParameterList & plist,
         const Teuchos::RCP< Node > & node) :
  _teuchosComm(teuchosComm),
  _mdMap(),
  _mdArrayRcp(),
  _mdArrayView(),
  _nextAxis(0),
  _sliceBndryPad(),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");
  
  // Compute the MDComm and MDMap
  MDMap< Node > * myMdMap = new MDMap< Node >(teuchosComm, plist, node);
  dim_type leadingDim  = plist.get("leading dimension" , 0);
  dim_type trailingDim = plist.get("trailing dimension", 0);
  if (leadingDim + trailingDim > 0)
  {
    _mdMap = myMdMap->getAugmentedMDMap(leadingDim, trailingDim);
    delete myMdMap;
  }
  else
    _mdMap = Teuchos::rcp(myMdMap);

  // Obtain the array of dimensions
  int numDims = _mdMap->numDims();
  Teuchos::Array< dim_type > dims(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    dims[axis] = _mdMap->getLocalDim(axis,true);

  // Resize the MDArrayRCP and set the MDArrayView
  _mdArrayRcp.resize(dims);
  _mdArrayView = _mdArrayRcp();

  //  Set the slice boundary padding
  _sliceBndryPad.resize(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    _sliceBndryPad[axis] = _mdMap->getBndryPadSize(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const MDCommRCP mdComm,
         Teuchos::ParameterList & plist,
         const Teuchos::RCP< Node > & node) :
  _teuchosComm(mdComm->getTeuchosComm()),
  _mdMap(Teuchos::rcp(new MDMap< Node >(mdComm, plist, node))),
  _mdArrayRcp(),
  _mdArrayView(),
  _nextAxis(0),
  _sliceBndryPad(),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");
  
  // Compute the MDMap
  MDMap< Node > * myMdMap = new MDMap< Node >(mdComm, plist, node);
  dim_type leadingDim  = plist.get("leading dimension" , 0);
  dim_type trailingDim = plist.get("trailing dimension", 0);
  if (leadingDim + trailingDim > 0)
  {
    _mdMap = myMdMap->getAugmentedMDMap(leadingDim, trailingDim);
    delete myMdMap;
  }
  else
    _mdMap = Teuchos::rcp(myMdMap);

  // Obtain the array of dimensions
  int numDims = _mdMap->numDims();
  Teuchos::Array< dim_type > dims(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    dims[axis] = _mdMap->getLocalDim(axis,true);

  // Resize the MDArrayRCP and set the MDArrayView
  _mdArrayRcp.resize(dims);
  _mdArrayView = _mdArrayRcp();

  //  Set the slice boundary padding
  _sliceBndryPad.resize(numDims);
  for (int axis = 0; axis < numDims; ++axis)
    _sliceBndryPad[axis] = _mdMap->getBndryPadSize(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const MDVector< Scalar, Node > & parent,
         int axis,
         dim_type globalIndex) :
  _teuchosComm(parent._teuchosComm),
  _mdMap(),
  _mdArrayRcp(parent._mdArrayRcp),
  _mdArrayView(parent._mdArrayView),
  _nextAxis(0),
  _sliceBndryPad(),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");

  // Obtain the parent MDMap
  Teuchos::RCP< const MDMap< Node > > parentMdMap = parent->getMDMap();

  // Obtain the new, sliced MDMap
  _mdMap = Teuchos::rcp(new MDMap< Node >(*parentMdMap,
                                          axis,
                                          globalIndex));

  // Check that we are on the new sub-communicator
  if (_mdMap->onSubcommunicator())
  {
    // Convert the index from global to local.  We start by
    // determining the starting global index on this processor along
    // the given axis, ignoring the boundary padding.  We then
    // subtract the lower padding, whether it is communication padding
    // or boundary padding.
    dim_type origin = parentMdMap->getGlobalRankBounds(axis,false).start() -
                      parentMdMap->getLowerPadSize(axis);

    // The local index along the given axis is the global axis minus
    // the starting index.  Since we are on the subcommunicator, this
    // should be valid.
    dim_type localIndex = globalIndex - origin;

    // Obtain the new MDArrayView using the local index
    MDArrayView< Scalar > newView(_mdArrayView, axis, localIndex);
    _mdArrayView = newView;

    // Compute the new slice boundary padding
    for (int myAxis = 0; myAxis < parentMdMap->numDims(); ++axis)
      if (myAxis != axis)
        _sliceBndryPad.append(parent._sliceBndryPad[myAxis]);
  }
  else
  {
    // We are not on the sub-communicator, so clear out the data
    // buffer and view
    _mdArrayRcp.clear();
    _mdArrayView = MDArrayView< Scalar >();
  }
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
MDVector(const MDVector< Scalar, Node > & parent,
         int axis,
         const Slice & slice,
         int bndryPad) :
  _teuchosComm(parent._teuchosComm),
  _mdMap(),
  _mdArrayRcp(parent._mdArrayRcp),
  _mdArrayView(parent._mdArrayView),
  _nextAxis(0),
  _sliceBndryPad(parent._sliceBndryPad),
  _sendMessages(),
  _recvMessages(),
  _requests()
{
  setObjectLabel("Domi::MDVector");

  // Obtain the parent MDMap
  Teuchos::RCP< const MDMap< Node > > parentMdMap = parent->getMDMap();

  // Obtain the new, sliced MDMap
  _mdMap = Teuchos::rcp(new MDMap< Node >(*parentMdMap,
                                          axis,
                                          slice,
                                          bndryPad));

  // Check that we are on the new sub-communicator
  if (_mdMap->onSubcommunicator())
  {
    // Convert the given Slice start index from global to local.  We
    // start by determining the starting global index on this
    // processor along the given axis, ignoring the boundary padding.
    // We then subtract the lower padding, whether it is communication
    // padding or boundary padding.
    dim_type origin = parentMdMap->getGlobalRankBounds(axis,false).start() -
                      parentMdMap->getLowerPadSize(axis);

    // Determine the starting index of our local slice.  This will be
    // the start of the given slice minus the starting global index on
    // this processor minus the given boundary pad.  If this is less
    // than zero, then the start is on a lower processor, so set the
    // local start to zero.
    dim_type start = std::max(0, slice.start() - origin - bndryPad);

    // Now get the stop index of the local slice.  This will be the
    // stop of the given slice minus the starting global index on this
    // processor plus the given boundary pad.  If this is larger than
    // the local dimension, then set the local stop to the local
    // dimension.
    dim_type stop = std::min(slice.stop() - origin + bndryPad,
                             parentMdMap->getLocalDim(axis,true));

    // Obtain the new MDArrayView using the local slice
    MDArrayView< Scalar > newView(_mdArrayView, axis, Slice(start,stop));
    _mdArrayView = newView;
  }
  else
  {
    // We are not on the sub-communicator, so clear out the data
    // buffer and view
    _mdArrayRcp.clear();
    _mdArrayView = MDArrayView< Scalar >();
  }
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >::
~MDVector()
{
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
const Teuchos::RCP< const MDMap< Node > >
MDVector< Scalar, Node >::
getMDMap() const
{
  return _mdMap;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
bool
MDVector< Scalar, Node >::
onSubcommunicator() const
{
  return _mdMap->onSubcommunicator();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
TeuchosCommRCP 
MDVector< Scalar, Node >::
getTeuchosComm() const
{
  return _mdMap->getTeuchosComm();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
numDims() const
{
  return _mdMap->numDims();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getCommDim(int axis) const
{
  return _mdMap->getCommDim(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
bool
MDVector< Scalar, Node >::
isPeriodic(int axis) const
{
  return _mdMap->isPeriodic(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getCommIndex(int axis) const
{
  return _mdMap->getCommIndex(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getLowerNeighbor(int axis) const
{
  return _mdMap->getLowerNeighbor(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getUpperNeighbor(int axis) const
{
  return _mdMap->getUpperNeighbor(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
dim_type
MDVector< Scalar, Node >::
getGlobalDim(int axis, bool withBndryPad) const
{
  return _mdMap->getGlobalDim(axis, withBndryPad);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
Slice
MDVector< Scalar, Node >::
getGlobalBounds(int axis, bool withBndryPad) const
{
  return _mdMap->getGlobalBounds(axis, withBndryPad);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
Slice
MDVector< Scalar, Node >::
getGlobalRankBounds(int axis, bool withBndryPad) const
{
  return _mdMap->getGlobalRankBounds(axis, withBndryPad);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
dim_type
MDVector< Scalar, Node >::
getLocalDim(int axis, bool withCommPad) const
{
  return _mdMap->getLocalDim(axis, withCommPad);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
Slice
MDVector< Scalar, Node >::
getLocalBounds(int axis, bool withCommPad) const
{
  return _mdMap->getLocalBounds(axis, withCommPad);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
bool
MDVector< Scalar, Node >::
hasPadding() const
{
  return _mdMap->hasPadding();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getLowerPadSize(int axis) const
{
  return _mdMap->getLowerPadSize(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getUpperPadSize(int axis) const
{
  return _mdMap->getUpperPadSize(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getCommPadSize(int axis) const
{
  return _mdMap->getCommPadSize(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getLowerBndryPad(int axis) const
{
  return _mdMap->getLowerBndryPad(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getUpperBndryPad(int axis) const
{
  return _mdMap->getUpperBndryPad(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
int
MDVector< Scalar, Node >::
getBndryPadSize(int axis) const
{
  return _mdMap->getBndryPadSize(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
Layout
MDVector< Scalar, Node >::
getLayout() const
{
  return _mdMap->getLayout();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
bool
MDVector< Scalar, Node >::
isContiguous() const
{
  return _mdMap->isContiguous();
}

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_EPETRA

// The getEpetraVector() method only makes sense for Scalar=double,
// because Epetra_Vectors store data buffers of type double only.
// There is no convenient way to specialize just one (or a small
// handfull of) methods, instead you have to specialize the whole
// class.  So we allow getEpetraVector() to compile for any Scalar,
// but we will throw an exception if Scalar is not double.

template< class Scalar,
          class Node >
Teuchos::RCP< Epetra_Vector >
MDVector< Scalar, Node >::
getEpetraVector() const
{
  // Throw an exception if Scalar is not double
  const char * scalarType = typeid(Scalar).name();
  TEUCHOS_TEST_FOR_EXCEPTION(
    strncmp(scalarType, "double", 6) != 0,
    TypeError,
    "MDVector is of scalar type '" << scalarType << "', but Epetra_Vector "
    "requires scalar type 'double'");

  // Throw an exception if this MDVector's MDMap is not contiguous
  TEUCHOS_TEST_FOR_EXCEPTION(
    !isContiguous(),
    MDMapNoncontiguousError,
    "This MDVector's MDMap is non-contiguous.  This can happen when you take "
    "a slice of a parent MDVector.");

  // Get the Epetra_Map that is equivalent to this MDVector's MDMap
  Teuchos::RCP< const Epetra_Map > epetraMap = _mdMap->getEpetraMap(true);

  // Return the result.  We are changing a Sca* to a double*, which
  // looks sketchy, but we have thrown an exception if Sca is not
  // double, so everything is kosher.
  void * buffer = (void*) _mdArrayView.getRawPtr();
  return Teuchos::rcp(new Epetra_Vector(View,
                                        *epetraMap,
                                        (double*) buffer));
}

#endif

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_EPETRA

// The getEpetraMultiVector() method only makes sense for
// Scalar=double, because Epetra_MultiVectors store data buffers of
// type double only.  There is no convenient way to specialize just
// one (or a small handfull of) methods, instead you have to
// specialize the whole class.  So we allow getEpetraVector() to
// compile for any Scalar, but we will throw an exception if Scalar is
// not double.

template< class Scalar,
          class Node >
Teuchos::RCP< Epetra_MultiVector >
MDVector< Scalar, Node >::
getEpetraMultiVector() const
{
  // Throw an exception if Scalar is not double
  const char * scalarType = typeid(Scalar).name();
  TEUCHOS_TEST_FOR_EXCEPTION(
    strncmp(scalarType, "double", 6) != 0,
    TypeError,
    "MDVector is of scalar type '" << scalarType << "', but Epetra_Vector "
    "requires scalar type 'double'");

  // Determine the vector axis and related info
  int vectorAxis = (getLayout() == C_ORDER) ? 0 : numDims()-1;
  int padding    = getLowerPadSize(vectorAxis) + getUpperPadSize(vectorAxis);
  int commDim    = getCommDim(vectorAxis);
  int numVectors = getGlobalDim(vectorAxis);

  // Obtain the appropriate MDMap and check that it is contiguous
  Teuchos::RCP< const MDMap< Node > > newMdMap;
  if (padding == 0 && commDim == 1)
    newMdMap = Teuchos::rcp(new MDMap<Node>(*_mdMap, vectorAxis, 0));
  else
  {
    newMdMap = _mdMap;
    numVectors = 1;
  }
  TEUCHOS_TEST_FOR_EXCEPTION(
    ! newMdMap->isContiguous(),
    MDMapNoncontiguousError,
    "This MDVector's MDMap is non-contiguous.  This can happen when you take "
    "a slice of a parent MDVector.");

  // Get the stride between vectors.  The MDMap strides are private,
  // but we know the new MDMap is contiguous, so we can calculate it
  // as the product of the new MDMap dimensions (including padding)
  size_type stride = newMdMap->getLocalDim(0,true);
  for (int axis = 0; axis < newMdMap->numDims(); ++axis)
    stride *= newMdMap->getLocalDim(axis,true);
  TEUCHOS_TEST_FOR_EXCEPTION(
    stride*numVectors > Teuchos::OrdinalTraits<int>::max(),
    MapOrdinalError,
    "Buffer size " << stride*numVectors << " is too large for Epetra int "
    "ordinals");
  int lda = (int)stride;

  // Get the Epetra_Map that is equivalent to newMdMap
  Teuchos::RCP< const Epetra_Map > epetraMap = newMdMap->getEpetraMap(true);

  // Return the result.  We are changing a Sca* to a double*, which
  // looks sketchy, but we have thrown an exception if Sca is not
  // double, so everything is kosher.
  void * buffer = (void*) _mdArrayView.getRawPtr();
  return Teuchos::rcp(new Epetra_MultiVector(View,
                                             *epetraMap,
                                             (double*) buffer,
                                             lda,
                                             numVectors));
}

#endif

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_TPETRA

template< class Scalar, class Node >
template< class LocalOrdinal >
Teuchos::RCP< Tpetra::Vector< Scalar, LocalOrdinal, LocalOrdinal, Node > >
MDVector< Scalar, Node >::
getTpetraVector() const
{
  return getTpetraVector< LocalOrdinal, LocalOrdinal, Node >();
}

#endif

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_TPETRA

template< class Scalar, class Node >
template< class LocalOrdinal,
          class GlobalOrdinal >
Teuchos::RCP< Tpetra::Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node > >
MDVector< Scalar, Node >::
getTpetraVector() const
{
  return getTpetraVector< LocalOrdinal, GlobalOrdinal, Node >();
}

#endif

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_TPETRA

template< class Scalar, class Node >
template< class LocalOrdinal,
          class GlobalOrdinal,
          class Node2 >
Teuchos::RCP< Tpetra::Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node2 > >
MDVector< Scalar, Node >::
getTpetraVector() const
{
  // Throw an exception if this MDVector's MDMap is not contiguous
  TEUCHOS_TEST_FOR_EXCEPTION(
    !isContiguous(),
    MDMapNoncontiguousError,
    "This MDVector's MDMap is non-contiguous.  This can happen when you take "
    "a slice of a parent MDVector.");

  // Get the Tpetra::Map that is equivalent to this MDVector's MDMap
  Teuchos::RCP< const Tpetra::Map< LocalOrdinal,
                                   GlobalOrdinal,
                                   Node2 > > tpetraMap =
    _mdMap->template getTpetraMap< LocalOrdinal, GlobalOrdinal, Node2 >(true);

  // Return the result
  return Teuchos::rcp(new Tpetra::Vector< Scalar,
                                          LocalOrdinal,
                                          GlobalOrdinal,
                                          Node2 >(tpetraMap,
                                                  _mdArrayView.arrayView()));
}

#endif

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_TPETRA

template< class Scalar, class Node >
template < class LocalOrdinal >
Teuchos::RCP< Tpetra::MultiVector< Scalar,
                                   LocalOrdinal,
                                   LocalOrdinal,
                                   Node > >
MDVector< Scalar, Node >::
getTpetraMultiVector() const
{
  return getTpetraMultiVector< LocalOrdinal, LocalOrdinal, Node >();
}

#endif

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_TPETRA

template< class Scalar, class Node >
template < class LocalOrdinal,
           class GlobalOrdinal >
Teuchos::RCP< Tpetra::MultiVector< Scalar,
                                   LocalOrdinal,
                                   GlobalOrdinal,
                                   Node > >
MDVector< Scalar, Node >::
getTpetraMultiVector() const
{
  return getTpetraMultiVector< LocalOrdinal, GlobalOrdinal, Node >();
}

#endif

////////////////////////////////////////////////////////////////////////

#ifdef HAVE_TPETRA

template< class Scalar, class Node >
template < class LocalOrdinal,
           class GlobalOrdinal,
           class Node2 >
Teuchos::RCP< Tpetra::MultiVector< Scalar,
                                   LocalOrdinal,
                                   GlobalOrdinal,
                                   Node2 > >
MDVector< Scalar, Node >::
getTpetraMultiVector() const
{
  // Determine the vector axis and related info
  int vectorAxis = (getLayout() == C_ORDER) ? 0 : numDims()-1;
  int padding    = getLowerPadSize(vectorAxis) + getUpperPadSize(vectorAxis);
  int commDim    = getCommDim(vectorAxis);
  int numVectors = getGlobalDim(vectorAxis);

  // Obtain the appropriate MDMap and check that it is contiguous
  Teuchos::RCP< const MDMap< Node > > newMdMap;
  if (padding == 0 && commDim == 1)
    newMdMap = Teuchos::rcp(new MDMap< Node >(*_mdMap, vectorAxis, 0));
  else
  {
    newMdMap = _mdMap;
    numVectors = 1;
  }
  TEUCHOS_TEST_FOR_EXCEPTION(
    ! newMdMap->isContiguous(),
    MDMapNoncontiguousError,
    "This MDVector's MDMap is non-contiguous.  This can happen when you take "
    "a slice of a parent MDVector.");

  // Get the stride between vectors.  The MDMap strides are private,
  // but we know the new MDMap is contiguous, so we can calculate it
  // as the product of the new MDMap dimensions (including padding)
  size_type stride = newMdMap->getLocalDim(0,true);
  for (int axis = 0; axis < newMdMap->numDims(); ++axis)
    stride *= newMdMap->getLocalDim(axis,true);
  TEUCHOS_TEST_FOR_EXCEPTION(
    stride*numVectors > Teuchos::OrdinalTraits<GlobalOrdinal>::max(),
    MapOrdinalError,
    "Buffer size " << stride*numVectors << " is too large for Tpetra "
    "GlobalOrdinal = " << typeid(GlobalOrdinal).name() );
  size_t lda = (size_t)stride;

  // Get the Tpetra::Map that is equivalent to newMdMap
  Teuchos::RCP< const Tpetra::Map< LocalOrdinal,
                                   GlobalOrdinal,
                                   Node2> > tpetraMap =
    newMdMap->template getTpetraMap< LocalOrdinal, GlobalOrdinal, Node2 >(true);

  // Return the result
  return Teuchos::rcp(new Tpetra::MultiVector< Scalar,
                                               LocalOrdinal,
                                               GlobalOrdinal,
                                               Node2 >(tpetraMap,
                                                       _mdArrayRcp.arrayRCP(),
                                                       lda,
                                                       numVectors));
}

#endif

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDArrayView< Scalar >
MDVector< Scalar, Node >::
getDataNonConst(bool includePadding)
{
  if (includePadding)
    return _mdArrayView;
  MDArrayView< Scalar > newArray(_mdArrayView);
  for (int axis = 0; axis < numDims(); ++axis)
  {
    int lo = getLowerPadSize(axis);
    int hi = getLocalDim(axis,true) - getUpperPadSize(axis);
    newArray = MDArrayView< Scalar >(newArray, axis, Slice(lo,hi));
  }
  return newArray;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDArrayView< const Scalar >
MDVector< Scalar, Node >::
getData(bool includePadding) const
{
  if (includePadding)
    return _mdArrayView.getConst();
  MDArrayView< Scalar > newArray(_mdArrayView);
  for (int axis = 0; axis < numDims(); ++axis)
  {
    int lo = getLowerPadSize(axis);
    int hi = getLocalDim(axis,true) - getUpperPadSize(axis);
    newArray = MDArrayView< Scalar >(newArray, axis, Slice(lo,hi));
  }
  return newArray.getConst();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDArrayView< Scalar >
MDVector< Scalar, Node >::
getLowerPadDataNonConst(int axis)
{
  MDArrayView< Scalar > newArrayView(_mdArrayView,
                                     axis,
                                     Slice(getLowerPadSize(axis)));
  return newArrayView;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDArrayView< const Scalar >
MDVector< Scalar, Node >::
getLowerPadData(int axis) const
{
  MDArrayView< const Scalar > newArrayView(_mdArrayView.getConst(),
                                           axis,
                                           Slice(getLowerPadSize(axis)));
  return newArrayView;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDArrayView< Scalar >
MDVector< Scalar, Node >::
getUpperPadDataNonConst(int axis)
{
  dim_type n   = getLocalDim(axis,true);
  int     pad = getUpperPadSize(axis);
  Slice   slice;
  if (pad) slice = Slice(n-pad,n);
  else     slice = Slice(n-1,n-1);
  MDArrayView< Scalar > newArrayView(_mdArrayView,
                                     axis,
                                     slice);
  return newArrayView;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDArrayView< const Scalar >
MDVector< Scalar, Node >::
getUpperPadData(int axis) const
{
  dim_type n   = getLocalDim(axis,true);
  int     pad = getUpperPadSize(axis);
  Slice   slice;
  if (pad) slice = Slice(n-pad,n);
  else     slice = Slice(n-1,n-1);
  MDArrayView< const Scalar > newArrayView(_mdArrayView.getConst(),
                                           axis,
                                           slice);
  return newArrayView;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
Scalar
MDVector< Scalar, Node >::
dot(const MDVector< Scalar, Node > & a) const
{
  typedef typename MDArrayView< const Scalar >::iterator iterator;

  TEUCHOS_TEST_FOR_EXCEPTION(
    ! _mdMap->isCompatible(*(a._mdMap)),
    MDMapError,
    "MDMap of calling MDVector and argument 'a' are incompatible");

  MDArrayView< const Scalar > aView = a.getData();
  Scalar local_dot = 0;
  iterator a_it = aView.begin();
  for (iterator it = _mdArrayView.begin(); it != _mdArrayView.end();
       ++it, ++a_it)
    local_dot += *it * *a_it;
  Scalar global_dot = 0;
  Teuchos::reduceAll(*_teuchosComm,
                     Teuchos::REDUCE_SUM,
                     1,
                     &local_dot,
                     &global_dot);
  return global_dot;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
typename Teuchos::ScalarTraits< Scalar >::magnitudeType
MDVector< Scalar, Node >::
norm1() const
{
  typedef typename Teuchos::ScalarTraits< Scalar >::magnitudeType mag;
  mag local_norm1 = 0;
  for (typename MDArrayView< const Scalar >::iterator it = _mdArrayView.begin();
       it != _mdArrayView.end(); ++it)
    local_norm1 += std::abs(*it);
  mag global_norm1 = 0;
  Teuchos::reduceAll(*_teuchosComm,
                     Teuchos::REDUCE_SUM,
                     1,
                     &local_norm1,
                     &global_norm1);
  return global_norm1;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
typename Teuchos::ScalarTraits< Scalar >::magnitudeType
MDVector< Scalar, Node >::
norm2() const
{
  typedef typename Teuchos::ScalarTraits< Scalar >::magnitudeType mag;
  mag norm2 = dot(*this);
  return Teuchos::ScalarTraits<mag>::squareroot(norm2);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
typename Teuchos::ScalarTraits< Scalar >::magnitudeType
MDVector< Scalar, Node >::
normInf() const
{
  typedef typename Teuchos::ScalarTraits< Scalar >::magnitudeType mag;
  mag local_normInf = 0;
  for (typename MDArrayView< const Scalar >::iterator it = _mdArrayView.begin();
       it != _mdArrayView.end(); ++it)
    local_normInf = std::max(local_normInf, std::abs(*it));
  mag global_normInf = 0;
  Teuchos::reduceAll(*_teuchosComm,
                     Teuchos::REDUCE_MAX,
                     1,
                     &local_normInf,
                     &global_normInf);
  return global_normInf;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
typename Teuchos::ScalarTraits< Scalar >::magnitudeType
MDVector< Scalar, Node >::
normWeighted(const MDVector< Scalar, Node > & weights) const
{
  typedef typename Teuchos::ScalarTraits< Scalar >::magnitudeType mag;

  TEUCHOS_TEST_FOR_EXCEPTION(
    ! _mdMap->isCompatible(*(weights._mdMap)),
    MDMapError,
    "MDMap of calling MDVector and argument 'weights' are incompatible");

  MDArrayView< const Scalar > wView = weights.getData();
  mag local_wNorm = 0;
  typename MDArrayView< const Scalar >::iterator w_it = wView.begin();
  for (typename MDArrayView< const Scalar >::iterator it = _mdArrayView.begin();
       it != _mdArrayView.end(); ++it, ++w_it)
    local_wNorm += *it * *it * *w_it;
  mag global_wNorm = 0;
  Teuchos::reduceAll(*_teuchosComm,
                     Teuchos::REDUCE_SUM,
                     1,
                     &local_wNorm,
                     &global_wNorm);
  Teuchos::Array< dim_type > dimensions(numDims());
  for (int i = 0; i < numDims(); ++i)
    dimensions[i] = _mdMap->getGlobalDim(i);
  size_type n = computeSize(dimensions);
  if (n == 0) return 0;
  return Teuchos::ScalarTraits<mag>::squareroot(global_wNorm / n);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
Scalar
MDVector< Scalar, Node >::
meanValue() const
{
  typedef typename Teuchos::ScalarTraits< Scalar >::magnitudeType mag;
  mag local_sum = 0;
  for (typename MDArrayView< const Scalar >::iterator it = _mdArrayView.begin();
       it != _mdArrayView.end(); ++it)
    local_sum += *it;
  mag global_sum = 0;
  Teuchos::reduceAll(*_teuchosComm,
                     Teuchos::REDUCE_SUM,
                     1,
                     &local_sum,
                     &global_sum);
  Teuchos::Array< dim_type > dimensions(numDims());
  for (int i = 0; i < numDims(); ++i)
    dimensions[i] = _mdMap->getGlobalDim(i);
  size_type n = computeSize(dimensions);
  if (n == 0) return 0;
  return global_sum / n;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
std::string
MDVector< Scalar, Node >::
description() const
{
  using Teuchos::TypeNameTraits;

  Teuchos::Array< dim_type > dims(numDims());
  for (int axis = 0; axis < numDims(); ++axis)
    dims[axis] = getGlobalDim(axis, true);

  std::ostringstream oss;
  oss << "\"Domi::MDVector\": {"
      << "Template parameters: {"
      << "Scalar: " << TypeNameTraits<Scalar>::name()
      << ", Node: " << TypeNameTraits<Node>::name()
      << "}";
  if (this->getObjectLabel() != "")
    oss << ", Label: \"" << this->getObjectLabel () << "\", ";
  oss << "Global dimensions: " << dims << " }";
  return oss.str();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
describe(Teuchos::FancyOStream &out,
         const Teuchos::EVerbosityLevel verbLevel) const
{
  using std::endl;
  using std::setw;
  using Teuchos::Comm;
  using Teuchos::RCP;
  using Teuchos::TypeNameTraits;
  using Teuchos::VERB_DEFAULT;
  using Teuchos::VERB_NONE;
  using Teuchos::VERB_LOW;
  using Teuchos::VERB_MEDIUM;
  using Teuchos::VERB_HIGH;
  using Teuchos::VERB_EXTREME;

  const Teuchos::EVerbosityLevel vl =
    (verbLevel == VERB_DEFAULT) ? VERB_LOW : verbLevel;

  const MDMap< Node > & mdMap = *(getMDMap());
  TeuchosCommRCP comm = mdMap.getTeuchosComm();
  const int myImageID = comm->getRank();
  const int numImages = comm->getSize();
  Teuchos::OSTab tab0(out);

  if (vl != VERB_NONE)
  {
    if (myImageID == 0)
    {
      out << "\"Domi::MDVector\":" << endl;
    }
    Teuchos::OSTab tab1(out);// applies to all processes
    if (myImageID == 0)
    {
      out << "Template parameters:";
      {
        Teuchos::OSTab tab2(out);
        out << "Scalar: " << TypeNameTraits<Scalar>::name() << endl
            << "Node: " << TypeNameTraits<Node>::name() << endl;
      }
      out << endl;
      if (this->getObjectLabel() != "")
      {
        out << "Label: \"" << getObjectLabel() << "\"" << endl;
      }
      Teuchos::Array< dim_type > globalDims(numDims());
      for (int axis = 0; axis < numDims(); ++axis)
        globalDims[axis] = getGlobalDim(axis, true);
      out << "Global dimensions: " << globalDims << endl;
    }
    for (int imageCtr = 0; imageCtr < numImages; ++imageCtr)
    {
      if (myImageID == imageCtr)
      {
        if (vl != VERB_LOW)
        {
          // VERB_MEDIUM and higher prints getLocalLength()
          out << "Process: " << myImageID << endl;
          Teuchos::OSTab tab2(out);
          Teuchos::Array< dim_type > localDims(numDims());
          for (int axis = 0; axis < numDims(); ++axis)
            localDims[axis] = getLocalDim(axis, true);
          out << "Local dimensions: " << localDims << endl;

          // if (vl == VERB_EXTREME && this->getLocalLength() > 0)
          // {
          //   // VERB_EXTREME prints values
          //   out << "Global indices and values:" << endl;
          //   Teuchos::OSTab tab3 (out);
          //   RCP<Node> node = this->lclMV_.getNode();
          //   ArrayRCP<const Scalar> myview =
          //     node->template viewBuffer<Scalar> (this->getLocalLength(),
          //                                        MVT::getValues (this->lclMV_));
          //   for (size_t i = 0; i < this->getLocalLength(); ++i)
          //   {
          //     out << map.getGlobalElement(i) << ": " << myview[i] << endl;
          //   }
          // }
        }
        std::flush(out); // give output time to complete
      }
      comm->barrier(); // give output time to complete
      comm->barrier();
      comm->barrier();
    }
  }
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
putScalar(const Scalar & value,
          bool includePadding)
{
  MDArrayView< Scalar > newArray = getDataNonConst(includePadding);
  for (typename MDArrayView< Scalar >::iterator it = newArray.begin();
       it != newArray.end(); ++it)
    *it = value;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
randomize()
{
  typedef typename MDArrayView< Scalar >::iterator iterator;
  Teuchos::ScalarTraits< Scalar >::seedrandom(time(NULL));
  for (iterator it = _mdArrayView.begin(); it != _mdArrayView.end(); ++it)
    *it = Teuchos::ScalarTraits< Scalar >::random();
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
updateCommPad()
{
  for (int axis = 0; axis < numDims(); ++axis)
  {
    updateCommPad(axis);
  }
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
updateCommPad(int axis)
{
  startUpdateCommPad(axis);
  endUpdateCommPad(axis);
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
startUpdateCommPad(int axis)
{
  // #define DOMI_MDVECTOR_OUTPUT_UPDATECOMMPAD

  // Initialize the _sendMessages and _recvMessages members on the
  // first call to startUpdateCommPad(int).
  if (_sendMessages.empty())
    initializeMessages();

  int rank    = _teuchosComm->getRank();
  int numProc = _teuchosComm->getSize();
  int tag;
#ifdef HAVE_MPI
  // Since HAVE_MPI is defined, we know that _teuchosComm points to a
  // const Teuchos::MpiComm< int >.  We downcast, extract and
  // dereference so that we can get access to the MPI_Comm used to
  // construct it.
  Teuchos::RCP< const Teuchos::MpiComm< int > > mpiComm =
    Teuchos::rcp_dynamic_cast< const Teuchos::MpiComm< int > >(_teuchosComm);
  const Teuchos::OpaqueWrapper< MPI_Comm > & communicator =
    *(mpiComm->getRawMpiComm());

  // Post the non-blocking sends
  MPI_Request request;
  for (int boundary = 0; boundary < 2; ++boundary)
  {
    MessageInfo message = _sendMessages[axis][boundary];
    if (message.proc >= 0)
    {
      tag = rank * numProc + message.proc;

#ifdef DOMI_MDVECTOR_OUTPUT_UPDATECOMMPAD
      cout << rank << ": post send for axis " << axis << ", boundary "
           << boundary << ", buffer = " << message.buffer << ", proc = "
           << message.proc << ", tag = " << tag << endl;
#endif

      if (MPI_Isend(message.buffer,
                    1,
                    *(message.datatype),
                    message.proc,
                    tag,
                    communicator(),
                    &request))
        throw std::runtime_error("Domi::MDVector: Error in MPI_Isend");
      _requests.push_back(request);
    }
  }

  // Post the non-blocking receives
  for (int boundary = 0; boundary < 2; ++boundary)
  {
    MessageInfo message = _recvMessages[axis][boundary];
    if (message.proc >= 0)
    {
      tag = message.proc * numProc + rank;

#ifdef DOMI_MDVECTOR_OUTPUT_UPDATECOMMPAD
      cout << rank << ": post recv for axis " << axis << ", boundary "
           << boundary << ", buffer = " << message.buffer << ", proc = "
           << message.proc << ", tag = " << tag << endl;
#endif

      if (MPI_Irecv(message.buffer,
                    1,
                    *(message.datatype),
                    message.proc,
                    tag,
                    communicator(),
                    &request))
        throw std::runtime_error("Domi::MDVector: Error in MPI_Irecv");
      _requests.push_back(request);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
endUpdateCommPad(int axis)
{
#ifdef HAVE_MPI
  if (_requests.size() > 0)
  {
    Teuchos::Array< MPI_Status > status(_requests.size());
    if (MPI_Waitall(_requests.size(),
                    &(_requests[0]),
                    &(status[0]) ) )
      throw std::runtime_error("Domi::MDVector: Error in MPI_Waitall");
    _requests.clear();
  }
#endif
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >
MDVector< Scalar, Node >::
operator[](dim_type index) const
{
  MDVector< Scalar, Node > result(*this,
                                  _nextAxis,
                                  index);
  int newAxis = _nextAxis + 1;
  if (newAxis >= result.numDims()) newAxis = 0;
  result._nextAxis = newAxis;
  return result;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
MDVector< Scalar, Node >
MDVector< Scalar, Node >::
operator[](Slice slice) const
{
  MDVector< Scalar, Node > result(*this,
                                  _nextAxis,
                                  slice,
                                  _sliceBndryPad[_nextAxis]);
  int newAxis = _nextAxis + 1;
  if (newAxis >= result.numDims()) newAxis = 0;
  result._nextAxis = newAxis;
  return result;
}

////////////////////////////////////////////////////////////////////////

template< class Scalar,
          class Node >
void
MDVector< Scalar, Node >::
initializeMessages()
{
  // #define DOMI_MDVECTOR_OUTPUT_INITIALIZE

  int ndims = numDims();
  Teuchos::Array<int> sizes(ndims);
  Teuchos::Array<int> subsizes(ndims);
  Teuchos::Array<int> starts(ndims);
  MessageInfo messageInfo;

  _sendMessages.resize(ndims);
  _recvMessages.resize(ndims);

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
  std::stringstream msg;
  int rank = _teuchosComm->getRank();
#endif

#ifdef HAVE_MPI
  int order = mpiOrder(getLayout());
  MPI_Datatype datatype = mpiType< Scalar >();
#endif

  // Loop over the axes we are going to send messages along
  for (int msgAxis = 0; msgAxis < ndims; ++msgAxis)
  {
    // Set the initial values for sizes, subsizes and starts
    for (int axis = 0; axis < ndims; ++axis)
    {
      sizes[axis]    = _mdArrayRcp.dimension(axis);
      subsizes[axis] = _mdArrayView.dimension(axis);
      starts[axis]   = 0;
    }

    // Set the lower receive and send messages
    int proc = getLowerNeighbor(msgAxis);

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
    msg << endl << "P" << rank << ": axis " << msgAxis << ", lower neighbor = "
        << proc << endl;
#endif

    // Fix the subsize along this message axis
    subsizes[msgAxis] = getLowerPadSize(msgAxis);
    // Fix the proc if the subsize is zero
    if (subsizes[msgAxis] == 0) proc = -1;
    // Assign the non-MPI members of messageInfo
    messageInfo.buffer = (void*) getData().getRawPtr();
    messageInfo.proc   = proc;
    messageInfo.axis   = msgAxis;

    if (proc >= 0)
    {
      // Lower receive message

#ifdef HAVE_MPI
      {

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
        msg << endl << "P" << rank << ": axis " << msgAxis
            << ", Lower receive message:" << endl << "  ndims    = " << ndims
            << endl << "  sizes    = " << sizes << endl << "  subsizes = "
            << subsizes << endl << "  starts   = " << starts << endl
            << "  order    = " << order << endl;
#endif

        Teuchos::RCP< MPI_Datatype > commPad = Teuchos::rcp(new MPI_Datatype);
        MPI_Type_create_subarray(ndims,
                                 &sizes[0],
                                 &subsizes[0],
                                 &starts[0],
                                 order,
                                 datatype,
                                 commPad.get());
        MPI_Type_commit(commPad.get());
        messageInfo.datatype = commPad;
      }
#endif

    }
    _recvMessages[msgAxis][0] = messageInfo;

    // Lower send message
    starts[msgAxis] = getLowerPadSize(msgAxis);
    if (proc >= 0)
    {

#ifdef HAVE_MPI
      {

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
        msg << endl << "P" << rank << ": axis " << msgAxis
            << ", Lower send message:" << endl << "  ndims    = " << ndims
            << endl << "  sizes    = " << sizes << endl << "  subsizes = "
            << subsizes << endl << "  starts   = " << starts << endl
            << "  order    = " << order << endl;
#endif

        Teuchos::RCP< MPI_Datatype > commPad = Teuchos::rcp(new MPI_Datatype);
        MPI_Type_create_subarray(ndims,
                                 &sizes[0],
                                 &subsizes[0],
                                 &starts[0],
                                 order,
                                 datatype,
                                 commPad.get());
        MPI_Type_commit(commPad.get());
        messageInfo.datatype = commPad;
      }
#endif

    }
    _sendMessages[msgAxis][0] = messageInfo;

    // Set the upper receive and send messages
    proc = getUpperNeighbor(msgAxis);

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
    msg << endl << "P" << rank << ": axis " << msgAxis << ", upper neighbor = "
        << proc << endl;
#endif

    subsizes[msgAxis] = getUpperPadSize(msgAxis);
    starts[msgAxis]  = _mdArrayView.dimension(msgAxis) -
                       getUpperPadSize(msgAxis);
    if (subsizes[msgAxis] == 0) proc = -1;
    messageInfo.proc = proc;
    if (proc >= 0)
    {
      // Upper receive message

#ifdef HAVE_MPI
      {

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
        msg << endl << "P" << rank << ": axis " << msgAxis
            << ", Upper receive message:" << endl << "  ndims    = " << ndims
            << endl << "  sizes    = " << sizes << endl << "  subsizes = "
            << subsizes << endl << "  starts   = " << starts << endl
            << "  order    = " << order << endl;
#endif

        Teuchos::RCP< MPI_Datatype > commPad = Teuchos::rcp(new MPI_Datatype);
        MPI_Type_create_subarray(ndims,
                                 &sizes[0],
                                 &subsizes[0],
                                 &starts[0],
                                 order,
                                 datatype,
                                 commPad.get());
        MPI_Type_commit(commPad.get());
        messageInfo.datatype = commPad;
      }
#endif

    }
    _recvMessages[msgAxis][1] = messageInfo;

    // Upper send message
    starts[msgAxis] -= getUpperPadSize(msgAxis);
    if (proc >= 0)
    {

#ifdef HAVE_MPI
      {

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
        msg << endl << "P" << rank << ": axis " << msgAxis
            << ", Upper send message:" << endl << "  ndims    = " << ndims
            << endl << "  sizes    = " << sizes << endl << "  subsizes = "
            << subsizes << endl << "  starts   = " << starts << endl
            << "  order    = " << order << endl;
#endif

        Teuchos::RCP< MPI_Datatype > commPad = Teuchos::rcp(new MPI_Datatype);
        MPI_Type_create_subarray(ndims,
                                 &sizes[0],
                                 &subsizes[0],
                                 &starts[0],
                                 order,
                                 datatype,
                                 commPad.get());
        MPI_Type_commit(commPad.get());
        messageInfo.datatype = commPad;
      }
#endif

    }
    _sendMessages[msgAxis][1] = messageInfo;
  }

#ifdef DOMI_MDVECTOR_OUTPUT_INITIALIZE
  for (int proc = 0; proc < _teuchosComm->getSize(); ++proc)
  {
    if (proc == rank)
    {
      cout << msg.str();
      msg.flush();
    }
    _teuchosComm->barrier();
  }
#endif

}

}  // Namespace Domi

#endif
