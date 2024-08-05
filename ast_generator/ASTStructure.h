//
// Created by thamindu on 7/24/24.
//
#include <iostream>
#include <vector>
#include <string>

#ifndef AST_STRUCTURE_H
#define AST_STRUCTURE_H

using namespace std;

class ASTNode {
    public:
        string nodeType;
        vector<ASTNode*> elements;
        string value;
        virtual ~ASTNode() = default;

    void print(int depth = 0, string prefix = "", bool isLast = true) const {
        cout << prefix;
        cout << (isLast ? "└───" : "├──");
        cout << nodeType << ": " << value << endl;

        prefix += isLast ? "    " : "│   ";

        for (size_t i = 0; i < elements.size(); ++i) {
            elements[i]->print(depth + 1, prefix, i == elements.size() - 1);
        }
    }

};




#endif //AST_STRUCTURE_H
