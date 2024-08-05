cmake_minimum_required(VERSION 3.28)
project(cypher)

set(CMAKE_CXX_STANDARD 17)

# Include the ANTLR4 runtime
include_directories(/usr/local/include/antlr4-runtime)
link_directories(/usr/local/lib)

# Include the generated ANTLR4 files
include_directories(${CMAKE_SOURCE_DIR}/generated)

# Add the ANTLR4 runtime and generated parser files
file(GLOB GENERATED_SRC ${CMAKE_SOURCE_DIR}/generated/*.cpp)
add_executable(cypher main.cpp ${GENERATED_SRC}
        semantic_analyzer/SemanticAnalyzer.cpp
        semantic_analyzer/SemanticAnalyzer.h
        ast_generator/ASTStructure.cpp
        ast_generator/ASTStructure.h
        ast_generator/ASTBuilder.cpp
        ast_generator/ASTBuilder.h
        ast_generator/ASTInternalNode.cpp
        ast_generator/ASTInternalNode.h
        ast_generator/ASTLeafValue.cpp
        ast_generator/ASTLeafValue.h
        ast_generator/ASTLeafNoValue.cpp
        ast_generator/ASTLeafNoValue.h)

# Link the ANTLR4 runtime
target_link_libraries(cypher antlr4-runtime)