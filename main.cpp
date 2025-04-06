#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "CypherLexer.h"
#include "CypherParser.h"
#include "ast_generator/ASTBuilder.h"
#include "ast_generator/ASTNode.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "query_planner/QueryPlanner.h"

using namespace std;

int main() {
    antlr4::ANTLRInputStream input("MATCH (n:Person) RETURN n.city, COUNT(n) AS cityCount Group BY cityCount DESC");
    //match (n)-[r:Works]-(m:Person)-[]-() return n
    // Create a lexer from the inpu
    CypherLexer lexer(&input);

    // Create a token stream from the lexer
    antlr4::CommonTokenStream tokens(&lexer);

    // Create a parser from the token stream
    CypherParser parser(&tokens);

    ASTBuilder ast_builder;
    auto* ast = any_cast<ASTNode*>(ast_builder.visitOC_Cypher(parser.oC_Cypher()));
    cout<<ast->print(1);
    SemanticAnalyzer semantic_analyzer;
    if(semantic_analyzer.analyze(ast))
    {
        QueryPlanner query_planner;
        query_planner.setRoot(ast);
        Operator* opr = query_planner.createExecutionPlan(ast);
        opr->execute();
    }else
    {
        cout<<"Error"<<endl;
    }


    return 0;
}


