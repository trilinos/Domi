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

#ifndef DOMI_MDARRAY_HPP
#define DOMI_MDARRAY_HPP

// Standard includes
#include <cstdarg>

// Teuchos includes
#include "Teuchos_Array.hpp"

// Domi includes
#include "Domi_MDArray_Utils.hpp"
#include "Domi_MDIterator.hpp"
#include "Domi_MDArrayView.hpp"

namespace Domi
{

// I put these non-member template functions here for the same reason
// that Ross did the same thing for the Teuchos::Array class.  See
// Teuchos_Array.hpp for details.
template< typename T > class MDArray;

/** \brief Equality operator.
 *
 * \relates MDArray
 */
template< typename T >
bool operator==(const MDArray< T > & a1, const MDArray< T > & a2);

/** \brief MDArray/MDArrayView equality operator.
 *
 * \relates MDArray
 * \relates MDArrayView
 */
template< typename T >
bool operator==(const MDArray< T > & a1, const MDArrayView< T > & a2);

/** \brief MDArrayView/MDArray equality operator.
 *
 * \relates MDArray
 * \relates MDArrayView
 */
template< typename T >
bool operator==(const MDArrayView< T > & a1, const MDArray< T > & a2);

/** \brief Inequality operator.
 *
 * \relates MDArray
 */
template< typename T >
bool operator!=(const MDArray< T > & a1, const MDArray< T > & a2);

/** \brief MDArrayView inequality operator.
 *
 * \relates MDArray
 */
template< typename T >
bool operator!=(const MDArray< T > & a1, const MDArrayView< T > & a2);

/** \brief MDArrayView/MDArray inequality operator.
 *
 * \relates MDArray
 * \relates MDArrayView
 */
template< typename T >
bool operator!=(const MDArrayView< T > & a1, const MDArray< T > & a2);

/** \brief Non-member swap
 *
 * \relates MDArray
 */
template< typename T >
void swap(MDArray< T > & a1, MDArray< T > & a2);

/** \brief Memory-safe templated multi-dimensional array class
 *
 * \section Domi_MDArray_DesignDiscussion_sec MDArray Design Discussion
 *
 * The purpose of the <tt>MDArray</tt> class is to provide
 * multi-dimensional array storage for an arbitrary number of
 * dimensions.  It stores a <tt>Teuchos::Array</tt> as member data to
 * provide its data buffer and so adopts all of the memory safety of
 * the <tt>Teuchos::Array</tt> class.  Storage order can be set by the
 * user to be "C" ordering or "Fortran" ordering (C-ordering is also
 * known as row-major or last-index-fastest; Fortran-ordering is also
 * known as column-major or first-index-fastest).  If not specified,
 * the default is Fortran ordering.
 *
 * The <tt>MDArray</tt> class also provides efficient indexing of the
 * form <tt>a(i,j,k,...)</tt>, regardless of storage order.  Here,
 * <tt>i,j,k</tt> must all be ordinals of type <tt>size_type</tt>, and
 * the return type is <tt>T &</tt>, where <tt>T</tt> is the type of
 * data stored in the array.
 *
 * The square bracket operator, for which the C++ standard requires
 * one and only one argument, can take a <tt>size_type</tt> ordinal or
 * a <tt>Slice</tt> struct and always returns an <tt>MDArrayView</tt>
 * object.  Providing a <tt>Slice</tt> argument (where a
 * <tt>Slice</tt> contains a start index, a stop index, and a step
 * interval) will return an <tt>MDArrayView</tt> object with the same
 * number of dimensions.  Providing a <tt>size_type</tt> argument will
 * return an <tt>MDArrayView</tt> object with one fewer dimensions.
 * It is possible to mix ordinal indexes and slice indexes by
 * repeatedly using the square bracket operator.  For example, if
 * <tt>MDArray a</tt> is 3-dimensional, <tt>a[Slice()][5][4]</tt>
 * would return a 1-dimensional <tt>MDArrayView</tt> which is all
 * values of index <tt>i</tt> at <tt>j</tt>=5 and <tt>k</tt>=4.  Note
 * that the default <tt>Slice()</tt> object represents a full range.
 * Note also that since the square bracket operator always returns an
 * <tt>MDArrayView</tt>, that <tt>a[i][j][k]</tt> would return an
 * <tt>MDArrayView<T></tt> of one dimension of length 1, not a
 * <tt>T&</tt>.  So in this case, what is probably desired is
 * <tt>a(i,j,k)</tt>.
 *
 * \section Domi_MDArray_Construction_sec MDArray Construction
 *
 * All <tt>MDArray</tt> constructors allocate new memory.  (To
 * construct an array that points to a view of existing memory, use
 * the <tt>MDArrayView</tt> class.)  Each constructor needs to know
 * the number of dimensions and the length of each dimension.  This is
 * provided by an <tt>Teuchos::ArrayView</tt> object whose size is the
 * number of dimensions and whose values are the lengths of each of
 * those dimensions.  A common and convenient way to provide an
 * <tt>Teuchos::ArrayView</tt> object to <tt>MDArray</tt> constructors
 * is with the overloaded <tt>Teuchos::tuple</tt> non-member
 * constructor functions.  For example, to construct an integer array
 * with dimensions (5,6,7), you can use
 *
 *     \code
 *     MDArray<int> a(Teuchos::tuple(5,6,7));
 *     \endcode
 *
 * There are various other optional arguments that can be passed to
 * the <tt>MDArray</tt> constructors.  These include a flag for
 * storage order, of enumerated type <tt>EStorageOrder</tt>, and a
 * default fill value of type <tt>T</tt>.
 *
 * \section Domi_MDArray_Indexing MDArray Indexing
 *
 * An introductory discussion of the indexing is provided above.  Here
 * we provide more details to help users and developers understand
 * some of the mechanics.
 *
 * Since the compiler does not know the dimension of any given
 * <tt>MDArray</tt>, the operators to access a single element of the
 * <tt>MDArray</tt> take the following forms:
 *
 *     \code
 *     T & operator()(size_type i, ...);
 *     const T & operator()(size_type i, ...);
 *     \endcode
 *
 * Functions and methods that take arbitrary arguments denoted by the
 * ellipsis ("...") are called "variadic", and unfortunately, they
 * reduce the amount of error checking that can be done, even with
 * <tt>DOMI_ENABLE_ABC</tt> defined.  The reason for this
 * is that the macros provided in C/C++ to handle variadic arguments
 * do not provide a way to count the number of arguments provided.  If
 * you provide too many indexes, the excess are just ignored.  If you
 * provide too few, the behavior is undefined.  The expectation is
 * that if you construct an <tt>MDArray</tt> of <tt>n</tt> dimensions,
 * then you will call operator() with <tt>n</tt> indexes.
 *
 * The offset represented by a set of indexes is computed by
 * maintaining an internal array of stride lengths, called
 * <tt>_strides</tt>.  These are initialized such that (say, for the
 * case of a three dimensional array) <tt>a(i,j,k)</tt> will compute
 * offset
 *
 *     \code
 *     i * _strides[0] + j * _strides[1] + k * _strides[2]
 *     \endcode
 *
 * This supplies sufficient flexibility to support both C and Fortran
 * storage orders, simply by computing the stride lengths correctly.
 * It also allows for correct indexing into sub-arrays.  The stride
 * values cannot be changed external to the class, but you can request
 * a reference to their values with the
 *
 *     \code
 *     const Teuchos::Array<size_type> & strides() const;
 *     \endcode
 *
 * method.
 *
 * The remaining <tt>operator[]</tt> indexing operators each return an
 * <tt>MDArrayView</tt> object with a view into the calling
 * <tt>MDArray</tt>.  The <tt>operator[](size_type)</tt> operator
 * returns an <tt>MDArrayView</tt> object with one fewer dimensions.
 * The <tt>operator[](Slice)</tt> operator returns an
 * <tt>MDArrayView</tt> object with the same number of dimensions, but
 * with a sub-view into the calling <tt>MDArray</tt>.  Which axis
 * provides the sub-view requires a little explanation.
 *
 * Suppose we set <tt>b = a[Slice]</tt>.  We expect <tt>b</tt> to be
 * an <tt>MDArrayView</tt> with the same number of dimensions as
 * <tt>a</tt>, but with a sub-view of the first axis.  Logically, we
 * would expect <tt>b[Slice]</tt> to also be a subview of the first
 * axis.  However, this is equivalent to <tt>a[Slice][Slice]</tt>, and
 * in this case we would expect that the second square brack would
 * reference a sub-view of the second axis.  In order to make
 * <tt>a[Slice][Slice]</tt> work the way we would expect, we have to
 * store a private "next axis" data member, and update it
 * appropriately as square bracket operators are applied.  This means
 * that the <tt>b[Slice]</tt> example above would actually refer to
 * the second axis, not the first.  For this reason, it is strongly
 * suggested that when indexing an n-dimensional <tt>MDArray</tt> with
 * the square bracket operator, <i>always</i> chain together
 * <tt>n</tt> square brackets, which will reset the internal "next
 * axis" data member to zero.
 *
 * \ingroup domi_mem_mng_grp
 */
template< typename T >
class MDArray
{
public:

