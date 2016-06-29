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
        src/IA_x86Details.C 
        src/IA_powerDetails.C 
        src/IA_aarch64Details.C 
        src/IA_platformDetailsFactory.C 
        src/CFGModifier.C
        src/StackTamperVisitor.C
	src/JumpTablePred.C
	src/BoundFactCalculator.C
	src/BoundFactData.C
	src/IndirectAnalyzer.C
	src/IndirectASTVisitor.C
	src/ThunkData.C
	../dataflowAPI/src/ABI.C 
	src/dominator.C
	src/LoopAnalyzer.C
	src/Loop.C
	src/LoopTreeNode.C
	src/IdiomModelDesc.C
	src/ProbabilisticParser.C
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
		../dataflowAPI/rose/semantics/RegisterParts.C
		../dataflowAPI/rose/semantics/Registers.C
		../dataflowAPI/rose/semantics/SMTSolver.C
		../dataflowAPI/rose/semantics/BinarySymbolicExpr.C
		../dataflowAPI/rose/semantics/RegisterStateGeneric.C
        ../dataflowAPI/src/RoseInsnFactory.C 
        ../dataflowAPI/src/slicing.C 
        ../dataflowAPI/src/stackanalysis.C 
        ../dataflowAPI/src/SymbolicExpansion.C 
        ../dataflowAPI/src/SymEval.C 
        ../dataflowAPI/src/SymEvalPolicy.C 
        ../dataflowAPI/src/templates.C 
        ../dataflowAPI/src/Visitors.C 
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

if (PLATFORM MATCHES amd64 OR PLATFORM MATCHES i386 OR PLATFORM
MATCHES x86_64)
set (SRC_LIST ${SRC_LIST}
     src/InstructionSource-x86.C
     src/IA_x86.C
)
elseif (PLATFORM MATCHES ppc)
set (SRC_LIST ${SRC_LIST}
     src/InstructionSource-power.C
     src/IA_power.C
)
elseif (PLATFORM MATCHES aarch64)
set (SRC_LIST ${SRC_LIST}
     src/InstructionSource-aarch64.C
     src/IA_aarch64.C
)
endif()

if (ENABLE_PARSE_API_GRAPHS)
set (SRC_LIST ${SRC_LIST}
     src/GraphAdapter.C
    )
endif()

SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX COMPILE_FLAGS -w)

ADD_DEFINITIONS(-DPARSER_LIB)
ADD_DEFINITIONS(-DDATAFLOW_LIB)

dyninst_library(parseAPI common instructionAPI ${SYMREADER})

if (UNIX)
	# Boost auto-links on Windows; don't double-link
	target_link_private_libraries(parseAPI ${Boost_LIBRARIES})
endif()

FILE (GLOB headers "h/*.h")
FILE (GLOB dataflowheaders "../dataflowAPI/h/*.h")
set_target_properties (parseAPI PROPERTIES PUBLIC_HEADER "${headers};${dataflowheaders}")
if(${ENABLE_STATIC_LIBS})
  set_target_properties (parseAPI_static PROPERTIES PUBLIC_HEADER "${headers};${dataflowheaders}")
endif()

install (TARGETS parseAPI
    RUNTIME DESTINATION ${INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
