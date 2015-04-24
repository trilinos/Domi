include(Join)

# Ifpack2 ETI type fields
SET(Ifpack2_ETI_FIELDS "S|LO|GO|N")

ASSERT_DEFINED(Ifpack2_ENABLE_EXPLICIT_INSTANTIATION)
IF(Ifpack2_ENABLE_EXPLICIT_INSTANTIATION)
  MESSAGE(STATUS "User/Downstream ETI set: ${Ifpack2_ETI_LIBRARYSET}")
  MESSAGE(STATUS "Excluded instantiations: ${Ifpack2_ETI_EXCLUDE_SET}")
  MESSAGE(STATUS "Full coverage explicit instantiation for scalars:         ${Ifpack2_ETI_SCALARS}")
  MESSAGE(STATUS "Full coverage explicit instantiation for global ordinals: ${Ifpack2_ETI_GORDS}")
  MESSAGE(STATUS "Full coverage explicit instantiation for local ordinals:  ${Ifpack2_ETI_LORDS}")
  MESSAGE(STATUS "Full coverage explicit instantiation for nodes:           ${Ifpack2_ETI_NODES}")  
  JOIN(Ifpack2_ETI_SCALARS "|" FALSE ${Ifpack2_ETI_SCALARS})
  JOIN(Ifpack2_ETI_LORDS   "|" FALSE ${Ifpack2_ETI_LORDS}  )
  JOIN(Ifpack2_ETI_GORDS   "|" FALSE ${Ifpack2_ETI_GORDS}  )
  JOIN(Ifpack2_ETI_NODES   "|" FALSE ${Ifpack2_ETI_NODES}  )  
  # assemble single scalar instantiations
  TRIBITS_ETI_TYPE_EXPANSION(SingleScalarInsts   
                             "S=${Ifpack2_ETI_SCALARS}" 
                             "LO=${Ifpack2_ETI_LORDS}" 
                             "GO=${Ifpack2_ETI_GORDS}"
                             "N=${Ifpack2_ETI_NODES}")
  TRIBITS_ADD_ETI_INSTANTIATIONS(Ifpack2 ${SingleScalarInsts})
  IF(${PROJECT_NAME}_VERBOSE_CONFIGURE)
    MESSAGE(STATUS "ETI set (before exclusions): ${Ifpack2_ETI_LIBRARYSET}")
  ENDIF()
ELSE()
  # FIXME (mfh 11 Jan 2015) We should still use Tpetra to govern what
  # Scalar, LO, and GO types get used here.
  #
  # these macros are used only for testing
  TRIBITS_ETI_TYPE_EXPANSION(Ifpack2_ETI_LIBRARYSET "S=double" "LO=int" "GO=int|long" "N=${Ifpack2_ETI_NODES}")
  IF (SS_FOR_DEV_PS_FOR_RELEASE AND HAVE_COMPLEX_BLAS)
    TRIBITS_ETI_TYPE_EXPANSION(Ifpack2_ETI_LIBRARYSET "S=std::complex<double>" "LO=int" "GO=int|long" "N=${Ifpack2_ETI_NODES}")
  ENDIF()
ENDIF()

TRIBITS_ETI_GENERATE_MACROS("${Ifpack2_ETI_FIELDS}" "${Ifpack2_ETI_LIBRARYSET}" "${Ifpack2_ETI_EXCLUDE_SET}"  
                            list_of_manglings eti_typedefs
                            "IFPACK2_INSTANTIATE_LG(LO,GO)"         IFPACK2_ETIMACRO_LG
                            "IFPACK2_INSTANTIATE_SLG(S,LO,GO)"      IFPACK2_ETIMACRO_SLG
                            "IFPACK2_INSTANTIATE_LGN(S,LO,GO,N)"      IFPACK2_ETIMACRO_LGN
                            "IFPACK2_INSTANTIATE_SLGN(S,LO,GO,N)"     IFPACK2_ETIMACRO_SLGN                            
                            )
TRIBITS_ETI_GENERATE_MACROS("${Ifpack2_ETI_FIELDS}" "${Ifpack2_ETI_LIBRARYSET}" 
                            "${Ifpack2_ETI_EXCLUDE_SET};S=std::complex<double> LO=.* GO=.*; S=std::complex<float> LO=.* GO=.*"  
                            list_of_manglings eti_typedefs
                            "IFPACK2_INSTANTIATE_SLG_REAL(S,LO,GO)" IFPACK2_ETIMACRO_SLG_REAL
                            "IFPACK2_INSTANTIATE_SLGN_REAL(S,LO,GO,N)" IFPACK2_ETIMACRO_SLGN_REAL
                            )
TRIBITS_ETI_GENERATE_TYPEDEF_MACRO(IFPACK2_ETI_TYPEDEFS "IFPACK2_ETI_MANGLING_TYPEDEFS" "${eti_typedefs}")

CONFIGURE_FILE(${Ifpack2_SOURCE_DIR}/cmake/Ifpack2_ETIHelperMacros.h.in ${Ifpack2_BINARY_DIR}/src/Ifpack2_ETIHelperMacros.h)