  /** \name Teuchos::Array typedefs */
  //@{

  typedef typename Teuchos::Array< T >::size_type  size_type;
  typedef typename Teuchos::Array< T >::value_type value_type;

  //@}

  /** \name Constructors and Destructor */
  //@{

  /** \brief Default constructor
   *
   * Produces and <tt>MDArray</tt> with one dimension of length 0.
   */
  inline MDArray();

  /** \brief Constructor with dimensions only
   *
   * \param dims [in] An array that defines the lengths of each
   *        dimension.  The most convenient way to specify dimensions
   *        is with a Teuchos::Tuple returned by the non-member
   *        <tt>Teuchos::tuple<T>()</tt> function.
   */
  inline MDArray(const Teuchos::ArrayView< size_type > & dims);

  /** \brief Constructor with dimensions and default value,
   *         with optional storage order
   *
   * \param dims [in] An array that defines the lengths of each
   *        dimension.  The most convenient way to specify dimensions
   *        is with a Teuchos::Tuple returned by the non-member
   *        <tt>Teuchos::tuple<T>()</tt> function.
   *
   * \param value [in] The default value for filling the
   *        <tt>MDArray</tt>
   *
   * \param storageOrder [in] An enumerated value specifying the
   *        internal storage order of the <tt>MDArray</tt>
   */
  inline MDArray(const Teuchos::ArrayView< size_type > & dims,
		 const value_type & value,
		 const EStorageOrder storageOrder = DEFAULT_ORDER);

