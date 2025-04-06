// QueryPlanner.h
#ifndef QUERY_PLANNER_H
#define QUERY_PLANNER_H

#include "../ast_generator/ASTNode.h"
#include "Operators.h" // Include all operators
#include <algorithm>

class QueryPlanner {
 private:
    ASTNode* root;
 public:
    QueryPlanner() = default;
    ~QueryPlanner() = default;
    ASTNode* getRoot() { return root; }
    void setRoot(ASTNode* root) { this->root = root; }

    Operator* createExecutionPlan(ASTNode* ast, Operator* op = nullptr, string var = "");
    bool isAllChildAreGivenType(string nodeType, ASTNode* root);
    bool isAvailable(string nodeType, ASTNode* subtree);
    vector<ASTNode*> getSubTreeListByNodeType(ASTNode* root, string nodeType);
    ASTNode* verifyTreeType(ASTNode* root, string nodeType);
    pair<vector<bool>, vector<ASTNode*>> getRelationshipDetails(ASTNode* node);
    pair<vector<bool>, vector<ASTNode*>> getNodeDetails(ASTNode* node);
    Operator* pathPatternHandler(ASTNode* pattern, Operator* opr);
    ASTNode* prepareWhereClause(string var1, string var2);

};

#endif // QUERY_PLANNER_H
