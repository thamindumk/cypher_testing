#include <iostream>
#include "antlr4-runtime.h"
#include "CypherLexer.h"
#include "CypherParser.h"
#include "ast_generator/ASTBuilder.h"

using namespace std;

int main() {
    const string input_string = "MATCH (n:LIMIT) WHERE n.age < 30 RETURN n.name AS young UNION MATCH (n:Person) WHERE n.age >= 30 RETURN n.name[0] AS not_young";
    //MATCH (n:Person) WHERE n.age < 30 RETURN n.name AS young UNION MATCH (n:Person) WHERE n.age >= 30 RETURN n.name AS not_young
    // Provide the input text in a stream
    antlr4::ANTLRInputStream input(input_string);
    // Create a lexer from the input
    CypherLexer lexer(&input);

    // Create a token stream from the lexer
    antlr4::CommonTokenStream tokens(&lexer);

    // Create a parser from the token stream
    CypherParser parser(&tokens);

    CypherASTBuilder ast_builder;
    auto* ast = any_cast<ASTNode*>(ast_builder.visitProg(parser.prog()));
    ast->print(1);
    return 0;
}