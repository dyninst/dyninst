# CMake configuration for parseAPI directory

include_directories (
    ${PROJECT_SOURCE_DIR}/parseAPI/src
  )

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
    src/IA_amdgpu.C
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
        ../dataflowAPI/rose/semantics/DispatcherAmdgpuVega.C
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
if(WIN32)
ADD_DEFINITIONS(-DROSE_UTIL_EXPORTS)
ADD_DEFINITIONS(-DNOMINMAX)
endif()
dyninst_library(parseAPI common instructionAPI ${SYMREADER})

target_link_private_libraries(parseAPI ${Boost_LIBRARIES} ${TBB_LIBRARIES} tbbmalloc)

if (WIN32)
target_link_private_libraries(parseAPI shlwapi)
endif()
message(STATUS "Architecture is: ${CMAKE_LIBRARY_ARCHITECTURE}")
FILE (GLOB headers "h/*.h")
FILE (GLOB dataflowheaders "../dataflowAPI/h/*.h")
set_target_properties (parseAPI PROPERTIES PUBLIC_HEADER "${headers};${dataflowheaders}")

if (USE_OpenMP)
set_target_properties (parseAPI PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS} LINK_FLAGS ${OpenMP_CXX_FLAGS})
endif()

if(${ENABLE_STATIC_LIBS})
  set_target_properties (parseAPI_static PROPERTIES PUBLIC_HEADER "${headers};${dataflowheaders}")
endif()

install (TARGETS parseAPI
    RUNTIME DESTINATION ${INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
