// @HEADER
// ***********************************************************************
//
//                 Anasazi: Block Eigensolvers Package
//                 Copyright (2004) Sandia Corporation
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
// @HEADER
//
//  This example shows how to use the block Krylov-Schur method to compute a few
//  of the largest singular values (sigma) and corresponding right singular 
//  vectors (v) for the matrix A by solving the symmetric problem:
//
//                             (A'*A)*v = sigma*v
//
//  where A is an m by n real matrix that is derived from the simplest finite
//  difference discretization of the 2-dimensional kernel K(s,t)dt where
//
//                    K(s,t) = s(t-1)   if 0 <= s <= t <= 1
//                             t(s-1)   if 0 <= t <= s <= 1
//
//  NOTE:  This example came from the ARPACK SVD driver dsvd.f
//
#include "AnasaziBlockKrylovSchurSolMgr.hpp"
#include "AnasaziBasicEigenproblem.hpp"
#include "AnasaziConfigDefs.hpp"
#include "AnasaziEpetraAdapter.hpp"

#include "Epetra_CrsMatrix.h"
#include "Teuchos_LAPACK.hpp"

#ifdef EPETRA_MPI
#include "Epetra_MpiComm.h"
#include <mpi.h>
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Map.h"

int main(int argc, char *argv[]) {
  int i, j, info;
  const double one = 1.0;
  const double zero = 0.0;
  Teuchos::LAPACK<int,double> lapack;

#ifdef EPETRA_MPI
  // Initialize MPI
  MPI_Init(&argc,&argv);
  Epetra_MpiComm Comm(MPI_COMM_WORLD);
#else
  Epetra_SerialComm Comm;
#endif

  int MyPID = Comm.MyPID();

  //  Dimension of the matrix
  int m = 500;
  int n = 100;

  // Construct a Map that puts approximately the same number of
  // equations on each processor.

  Epetra_Map RowMap(m, 0, Comm);
  Epetra_Map ColMap(n, 0, Comm);

  // Get update list and number of local equations from newly created Map.

  int NumMyRowElements = RowMap.NumMyElements();
  
  std::vector<int> MyGlobalRowElements(NumMyRowElements);
  RowMap.MyGlobalElements(&MyGlobalRowElements[0]);

  /* We are building an m by n matrix with entries
    
              A(i,j) = k*(si)*(tj - 1) if i <= j
               = k*(tj)*(si - 1) if i  > j
  
     where si = i/(m+1) and tj = j/(n+1) and k = 1/(n+1).
  */

  // Create an Epetra_Matrix
  Teuchos::RCP<Epetra_CrsMatrix> A = Teuchos::rcp( new Epetra_CrsMatrix(Copy, RowMap, n) );

  // Compute coefficients for discrete integral operator
  std::vector<double> Values(n);
  std::vector<int> Indices(n);
  double inv_mp1 = one/(m+1);
  double inv_np1 = one/(n+1);
  for (i=0; i<n; i++) { Indices[i] = i; }
  
  for (i=0; i<NumMyRowElements; i++) {
    //
    for (j=0; j<n; j++) {
      //
      if ( MyGlobalRowElements[i] <= j ) {
        Values[j] = inv_np1 * ( (MyGlobalRowElements[i]+one)*inv_mp1 ) * ( (j+one)*inv_np1 - one );  // k*(si)*(tj-1)
      }
      else {
        Values[j] = inv_np1 * ( (j+one)*inv_np1 ) * ( (MyGlobalRowElements[i]+one)*inv_mp1 - one );  // k*(tj)*(si-1)
      }
    }
    info = A->InsertGlobalValues(MyGlobalRowElements[i], n, &Values[0], &Indices[0]);
    assert( info==0 );
  }

  // Finish up
  info = A->FillComplete(ColMap, RowMap);
  assert( info==0 );
  info = A->OptimizeStorage();
  assert( info==0 );
  A->SetTracebackMode(1); // Shutdown Epetra Warning tracebacks

  //************************************
  // Start the block Arnoldi iteration
  //***********************************
  //
  //  Variables used for the Block Arnoldi Method
  //
  int nev = 4;
  int blockSize = 1;
  int numBlocks = 10;
  int maxRestarts = 20;
  int verbosity = Anasazi::Errors + Anasazi::Warnings + Anasazi::FinalSummary;
  double tol = lapack.LAMCH('E');
  std::string which = "LM";
  //
  // Create parameter list to pass into solver
  //
  Teuchos::ParameterList MyPL;
  MyPL.set( "Verbosity", verbosity );
  MyPL.set( "Which", which );
  MyPL.set( "Block Size", blockSize );
  MyPL.set( "Num Blocks", numBlocks );
  MyPL.set( "Maximum Restarts", maxRestarts );
  MyPL.set( "Convergence Tolerance", tol );

  typedef Anasazi::MultiVec<double> MV;
  typedef Anasazi::Operator<double> OP;

  // Create an Anasazi::EpetraMultiVec for an initial vector to start the solver. 
  // Note:  This needs to have the same number of columns as the blocksize.
  Teuchos::RCP<Anasazi::EpetraMultiVec> ivec = Teuchos::rcp( new Anasazi::EpetraMultiVec(ColMap, blockSize) );
  ivec->MvRandom();

  // Call the constructor for the (A^T*A) operator
  Teuchos::RCP<Anasazi::EpetraSymOp>  Amat = Teuchos::rcp( new Anasazi::EpetraSymOp(A) );  
  Teuchos::RCP<Anasazi::BasicEigenproblem<double, MV, OP> > MyProblem =
    Teuchos::rcp( new Anasazi::BasicEigenproblem<double, MV, OP>(Amat, ivec) );

  // Inform the eigenproblem that the matrix A is symmetric
  MyProblem->setHermitian(true);

  // Set the number of eigenvalues requested and the blocksize the solver should use
  MyProblem->setNEV( nev );

  // Inform the eigenproblem that you are finished passing it information
  bool boolret = MyProblem->setProblem();
  if (boolret != true) {
    if (MyPID == 0) {
      cout << "Anasazi::BasicEigenproblem::setProblem() returned with error." << endl;
    }
#ifdef HAVE_MPI
    MPI_Finalize() ;
#endif
    return -1;
  }

  // Initialize the Block Arnoldi solver
  Anasazi::BlockKrylovSchurSolMgr<double, MV, OP> MySolverMgr(MyProblem, MyPL);
  
  // Solve the problem to the specified tolerances or length
  Anasazi::ReturnType returnCode = MySolverMgr.solve();
  if (returnCode != Anasazi::Converged && MyPID==0) {
    cout << "Anasazi::EigensolverMgr::solve() returned unconverged." << endl;
  }

  // Get the eigenvalues and eigenvectors from the eigenproblem
  Anasazi::Eigensolution<double,MV> sol = MyProblem->getSolution();
  std::vector<Anasazi::Value<double> > evals = sol.Evals;
  int numev = sol.numVecs;

  if (numev > 0) {
    
    // Compute singular values/vectors and direct residuals.
    //
    // Compute singular values which are the square root of the eigenvalues
    if (MyPID==0) {
      cout<<"------------------------------------------------------"<<endl;
      cout<<"Computed Singular Values: "<<endl;
      cout<<"------------------------------------------------------"<<endl;
    }
    for (i=0; i<numev; i++) { evals[i].realpart = Teuchos::ScalarTraits<double>::squareroot( evals[i].realpart ); }
    //
    // Compute left singular vectors :  u = Av/sigma
    //
    std::vector<double> tempnrm(numev), directnrm(numev);
    std::vector<int> index(numev);
    for (i=0; i<numev; i++) { index[i] = i; }
    Anasazi::EpetraMultiVec Av(RowMap,numev), u(RowMap,numev);
    Anasazi::EpetraMultiVec* evecs = dynamic_cast<Anasazi::EpetraMultiVec* >(sol.Evecs->CloneView( index ));
    Teuchos::SerialDenseMatrix<int,double> S(numev,numev);
    A->Apply( *evecs, Av );
    Av.MvNorm( &tempnrm );
    for (i=0; i<numev; i++) { S(i,i) = one/tempnrm[i]; };
    u.MvTimesMatAddMv( one, Av, S, zero );
    //
    // Compute direct residuals : || Av - sigma*u ||
    //
    for (i=0; i<numev; i++) { S(i,i) = evals[i].realpart; }
    Av.MvTimesMatAddMv( -one, u, S, one );
    Av.MvNorm( &directnrm );
    if (MyPID==0) {
      cout.setf(ios_base::right, ios_base::adjustfield);
      cout<<std::setw(16)<<"Singular Value"
	  <<std::setw(20)<<"Direct Residual"
	  <<endl;
      cout<<"------------------------------------------------------"<<endl;
      for (i=0; i<numev; i++) {
	cout<<std::setw(16)<<evals[i].realpart
	    <<std::setw(20)<<directnrm[i] 
	    <<endl;
      }  
      cout<<"------------------------------------------------------"<<endl;
    }
  }
  
#ifdef EPETRA_MPI
    MPI_Finalize() ;
#endif
  
  return 0;
}
