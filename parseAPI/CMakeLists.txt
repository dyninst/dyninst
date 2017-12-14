# CMake configuration for parseAPI directory

include_directories (
    ${PROJECT_SOURCE_DIR}/parseAPI/src
  )
cmake_policy(PUSH)
cmake_policy(SET CMP0012 OLD)
find_package(OpenMP)
cmake_policy(POP)

set(PARALLEL_OPTIONS serial)
if(OPENMP_FOUND)
	message(STATUS "OpenMP available")
	list(APPEND PARALLEL_OPTIONS OpenMP)
endif()
if(TBB_FOUND)
	message(STATUS "TBB available")
	list(APPEND PARALLEL_OPTIONS TBB)
	list(APPEND RAJA_CMAKE_FLAGS -DTBB_ROOT_DIR=${TBB_ROOT_DIR})
	list(APPEND RAJA_CMAKE_FLAGS -DTBB_INCLUDE_DIR=${TBB_INCLUDE_DIR})
	list(APPEND RAJA_CMAKE_FLAGS -DTBB_LIBRARY=${TBB_LIBRARY})
endif()

set(DEFAULT_PARALLEL_TYPE serial)
if(OPENMP_FOUND)
	set(DEFAULT_PARALLEL_TYPE OpenMP)
endif()
if(TBB_FOUND)
	set(DEFAULT_PARALLEL_TYPE TBB)
endif()
message(STATUS "Available parallelization kits: ${PARALLEL_OPTIONS}")
if(NOT PARALLEL_TYPE)
	set(PARALLEL_TYPE ${DEFAULT_PARALLEL_TYPE}
			CACHE STRING "Type of parallelism to use in ParseAPI"
			FORCE)
endif()
set_property(CACHE PARALLEL_TYPE
		PROPERTY STRINGS ${PARALLEL_OPTIONS})
message(STATUS "Using ${PARALLEL_TYPE} for ParseAPI...")


message(STATUS "Flags for OpenMP: ${OpenMP_CXX_FLAGS}")
set (SRC_LIST
        src/ParserDetails.C 
        src/Parser.C 
        src/CFGFactory.C 
        src/Function.C 
        src/Block.C 
        src/CodeObject.C 
        src/debug_parse.C 
        src/CodeSource.C 
        src/ParseData.C
        src/InstructionAdapter.C
        src/Parser-speculative.C
        src/ParseCallback.C 
        src/IA_IAPI.C
	src/IA_x86.C
	src/IA_power.C
	src/IA_aarch64.C
        src/CFGModifier.C
        src/StackTamperVisitor.C
	src/JumpTableFormatPred.C
	src/JumpTableIndexPred.C
	src/IndirectAnalyzer.C
	src/IndirectASTVisitor.C
	src/SymbolicExpression.C
	src/BoundFactCalculator.C
	src/BoundFactData.C
	src/ThunkData.C
	../dataflowAPI/src/ABI.C 
	src/dominator.C
	src/LoopAnalyzer.C
	src/Loop.C
	src/LoopTreeNode.C
	src/IdiomModelDesc.C
	src/ProbabilisticParser.C
    )

