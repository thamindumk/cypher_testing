//
// Created by thamindu on 8/28/24.
//
#include <iostream>
#include "../ast_generator/ASTNode.h"
#include "ScopeManager.h"  // Include scope manager header

#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

class SemanticAnalyzer {
public:
    // Constructor
    SemanticAnalyzer();

    // Destructor
    ~SemanticAnalyzer();

    // Public method to analyze the AST
    bool analyze(ASTNode* root, bool canDefine = false, string type = "ANY");

    ScopeManager* getScopeManager();
    unordered_map<std::string, std::string> *temp;

private:
    ScopeManager* scopeManager;  // Add a ScopeManager instance
    void clearTemp();
    // Private helper methods
    bool checkVariableDeclarations(ASTNode* node, string type);
    bool checkVariableUsage(ASTNode* node, string type);

    // Method to report errors
    void reportError(const std::string &message, ASTNode* node);
};

#endif //SEMANTICANALYZER_H
