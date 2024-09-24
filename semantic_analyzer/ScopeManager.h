#ifndef SCOPE_MANAGER_H
#define SCOPE_MANAGER_H

#include <stack>
#include <optional>
#include <string>
#include "Scope.h"

class ScopeManager {
public:
    ScopeManager();

    void enterScope();
    void exitScope();
    void addSymbol(const std::string& symbolName, const std::string& symbolType);
    void clearTable();
    std::string getType(const std::string& symbolName);
    std::optional<std::string> lookup(const std::string& symbolName) const;

private:
    std::stack<Scope*> scopeStack; // Stack of smart pointers
    Scope* currentScope = nullptr;           // Current scope managed by smart pointer
};

#endif // SCOPE_MANAGER_H