set(ROSE_SRC
	../dataflowAPI/src/ABI.C 
        ../dataflowAPI/src/Absloc.C 
        ../dataflowAPI/src/AbslocInterface.C 
        ../dataflowAPI/src/convertOpcodes.C 
        ../dataflowAPI/src/debug_dataflow.C 
        ../dataflowAPI/src/ExpressionConversionVisitor.C 
        ../dataflowAPI/src/InstructionCache.C 
        ../dataflowAPI/src/liveness.C 
        ../dataflowAPI/src/RegisterMap.C
	../dataflowAPI/src/RoseImpl.C
        ../dataflowAPI/src/RoseInsnFactory.C
        ../dataflowAPI/src/slicing.C
        ../dataflowAPI/src/stackanalysis.C
        ../dataflowAPI/src/SymbolicExpansion.C
        ../dataflowAPI/src/SymEval.C
        ../dataflowAPI/src/SymEvalPolicy.C
        ../dataflowAPI/src/templates.C
        ../dataflowAPI/src/Visitors.C
        ../dataflowAPI/rose/ExtentMap.C
        ../dataflowAPI/rose/rangemap.C
        ../dataflowAPI/rose/util/Assert.C
        ../dataflowAPI/rose/util/Message.C
        ../dataflowAPI/rose/util/Sawyer.C
        ../dataflowAPI/rose/util/Synchronization.C
        ../dataflowAPI/rose/util/rose_getline.C
        ../dataflowAPI/rose/util/SmallObject.C
        ../dataflowAPI/rose/util/Stopwatch.C
        ../dataflowAPI/rose/util/StringUtility.C
        ../dataflowAPI/rose/util/Attribute.C
        ../dataflowAPI/rose/util/Combinatorics.C
        ../dataflowAPI/rose/util/LinearCongruentialGenerator.C
        ../dataflowAPI/rose/semantics/BaseSemantics2.C
        #../dataflowAPI/rose/semantics/ConcreteSemantics2.C
        ../dataflowAPI/rose/semantics/DispatcherARM64.C
	../dataflowAPI/rose/semantics/DispatcherPowerpc.C
        ../dataflowAPI/rose/semantics/RegisterParts.C
        ../dataflowAPI/rose/semantics/Registers.C
        #../dataflowAPI/rose/semantics/SMTSolver.C
        ../dataflowAPI/rose/semantics/BinarySymbolicExpr.C
        ../dataflowAPI/rose/semantics/RegisterStateGeneric.C
        ../dataflowAPI/rose/semantics/SymEvalSemantics.C
)

# FIXME: Rose needs a bunch of warning cleanup
SET_SOURCE_FILES_PROPERTIES(${ROSE_SRC} PROPERTIES LANGUAGE CXX COMPILE_FLAGS -w)
set (SRC_LIST ${SRC_LIST}
	${ROSE_SRC}
)

if (LIGHTWEIGHT_SYMTAB)
set (SRC_LIST ${SRC_LIST}
    src/SymLiteCodeSource.C
)
else()
set (SRC_LIST ${SRC_LIST}
    src/SymtabCodeSource.C
)
endif()

if (ENABLE_PARSE_API_GRAPHS)
set (SRC_LIST ${SRC_LIST}
     src/GraphAdapter.C
    )
endif()


ADD_DEFINITIONS(-DPARSER_LIB)
ADD_DEFINITIONS(-DDATAFLOW_LIB)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/RAJA/include)
if(WIN32)
ADD_DEFINITIONS(-DROSE_UTIL_EXPORTS)
ADD_DEFINITIONS(-DNOMINMAX)
endif()
SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
dyninst_library(parseAPI common instructionAPI ${SYMREADER})
add_dependencies(parseAPI RAJA)
set(OPENMP_LIBRARIES gomp)
target_link_private_libraries(parseAPI ${Boost_LIBRARIES} ${TBB_LIBRARIES} tbbmalloc ${OPENMP_LIBRARIES})

if (WIN32)
target_link_private_libraries(parseAPI shlwapi)
endif()
message(STATUS "Architecture is: ${CMAKE_LIBRARY_ARCHITECTURE}")
FILE (GLOB headers "h/*.h")
FILE (GLOB dataflowheaders "../dataflowAPI/h/*.h")
set_target_properties (parseAPI PROPERTIES PUBLIC_HEADER "${headers};${dataflowheaders}")
if (USE_COTIRE)
    cotire(parseAPI)
endif()
if(${ENABLE_STATIC_LIBS})
  set_target_properties (parseAPI_static PROPERTIES PUBLIC_HEADER "${headers};${dataflowheaders}")
endif()

install (TARGETS parseAPI
    RUNTIME DESTINATION ${INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
