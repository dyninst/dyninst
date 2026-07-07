project(liveness_dump LANGUAGES CXX)

add_executable(liveness_dump liveness_dump.cpp)
target_link_libraries(liveness_dump PRIVATE Dyninst::dyninstAPI Dyninst::parseAPI Dyninst::symtabAPI)