  /** \brief Constructor with dimensions and storage order,
   *         with optional default value
   *
   * \param dims [in] An array that defines the lengths of each
   *        dimension.  The most convenient way to specify dimensions
   *        is with a Teuchos::Tuple returned by the non-member
   *        <tt>Teuchos::tuple<T>()</tt> function.
   *
   * \param storageOrder [in] An enumerated value specifying the
   *        internal storage order of the <tt>MDArray</tt>
   *
   * \param value [in] The default value for filling the
   *        <tt>MDArray</tt>
   */
  inline MDArray(const Teuchos::ArrayView< size_type > & dims,
		 const EStorageOrder storageOrder,
		 const value_type & value = value_type());

  /** \brief Copy constructor
   *
   * \param source [in] The source <tt>MDArray</tt> to be copied
   */
  inline MDArray(const MDArray< T > & source);

  /** \brief Copy constructor from <tt>MDArrayView</tt>
   *
   * \param source [in] The source <tt>MDArrayView</tt> to be copied
   */
  MDArray(const MDArrayView< T > & source);

  /** \brief Destructor
   */
  ~MDArray();

  //@}

  /** \name Attribute accessor methods */
  //@{

  /** \brief Return the number of dimensions
   */
  inline int num_dims() const;

  /** \brief Return an array of dimensions
   */
  inline const Teuchos::Array< size_type > & dimensions() const;

  /** \brief Return the dimension of the given axis
   *
   * \param axis [in] The axis being queried (0 for the first axis,
   *        1 for the second axis, and so forth)
   */
  inline size_type dimension(int axis) const;

  /** \brief Return the total size of the <tt>MDArray</tt>
   */
  inline size_type size() const;

  /** \brief Return the indexing strides
   */
  inline const Teuchos::Array< size_type > & strides() const;

  /** \brief Return the underlying <tt>Teuchos::Array</tt>
   */
  inline const Teuchos::Array< T > & array() const;

  /** \brief Return the storage order
   */
  inline const EStorageOrder storage_order() const;

  //@}

  /** \name Iterator class and methods */
  //@{

  friend class MDIterator< MDArray< T > >;
  typedef MDIterator< MDArray< T > > iterator;

  /** \brief Return the beginning iterator
   */
  const iterator begin() const;

  /** \brief Return the ending iterator
   */
  const iterator end() const;

  //@}

  /** \name Conversions to MDArrayView */
  //@{

  /** \brief Perform an explicit conversion to a non-const
   *  <tt>MDArrayView<T></tt>
   */
  MDArrayView< T > mdArrayView();

  /** \brief Perform an explicit conversion to a const
   *  <tt>MDArrayView<T></tt>
   */
  const MDArrayView< T > mdArrayView() const;

  /** \brief Perform an explicit conversion to a non-const
   *  <tt>MDArrayView<const T></tt>
   */
  MDArrayView< const T > mdArrayViewConst();

  /** \brief Perform an explicit conversion to a const
   *  <tt>MDArrayView<const T></tt>
   */
  const MDArrayView< const T > mdArrayViewConst() const;

  /** \brief Perform an implicit conversion to a non-const
   *  <tt>MDArrayView</tt>
   */
  inline operator MDArrayView< T >() const;

  /** \brief Perform an implicit conversion to a const
   *  <tt>MDArrayView</tt>
   */
  inline operator MDArrayView< const T >() const;

  //@}

  /** \name Indexing operators that return <tt>MDArrayView</tt>s */
  //@{

  /** \brief Sub-array access operator.  The returned
   *  <tt>MDArrayView</tt> object will have one fewer dimensions than
   *  the calling <tt>MDArray</tt>.
   *
   * \param i [in] Index of the desired sub-array.  Note that to
   *        obtain expected behavior, you should always chain together
   *        <tt>n</tt> square bracket operators when referencing an
   *        <tt>n</tt>-dimensional <tt>MDArray</tt>.
   */
  MDArrayView< T > operator[](size_type i);

