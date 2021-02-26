// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
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
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#ifndef ROL2_TYPEU_CUBICINTERP_DEF_H
#define ROL2_TYPEU_CUBICINTERP_DEF_H

/** \class ROL2::TypeU::CubicInterp
    \brief Implements cubic interpolation back tracking line search.
*/

namespace ROL2 {
namespace TypeU {

template<typename Real>
CubicInterp<Real>::CubicInterp( ParameterList &parlist ) \
  : LineSearch_U<Real>(parlist) { 
  const Real half(0.5);
  auto& lslist = parlist.sublist("Step").sublist("Line Search");
  rho_ = lslist.sublist("Line-Search Method").get("Backtracking Rate",half);
}

template<typename Real>
void CubicInterp<Real>::initialize( const Vector<Real>& x, 
                 const Vector<Real>& g ) override {
  LineSearch<Real>::initialize(x,g);
  xnew_   = x.clone();
}

template<typename Real>
void CubicInterp<Real>::run(       Real&            alpha,
                                   Real&            fval,
                                   int&             ls_neval,
                                   int&             ls_ngrad,
                             const Real&            gs,
                             const Vector<Real>&    s,
                             const Vector<Real>&    x,
                                   Objective<Real>& obj ) override {

  Real tol = std::sqrt(ROL_EPSILON<Real>);
  ls_neval = 0;
  ls_ngrad = 0;

  // Get initial line search parameter
  alpha = LineSearch<Real>::getInitialAlpha(ls_neval,ls_ngrad,fval,gs,x,s,obj);

  // Update iterate
  xnew_->set(x); 
  xnew_->axpy(alpha,s);

  // Get objective value at xnew
  Real fold = fval;
  obj.update(*xnew_,UpdateType::Trial);
  fval = obj.value(*xnew_,tol);
  ls_neval++;

  // Initialize
  Real fvalp(0), alpha1(0), alpha2(0), a(0), b(0), x1(0), x2(0);
  const Real one(1), two(2), three(3), half(0.5), p1(0.1);
  bool first_iter = true;

  using LineSearc<Real>::Type;

  // Perform cubic interpolation back tracking
  while( !LineSearch<Real>::status(Type::CubicInterp,ls_neval,ls_ngrad,alpha,fold,gs,fval,x,s,obj) ) {
    if( first_iter ) { // Minimize quadratic interpolate
      alpha1 = -gs*alpha*alpha/(two*(fval-fold-gs*alpha));
      first_iter = false;
    }
    else {              // Minimize cubic interpolate
      x1 = fval-fold-alpha*gs;
      x2 = fvalp-fval-alpha2*gs;
      a = (one/(alpha - alpha2))*( x1/(alpha*alpha) - x2/(alpha2*alpha2));
      b = (one/(alpha - alpha2))*(-x1*alpha2/(alpha*alpha) + x2*alpha/(alpha2*alpha2));

      if (std::abs(a) < ROL_EPSILON<Real>)  alpha1 = -gs/(two*b);
      else                                  alpha1 = (-b+sqrt(b*b-three*a*gs))/(three*a);

      alpha1 = std::min(half*alpha,alpha);
    }

    alpha2 = alpha;
    fvalp  = fval;

    // Back track if necessary
    if(alpha1 <= p1*alpha)         alpha *= p1;
    else if(alpha1 >= half*alpha)  alpha *= half;
    else                           alpha = alpha1;
     
    // Update iterate
    xnew_->set(x); xnew_->axpy(alpha,s);

    // Get objective value at xnew
    obj.update(*xnew_,UpdateType::Trial);
    fval = obj.value(*xnew_,tol);
    ls_neval++;
  }
}

} // namespace TypeU
} // namespace ROL2

#endif // ROL2_TYPEU_CUBICINTERP_DEF_H
