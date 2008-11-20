
INCLUDE(Parse_Variable_Arguments)


MACRO(TRILINOS_PRIVATE_ADD_TEST_SET_PASS_PROPERTY TEST_NAME_IN)

  IF (PARSE_PASS_REGULAR_EXPRESSION)
    SET_TESTS_PROPERTIES(${TEST_NAME_IN} PROPERTIES PASS_REGULAR_EXPRESSION
      ${PARSE_PASS_REGULAR_EXPRESSION})
  ENDIF()

  IF (PARSE_FAIL_REGULAR_EXPRESSION)
    SET_TESTS_PROPERTIES(${TEST_NAME_IN} PROPERTIES FAIL_REGULAR_EXPRESSION
      ${PARSE_FAIL_REGULAR_EXPRESSION})
  ENDIF()

  IF (PARSE_STANDARD_PASS_OUTPUT)
    SET_TESTS_PROPERTIES(${TEST_NAME_IN} PROPERTIES PASS_REGULAR_EXPRESSION
      "End Result: TEST PASSED")
  ENDIF()

ENDMACRO()


#
# Main function for adding a test
#
FUNCTION(TRILINOS_ADD_TEST EXE_NAME)
   
  #
  # A) Parse the input arguments
  #

  PARSE_ARGUMENTS(
     #prefix
     PARSE
     #lists
     "DIRECTORY;KEYWORDS;COMM;NUM_MPI_PROCS;ARGS;NAME;HOST;XHOST;PASS_REGULAR_EXPRESSION;FAIL_REGULAR_EXPRESSION"
     #options
     "NOEXEPREFIX;STANDARD_PASS_OUTPUT"
     ${ARGN}
     )

  IF (PARSE_ARGS)
    LIST(LENGTH PARSE_ARGS NUM_PARSE_ARGS)
  ELSE()
    SET(PARSE_ARGS " ")
    SET(NUM_PARSE_ARGS 1)
  ENDIF()

  IF (Trilinos_VERBOSE_CONFIGURE)
    MESSAGE("")
    MESSAGE("TRILINOS_ADD_TEST: EXE_NAME = ${EXE_NAME}")
  ENDIF()
  
  #
  # B) Do not add any tests if host or xhost says not to
  #
  
  IF(NOT PARSE_XHOST)
    SET (PARSE_XHOST NONE)
  ENDIF()    
  LIST (FIND PARSE_XHOST ${Trilinos_HOSTNAME} INDEX_OF_HOSTNAME_IN_XHOST_LIST)           
  IF (NOT ${INDEX_OF_HOSTNAME_IN_XHOST_LIST} EQUAL -1)
    RETURN()
  ENDIF()

  IF(NOT PARSE_HOST)
    SET (PARSE_HOST ${Trilinos_HOSTNAME})
  ENDIF()  
  LIST (FIND PARSE_HOST ${Trilinos_HOSTNAME} INDEX_OF_HOSTNAME_IN_HOST_LIST)                 
  IF (${INDEX_OF_HOSTNAME_IN_HOST_LIST} EQUAL -1)
    RETURN()
  ENDIF()

  #
  # C) Set the name and path of the binary that will be run
  #
  
  SET(EXE_BINARY_NAME "${EXE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
  IF(DEFINED PACKAGE_NAME AND NOT PARSE_NOEXEPREFIX)
    SET(EXE_BINARY_NAME ${PACKAGE_NAME}_${EXE_BINARY_NAME})
  ELSE()
  ENDIF()

  #MESSAGE("TRILINOS_ADD_TEST: ${EXE_NAME}: EXE_BINARY_NAME = ${EXE_BINARY_NAME}")

  IF(PARSE_NAME)
    SET(TEST_NAME "${PACKAGE_NAME}_${PARSE_NAME}")
  ELSE()
    SET(TEST_NAME "${PACKAGE_NAME}_${EXE_NAME}")  
  ENDIF()

  IF(PARSE_DIRECTORY)
    SET(EXECUTABLE_PATH "./${PARSE_DIRECTORY}/${EXE_BINARY_NAME}")
  ELSE()
    SET(EXECUTABLE_PATH "./${EXE_BINARY_NAME}")
  ENDIF()
  #MESSAGE("TRILINOS_ADD_TEST: ${EXE_NAME}: EXECUTABLE_PATH = ${EXECUTABLE_PATH}")

  #
  # D) Append keywords to the name of the test
  #
  
  IF(PARSE_KEYWORDS)
    FOREACH(KEYWORD ${PARSE_KEYWORDS})
      SET(TEST_NAME ${TEST_NAME}_${KEYWORD})
    ENDFOREACH()
  ENDIF()

  #
  # E) Determine if we will add the serial or MPI tests based on input COMM
  # and Trilinos_ENABLE_MPI
  #

  IF (PARSE_COMM)
    SET(ADD_MPI_TEST OFF)
    SET(ADD_SERIAL_TEST OFF)
    FOREACH(COMM ${PARSE_COMM})
      IF (COMM STREQUAL "mpi")
        SET(ADD_MPI_TEST ON)
      ELSEIF(COMM STREQUAL "serial")
        SET(ADD_SERIAL_TEST ON)
      ELSE()
        MESSAGE(SEND_ERROR "Error, the COMM value '${COMM}' is not valid!.  Only 'mpi' and 'serial' are allowed.")
      ENDIF()
    ENDFOREACH()
  ELSE()
    SET(ADD_MPI_TEST ON)
    SET(ADD_SERIAL_TEST ON)
  ENDIF()

  IF (Trilinos_ENABLE_MPI)
    SET(ADD_SERIAL_TEST OFF)
  ELSE()
    SET(ADD_MPI_TEST OFF)
  ENDIF()

  #
  # F) Get teh MPI options
  #
    
  IF(Trilinos_ENABLE_MPI)

    SET(NP)
    SET(NUM_PROCS_USED ${MPIEXEC_MAX_NUMPROCS})
    IF(PARSE_NUM_MPI_PROCS)
      IF(${PARSE_NUM_MPI_PROCS} MATCHES [0-9]+-[0-9]+)
        STRING(REGEX REPLACE "([0-9]+)-([0-9]+)" "\\1" MIN_NP ${PARSE_NUM_MPI_PROCS} )
        STRING(REGEX REPLACE "([0-9]+)-([0-9]+)" "\\2" MAX_NP ${PARSE_NUM_MPI_PROCS} )
        IF(${MIN_NP} LESS ${MPIEXEC_MAX_NUMPROCS} AND  ${MAX_NP} GREATER ${MPIEXEC_MAX_NUMPROCS} )
          SET(NUM_PROCS_USED ${MPIEXEC_MAX_NUMPROCS})
        ELSEIF(${MIN_NP} EQUAL ${MPIEXEC_MAX_NUMPROCS})
          SET(NUM_PROCS_USED ${MIN_NP})
        ELSEIF(${MAX_NP} EQUAL ${MPIEXEC_MAX_NUMPROCS})
          SET(NUM_PROCS_USED ${MAX_NP})
        ELSEIF(${MAX_NP} LESS ${MPIEXEC_MAX_NUMPROCS})
          SET(NUM_PROCS_USED ${MAX_NP})
        ELSE()
          # The number of available processor is outside the given range
          # so the test should not be run.
          RETURN()
        ENDIF()
      ELSE()
        IF(${PARSE_NUM_MPI_PROCS} LESS ${MPIEXEC_MAX_NUMPROCS})
          SET(NUM_PROCS_USED ${PARSE_NUM_MPI_PROCS})
        ELSE()
          SET(NUM_PROCS_USED ${MPIEXEC_MAX_NUMPROCS})
        ENDIF()
      ENDIF()
    ENDIF()

    SET(NP ${MPI_NUMPROCS_FLAG} ${NUM_PROCS_USED})

  ENDIF()
    
  #
  # G) Add the tests
  #

  IF(ADD_MPI_TEST)

    SET(TEST_NAME "${TEST_NAME}_MPI_${NUM_PROCS_USED}")
    
    SET(COUNTER 0)

    FOREACH(PARSE_ARG ${PARSE_ARGS})

      IF(${NUM_PARSE_ARGS} EQUAL 1)
        SET(TEST_NAME_COUNTER "${TEST_NAME}")
      ELSE()
        SET(TEST_NAME_COUNTER "${TEST_NAME}_${COUNTER}")
      ENDIF()
      IF(Trilinos_VERBOSE_CONFIGURE)
        MESSAGE(STATUS "TEST_NAME = ${TEST_NAME_COUNTER}")
      ENDIF()
      
      #This is a little bit of a hack
      #If the argument string has multiple arguments then the white space will need 
      #to replaced by a semicolin.  If this is not done the add_test command will
      #add a slash to each white space in the argument string.
      STRING(REPLACE " " ";" MYARG ${PARSE_ARG}) 

      ADD_TEST(${TEST_NAME_COUNTER} ${MPI_EXECUTABLE} ${NP} ${EXECUTABLE_PATH} ${MYARG})
      
      TRILINOS_PRIVATE_ADD_TEST_SET_PASS_PROPERTY(${TEST_NAME_COUNTER})

      MATH(EXPR COUNTER ${COUNTER}+1 )

    ENDFOREACH()

  ENDIF()
  
  IF(ADD_SERIAL_TEST)

    SET(COUNTER 0)

    FOREACH(PARSE_ARG ${PARSE_ARGS})

      IF(${NUM_PARSE_ARGS} EQUAL 1)
        SET(TEST_NAME_COUNTER "${TEST_NAME}")
      ELSE()
        SET(TEST_NAME_COUNTER "${TEST_NAME}_${COUNTER}")
      ENDIF()
      IF(Trilinos_VERBOSE_CONFIGURE)
        MESSAGE(STATUS "TEST_NAME = ${TEST_NAME_COUNTER}")
      ENDIF()
    
      # See above about this hack
      STRING(REPLACE " " ";" MYARG ${PARSE_ARG})
      ADD_TEST(${TEST_NAME_COUNTER} ${EXECUTABLE_PATH} ${MYARG})

      TRILINOS_PRIVATE_ADD_TEST_SET_PASS_PROPERTY(${TEST_NAME_COUNTER})

      MATH(EXPR COUNTER ${COUNTER}+1 )
    
    ENDFOREACH()

  ENDIF()
  
ENDFUNCTION()