  /** \brief Sub-array const access operator.  The returned
   *  <tt>MDArrayView</tt> object will have one fewer dimensions than
   *  the calling <tt>MDArray</tt>.
   *
   * \param i [in] Index of the desired sub-array.  Note that to
   *        obtain expected behavior, you should always chain together
   *        <tt>n</tt> square bracket operators when referencing an
   *        <tt>n</tt>-dimensional <tt>MDArray</tt>.
   */
  const MDArrayView< T > operator[](size_type i) const;

  /** \brief Sub-array access operator.  The returned <tt>MDArrayView</tt>
   *  object will have the same number of dimensions as the calling
   *  <tt>MDArray</tt>.
   *
   * \param s [in] Slice representing the bounds of the desired
   *        sub-array.  Note that to obtain expected behavior, you
   *        should always chain together <tt>n</tt> square bracket operators
   *        when referencing an <tt>n</tt>-dimensional MDArray.
   */
  MDArrayView< T > operator[](Slice s);

  /** \brief Sub-array const access operator.  The returned
   *  <tt>MDArrayView</tt> object will have the same number of
   *  dimensions as the calling <tt>MDArray</tt>.
   *
   * \param s [in] Slice representing the bounds of the desired
   *        sub-array.  Note that to obtain expected behavior, you
   *        should always chain together <tt>n</tt> square bracket operators
   *        when referencing an <tt>n</tt>-dimensional MDArray.
   */
  const MDArrayView< T > operator[](Slice s) const;

  /** \brief Conversion to non-const <tt>MDArrayView</tt>
   */
  inline MDArrayView< T > operator()();

  /** \brief Conversion to const <tt>MDArrayView</tt>
   */
  inline const MDArrayView< T > operator()() const;

  //@}

  /** \name Indexing operators that return a reference to a single
   *        array element */
  //@{

  /** \brief Non-const 1D element access operator
   *
   * \param i [in] 1D index.
   *
   * This operator should only be used with a 1D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 1D, an
   * exception will be thrown.
   */
  inline T & operator()(size_type i);

  /** \brief Non-const 2D element access operator
   *
   * \param i [in] first 2D index.
   *
   * \param j [in] second 2D index.
   *
   * This operator should only be used with a 2D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 2D, an
   * exception will be thrown.
   */
  inline T & operator()(size_type i, size_type j);

  /** \brief Non-const 3D element access operator
   *
   * \param i [in] first 3D index.
   *
   * \param j [in] second 3D index.
   *
   * \param k [in] third 3D index.
   *
   * This operator should only be used with a 3D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 3D, an
   * exception will be thrown.
   */
  inline T & operator()(size_type i, size_type j, size_type k);

  /** \brief Non-const 4D element access operator
   *
   * \param i [in] first 4D index.
   *
   * \param j [in] second 4D index.
   *
   * \param k [in] third 4D index.
   *
   * \param m [in] fourth 4D index.
   *
   * This operator should only be used with a 4D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 4D, an
   * exception will be thrown.
   */
  inline T & operator()(size_type i, size_type j, size_type k, size_type m);

  /** \brief Non-const 5D element access operator
   *
   * \param i [in] first 5D index.
   *
   * \param j [in] second 5D index.
   *
   * \param k [in] third 5D index.
   *
   * \param m [in] fourth 5D index.
   *
   * \param n [in] fifth 5D index.
   *
   * This operator should only be used with a 5D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 5D, an
   * exception will be thrown.
   */
  inline T & operator()(size_type i, size_type j, size_type k, size_type m,
                        size_type n);

  /** \brief Non-const 6D and higher element access operator
   *
   * \param i [in] first index.
   *
   * \param j [in] second index.
   *
   * \param k [in] third index.
   *
   * \param m [in] fourth index.
   *
   * \param n [in] fifth index.
   *
   * \param p [in] sixth index.
   *
   * \param ... [in] seventh and higher indexes.
   *
   * This operator should only be used with a 6D and higher
   * <tt>MDArray</tt>s.  If DOMI_ENABLE_ABC is true and the
   * <tt>MDArray</tt> is less than 6D, an exception will be thrown.
   */
  inline T & operator()(size_type i, size_type j, size_type k, size_type m,
                        size_type n, size_type p, ...);

  /** \brief Const 1D element access operator
   *
   * \param i [in] 1D index.
   *
   * This operator should only be used with a 1D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 1D, an
   * exception will be thrown.
   */
  inline const T & operator()(size_type i) const;

  /** \brief Const 2D element access operator
   *
   * \param i [in] first 2D index.
   *
   * \param j [in] second 2D index.
   *
   * This operator should only be used with a 2D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 2D, an
   * exception will be thrown.
   */
  inline const T & operator()(size_type i, size_type j) const;

  /** \brief Const 3D element access operator
   *
   * \param i [in] first 3D index.
   *
   * \param j [in] second 3D index.
   *
   * \param k [in] third 3D index.
   *
   * This operator should only be used with a 3D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 3D, an
   * exception will be thrown.
   */
  inline const T & operator()(size_type i, size_type j, size_type k) const;

