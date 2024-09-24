// Scope.cpp
#include "Scope.h"
#include <iostream>
using namespace std;
Scope::Scope(Scope* parent)
{
    parentScope = parent;
    symbolTable = new unordered_map<string, string>();
}

void Scope::addSymbol(const string& symbolName, const std::string& symbolType) {
    pair<string,string> x = pair<string,string>(symbolName,symbolType);
    symbolTable->insert(x);
}

void Scope::clearTable()
{
    symbolTable->clear();
}

string Scope::getType(const string& symbolName) {
    return symbolTable->at(symbolName);
}

optional<string> Scope::lookup(const std::string& symbolName) const {

    if (symbolTable->find(symbolName) != symbolTable->end()) {
        return symbolTable->at(symbolName);
    } else if (parentScope) {
        return parentScope->lookup(symbolName);
    } else {
        return std::nullopt;
    }
}
