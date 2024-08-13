#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "CypherLexer.h"
#include "CypherParser.h"
#include "ast_generator/ASTBuilder.h"

using namespace std;

int main() {

    ifstream file("/home/thamindu/CLionProjects/cypher/sample_queries.txt");

    // Check if the file was opened successfully
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
        return 1; // Return a non-zero value to indicate an error
    }

    string line;
    while (std::getline(file, line)) {
        if(!line.empty())
        {
            std::clock_t start = std::clock();
            cout<<line<<endl;
            // Provide the input text in a stream
            antlr4::ANTLRInputStream input(line);
            // Create a lexer from the input
            CypherLexer lexer(&input);

            // Create a token stream from the lexer
            antlr4::CommonTokenStream tokens(&lexer);

            // Create a parser from the token stream
            CypherParser parser(&tokens);

            CypherASTBuilder ast_builder;
            auto* ast = any_cast<ASTNode*>(ast_builder.visitProg(parser.prog()));
            ast->print(1);

            std::clock_t end = std::clock();
            // Calculate duration
            double duration = double(end - start) / CLOCKS_PER_SEC;

            // Output the duration in seconds
            std::cout << "Elapsed time: " << duration << " seconds" << std::endl;
        }
    }
    file.close();

    return 0;
}