  /** \brief Const 4D element access operator
   *
   * \param i [in] first 4D index.
   *
   * \param j [in] second 4D index.
   *
   * \param k [in] third 4D index.
   *
   * \param m [in] fourth 4D index.
   *
   * This operator should only be used with a 4D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 4D, an
   * exception will be thrown.
   */
  inline const T & operator()(size_type i, size_type j, size_type k,
                              size_type m) const;

  /** \brief Const 5D element access operator
   *
   * \param i [in] first 5D index.
   *
   * \param j [in] second 5D index.
   *
   * \param k [in] third 5D index.
   *
   * \param m [in] fourth 5D index.
   *
   * \param n [in] fifth 5D index.
   *
   * This operator should only be used with a 5D <tt>MDArray</tt>.  If
   * DOMI_ENABLE_ABC is true and the <tt>MDArray</tt> is not 5D, an
   * exception will be thrown.
   */
  inline const T & operator()(size_type i, size_type j, size_type k,
                              size_type m, size_type n) const;

  /** \brief Const 6D and higher element access operator
   *
   * \param i [in] first index.
   *
   * \param j [in] second index.
   *
   * \param k [in] third index.
   *
   * \param m [in] fourth index.
   *
   * \param n [in] fifth index.
   *
   * \param p [in] sixth index.
   *
   * \param ... [in] seventh and higher indexes.
   *
   * This operator should only be used with a 6D and higher
   * <tt>MDArray</tt>s.  If DOMI_ENABLE_ABC is true and the
   * <tt>MDArray</tt> is less than 6D, an exception will be thrown.
   */
  inline const T & operator()(size_type i, size_type j, size_type k,
                              size_type m, size_type n, size_type p, ...) const;

  //@}

  /** \name Teuchos::Array-like and std::vector-like methods */
  //@{

  /** \brief Assign a value to all elements of the <tt>MDArray</tt>
   *
   * \param value [in] The value to be assigned
   */
  void assign(const T & value);

  /** \brief Non-const single element access method with bounds checking
   *
   * \param i, ... [in] Indexes representing the location of the
   *        single element of the <tt>MDArray</tt> to be accessed.  Note that
   *        this method assumes that the user will provide the same
   *        number of arguments as the number of dimensions of the
   *        <tt>MDArray</tt>.
   */
  T & at(size_type i, ...);

  /** \brief Const single element access method with bounds checking
   *
   * \param i, ... [in] Indexes representing the location of the
   *        single element of the <tt>MDArray</tt> to be accessed.  Note that
   *        this method assumes that the user will provide the same
   *        number of arguments as the number of dimensions of the
   *        <tt>MDArray</tt>.
   */
  const T & at(size_type i, ...) const;

  /** \brief Return the capacity of the underlying <tt>Teuchos::Array</tt>
   */
  inline size_type capacity() const;

  /** \brief Clear the <tt>MDArray</tt>
   */
  void clear();

  /** \brief Return whether the <tt>MDArray</tt> is empty
   */
  inline bool empty() const;

  /** \brief Return the maximum allowable size for the <tt>MDArray</tt>
   */
  inline size_type max_size() const;

  /** \brief Resize the <tt>MDArray</tt> based on the given dimensions
   *
   * \param dims [in] New dimensions of the resized <tt>MDArray</tt>
   */
  void resize(const Teuchos::ArrayView< size_type > & dims);

  /** \brief Swap this <tt>MDArray</tt> with the given <tt>MDArray</tt>
   */
  void swap(MDArray<T> & a);

  /** \brief Return true if <tt>MDArray</tt> has been compiled with bounds
   *         checking on. */
  inline static bool hasBoundsChecking();

  /** \brief Convert the <tt>MDArray</tt> to a string representation
   */
  std::string toString() const;

  /** \brief Return a const raw pointer to the beginning of the
   *         <tt>MDArray</tt> or NULL if unsized. */
  inline const T * getRawPtr() const;

  //@}

  // These operators are declared as friends so that the compiler will
  // do automatic type conversion.

  /** \name Non-member operators and functions */
  //@{

  /** \brief Equality operator.
   */
  template< typename T2 >
  friend bool operator==(const MDArray< T2 > & a1, const MDArray< T2 > & a2);

  /** \brief MDArray/MDArrayView equality operator.
   */
  template< typename T2 >
  friend bool operator==(const MDArray< T2 > & a1, const MDArrayView< T2 > & a2);

  /** \brief MDArrayView/MDArray equality operator.
   */
  template< typename T2 >
  friend bool operator==(const MDArrayView< T2 > & a1, const MDArray< T2 > & a2);

  /** \brief Inequality operator.
   */
  template< typename T2 >
  friend bool operator!=(const MDArray< T2 > & a1, const MDArray< T2 > & a2);

  /** \brief MDArray/MDArrayView inequality operator.
   */
  template< typename T2 >
  friend bool operator!=(const MDArray< T2 > & a1, const MDArrayView< T2 > & a2);

  /** \brief MDArrayView/MDArray inequality operator.
   */
  template< typename T2 >
  friend bool operator!=(const MDArrayView< T2 > & a1, const MDArray< T2 > & a2);

