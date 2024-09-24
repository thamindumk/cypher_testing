cmake_minimum_required(VERSION 3.22.1)
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
        ast_generator/ASTNode.cpp
        ast_generator/ASTNode.h
        ast_generator/ASTBuilder.cpp
        ast_generator/ASTBuilder.h
        ast_generator/ASTInternalNode.cpp
        ast_generator/ASTInternalNode.h
        ast_generator/ASTLeafValue.cpp
        ast_generator/ASTLeafValue.h
        ast_generator/ASTLeafNoValue.cpp
        ast_generator/ASTLeafNoValue.h
        semantic_analyzer/ScopeManager.cpp
        semantic_analyzer/ScopeManager.h
        semantic_analyzer/SemanticAnalyzer.cpp
        semantic_analyzer/SemanticAnalyzer.h
        semantic_analyzer/Scope.cpp
        semantic_analyzer/Scope.h
        query_planner/QueryPlanner.cpp
        query_planner/QueryPlanner.h
        query_planner/Operators.cpp
        query_planner/Operators.h
        query_planner/Operators.cpp
)

# Link the ANTLR4 runtime
target_link_libraries(cypher antlr4-runtime)