  /** \brief Stream output operator
   */
  template< typename T2 >
  friend std::ostream & operator<<(std::ostream & os, const MDArray< T2 > & a);

  /** \brief Swap function
   */
  template< typename T2 >
  friend void swap(MDArray< T2 > & a1, MDArray< T2 > & a2);

  //@}

private:
  Teuchos::Array< size_type > _dimensions;
  Teuchos::Array< size_type > _strides;
  Teuchos::Array< T >         _array;
  EStorageOrder               _storage_order;
  T *                         _ptr;

  // Used for array bounds checking
  void assertIndex(size_type i, int axis) const;
};

/////////////////////
// Implementations //
/////////////////////

template< typename T >
MDArray< T >::MDArray() :
  _dimensions(Teuchos::tuple< size_type >(0)),
  _strides(Teuchos::tuple< size_type >(1)),
  _array(),
  _storage_order(DEFAULT_ORDER),
  _ptr()
{
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::MDArray(const Teuchos::ArrayView< size_type > & dims) :
  _dimensions(dims),
  _strides(computeStrides(dims, DEFAULT_ORDER)),
  _array(computeSize(dims)),
  _storage_order(DEFAULT_ORDER),
  _ptr(_array.getRawPtr())
{
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::MDArray(const Teuchos::ArrayView< size_type > & dims,
		      const value_type & value,
		      const EStorageOrder storageOrder) :
  _dimensions(dims),
  _strides(computeStrides(dims, storageOrder)),
  _array(computeSize(dims), value),
  _storage_order(storageOrder),
  _ptr(_array.getRawPtr())
{
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::MDArray(const Teuchos::ArrayView< size_type > & dims,
		      const EStorageOrder storageOrder,
		      const value_type & value) :
  _dimensions(dims),
  _strides(computeStrides(dims, storageOrder)),
  _array(computeSize(dims), value),
  _storage_order(storageOrder),
  _ptr(_array.getRawPtr())
{
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::MDArray(const MDArray< T > & source) :
  _dimensions(source._dimensions),
  _strides(source._strides),
  _array(source._array),
  _storage_order(source._storage_order),
  _ptr(_array.getRawPtr())
{
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::MDArray(const MDArrayView< T > & source) :
  _dimensions(source.dimensions()),
  _strides(computeStrides(source.dimensions(), source.storage_order())),
  _array(computeSize(source.dimensions())),
  _storage_order(source.storage_order()),
  _ptr(_array.getRawPtr())
{
  // Copy the values from the MDArrayView to the MDArray
  iterator thisit = begin();
  typename MDArrayView< T >::iterator srcit = source.begin();
  for ( ; srcit != source.end(); ++thisit, ++srcit)
  {
    *thisit = *srcit;
  }
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::~MDArray()
{
}

////////////////////////////////////////////////////////////////////////

template< typename T >
int
MDArray< T >::num_dims() const
{
  return _dimensions.size();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const Teuchos::Array< typename Teuchos::Array< T >::size_type > &
MDArray< T >::dimensions() const
{
  return _dimensions;
}

////////////////////////////////////////////////////////////////////////

template< typename T >
typename Teuchos::Array< T >::size_type
MDArray< T >::dimension(int axis) const
{
  return _dimensions[axis];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
typename Teuchos::Array< T >::size_type
MDArray< T >::size() const
{
  return _array.size();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const Teuchos::Array< typename Teuchos::Array< T >::size_type > &
MDArray< T >::strides() const
{
  return _strides;
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const Teuchos::Array< T > &
MDArray< T >::array() const
{
  return _array;
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const EStorageOrder
MDArray< T >::storage_order() const
{
  return _storage_order;
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const typename MDArray< T >::iterator
MDArray< T >::begin() const
{
  return iterator(*this);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const typename MDArray< T >::iterator
MDArray< T >::end() const
{
  // Return the iterator corresponding to the last element
  return iterator(*this, true);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArrayView< T >
MDArray< T >::mdArrayView()
{
  return MDArrayView< T >(_array(), _dimensions, _storage_order);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const MDArrayView< T >
MDArray< T >::mdArrayView() const
{
  Teuchos::ArrayView< T > array(const_cast< T* >(_array.getRawPtr()),
                                _array.size());
  Teuchos::Array< size_type > dims(_dimensions);
  return MDArrayView< T >(array, dims(), _storage_order);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArrayView< const T >
MDArray< T >::mdArrayViewConst()
{
  return MDArrayView< const T >(_array, _dimensions, _storage_order);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const MDArrayView< const T >
MDArray< T >::mdArrayViewConst() const
{
  return MDArrayView< const T >(_array, _dimensions, _storage_order);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::operator MDArrayView< T >() const
{
  return mdArrayView();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArray< T >::operator MDArrayView< const T >() const
{
  return mdArrayViewConst();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArrayView< T >
MDArray< T >::operator[](size_type i)
{
  // Note: array bounds checking, if active, will be performed by the
  // MDArrayView class
  return mdArrayView()[i];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const MDArrayView< T >
MDArray< T >::operator[](size_type i) const
{
  // Note: array bounds checking, if active, will be performed by the
  // MDArrayView class
  return mdArrayView()[i];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArrayView< T >
MDArray< T >::operator[](Slice s)
{
  // Note: Slices produce safe indexes
  return mdArrayView()[s];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const MDArrayView< T >
MDArray< T >::operator[](Slice s) const
{
  // Note: Slices produce safe indexes
  return mdArrayView()[s];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
MDArrayView< T >
MDArray< T >::operator()()
{
  return mdArrayView();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const MDArrayView< T >
MDArray< T >::operator()() const
{
  return mdArrayView();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
T &
MDArray< T >::operator()(typename MDArray< T >::size_type i)
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 1), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 1 index"
    );
  assertIndex(i, 0);
#endif
  return _ptr[i * _strides[0]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j)
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 2), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 2 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
#endif
  return _ptr[i * _strides[0] + j * _strides[1]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k)
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 3), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 3 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
#endif
  return _ptr[i * _strides[0] + j * _strides[1] + k * _strides[2]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k,
                         typename MDArray< T >::size_type m)
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 4), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 4 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
  assertIndex(m, 3);
#endif
  return _ptr[i * _strides[0] + j * _strides[1] + k * _strides[2] +
              m * _strides[3]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k,
                         typename MDArray< T >::size_type m,
                         typename MDArray< T >::size_type n)
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 5), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 5 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
  assertIndex(m, 3);
  assertIndex(n, 4);
#endif
  return _ptr[i * _strides[0] + j * _strides[1] + k * _strides[2] +
              m * _strides[3] + n * _strides[4]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k,
                         typename MDArray< T >::size_type m,
                         typename MDArray< T >::size_type n,
                         typename MDArray< T >::size_type p,
                         ...)
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() < 6), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with too many indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
  assertIndex(m, 3);
  assertIndex(n, 4);
  assertIndex(p, 5);
#endif
  va_list indexes;
  size_type offset = i * _strides[0] + j * _strides[1] + k * _strides[2] +
                     m * _strides[3] + n * _strides[4] + p * _strides[5];
  va_start(indexes, p);
  for (size_type axis = 6; axis < _dimensions.size(); axis++)
  {
    size_type q = va_arg(indexes, size_type);
#ifdef DOMI_ENABLE_ABC
    assertIndex(q, axis);
#endif
    offset += q * _strides[axis];
  }
  va_end(indexes);
  return _ptr[offset];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T &
MDArray< T >::operator()(typename MDArray< T >::size_type i) const
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 1), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 1 index"
    );
  assertIndex(i, 0);
#endif
  return _ptr[i * _strides[0]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j) const
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 2), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 2 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
#endif
  return _ptr[i * _strides[0] + j * _strides[1]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k) const
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 3), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 3 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
#endif
  return _ptr[i * _strides[0] + j * _strides[1] + k * _strides[2]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k,
                         typename MDArray< T >::size_type m) const
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 4), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 4 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
  assertIndex(m, 3);
#endif
  return _ptr[i * _strides[0] + j * _strides[1] + k * _strides[2] +
              m * _strides[3]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k,
                         typename MDArray< T >::size_type m,
                         typename MDArray< T >::size_type n) const
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() != 5), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with 5 indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
  assertIndex(m, 3);
  assertIndex(n, 4);
#endif
  return _ptr[i * _strides[0] + j * _strides[1] + k * _strides[2] +
              m * _strides[3] + n * _strides[4]];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T &
MDArray< T >::operator()(typename MDArray< T >::size_type i,
                         typename MDArray< T >::size_type j,
                         typename MDArray< T >::size_type k,
                         typename MDArray< T >::size_type m,
                         typename MDArray< T >::size_type n,
                         typename MDArray< T >::size_type p,
                         ...) const
{
#ifdef DOMI_ENABLE_ABC
  TEUCHOS_TEST_FOR_EXCEPTION(
    (_dimensions.size() < 6), Teuchos::RangeError,
    "Attempt to access " << _dimensions.size() << "D array with too many indexes"
    );
  assertIndex(i, 0);
  assertIndex(j, 1);
  assertIndex(k, 2);
  assertIndex(m, 3);
  assertIndex(n, 4);
  assertIndex(p, 5);
#endif
  va_list indexes;
  size_type offset = i * _strides[0] + j * _strides[1] + k * _strides[2] +
                     m * _strides[3] + n * _strides[4] + p * _strides[5];
  va_start(indexes, p);
  for (size_type axis = 6; axis < _dimensions.size(); axis++)
  {
    size_type q = va_arg(indexes, size_type);
#ifdef DOMI_ENABLE_ABC
    assertIndex(q, axis);
#endif
    offset += q * _strides[axis];
  }
  va_end(indexes);
  return _ptr[offset];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
void
MDArray< T >::assign(const T & value)
{
  for (iterator it = begin(); it != end(); ++it)
    *it = value;
}

////////////////////////////////////////////////////////////////////////

template< typename T >
T &
MDArray< T >::at(size_type i, ...)
{
  assertIndex(i, 0);
  va_list indexes;
  size_type offset = i * _strides[0];
  va_start(indexes, i);
  for (size_type axis = 1; axis < _dimensions.size(); axis++)
  {
    size_type j = va_arg(indexes, size_type);
    assertIndex(j, axis);
    offset += j * _strides[axis];
  }
  va_end(indexes);
  return _ptr[offset];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T &
MDArray< T >::at(size_type i, ...) const
{
  assertIndex(i, 0);
  va_list indexes;
  size_type offset = i * _strides[0];
  va_start(indexes, i);
  for (size_type axis = 1; axis < _dimensions.size(); axis++)
  {
    size_type j = va_arg(indexes, size_type);
    assertIndex(j, axis);
    offset += j * _strides[axis];
  }
  va_end(indexes);
  return _ptr[offset];
}

////////////////////////////////////////////////////////////////////////

template< typename T >
typename MDArray< T >::size_type
MDArray< T >::capacity() const
{
  return _array.capacity();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
void
MDArray< T >::clear()
{
  _dimensions.resize(1);
  _dimensions[0] = 0;
  _strides.resize(1);
  _strides[0] = 1;
  _array.clear();
  _ptr = _array.getRawPtr();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool
MDArray< T >::empty() const
{
  return _array.empty();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
typename MDArray< T >::size_type
MDArray< T >::max_size() const
{
  return _array.max_size();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
void
MDArray< T >::resize(const Teuchos::ArrayView< size_type > & dims)
{
  _dimensions.assign(dims.begin(), dims.end());
  _strides = computeStrides(dims, _storage_order);
  _array.resize(computeSize(dims));
  _ptr = _array.getRawPtr();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
void
MDArray< T >::swap(MDArray<T> & a)
{
  // Use Teuchos::swap() to swap the dimensions, strides and
  // underlying array
  Teuchos::swap(_dimensions, a._dimensions);
  Teuchos::swap(_strides,    a._strides   );
  Teuchos::swap(_array,      a._array     );
  // Perform a raw swap of the storage order
  EStorageOrder tmp = _storage_order;
  _storage_order    = a._storage_order;
  a._storage_order  = tmp;
  // Make sure the pointers are correct
  _ptr   = _array.getRawPtr();
  a._ptr = a._array.getRawPtr();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool
MDArray< T >::hasBoundsChecking()
{
#ifdef DOMI_ENABLE_ABC
  return true;
#else
  return false;
#endif
}

////////////////////////////////////////////////////////////////////////

template< typename T >
std::string
MDArray< T >::toString() const
{
  return mdArrayView().toString();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
const T *
MDArray< T >::getRawPtr() const
{
  return _array.getRawPtr();
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool operator==(const MDArray< T > & a1, const MDArray< T > & a2)
{
  if (a1._dimensions != a2._dimensions) return false;
  if (a1._storage_order != a2._storage_order) return false;
  typename MDArray< T >::iterator it1 = a1.begin();
  typename MDArray< T >::iterator it2 = a2.begin();
  for ( ; it1 != a1.end() && it2 != a2.end(); ++it1, ++it2)
    if (*it1 != *it2) return false;
  return true;
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool operator==(const MDArray< T > & a1, const MDArrayView< T > & a2)
{
  return (a1() == a2);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool operator==(const MDArrayView< T > & a1, const MDArray< T > & a2)
{
  return (a1 == a2());
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool operator!=(const MDArray< T > & a1, const MDArray< T > & a2)
{
  return not (a1 == a2);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool operator!=(const MDArray< T > & a1, const MDArrayView< T > & a2)
{
  return (a1() != a2);
}

////////////////////////////////////////////////////////////////////////

template< typename T >
bool operator!=(const MDArrayView< T > & a1, const MDArray< T > & a2)
{
  return (a1 != a2());
}

////////////////////////////////////////////////////////////////////////

template< typename T >
std::ostream & operator<<(std::ostream & os, const MDArray< T > & a)
{
  os << a.toString();
  return os;
}

////////////////////////////////////////////////////////////////////////

template< typename T >
void swap(MDArray< T > & a1, MDArray< T > & a2)
{
  a1.swap(a2);
}

/////////////////////////////
// Private implementations //
/////////////////////////////

template< typename T >
void
MDArray< T >::assertIndex(size_type i, int axis) const
{
  TEUCHOS_TEST_FOR_EXCEPTION(
    !(0 <= i && i < _dimensions[axis]), Teuchos::RangeError,
    "MDArray<T>::assertIndex(i=" << i << ",axis=" << axis << "): out of range "
    << "i in [0, " << _dimensions[axis] << ")"
    );
}

}  // end namespace Domi

#endif
