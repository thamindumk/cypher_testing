#include "ScopeManager.h"

using namespace std;

ScopeManager::ScopeManager() {
    enterScope();// Initialize with a global scope
}

void ScopeManager::enterScope() {
    // Create a new scope and use shared_ptr to manage its memory
    auto *newScope = new Scope(currentScope);
    currentScope = newScope;
    scopeStack.push(newScope);
}

void ScopeManager::exitScope() {
    if (!scopeStack.empty()) {
        scopeStack.pop(); // Remove the current scope from the stack

        // Update currentScope to the new top of the stack or nullptr if empty
        currentScope = scopeStack.empty() ? nullptr : scopeStack.top();
    }
}

void ScopeManager::addSymbol(const string& symbolName, const string& symbolType) {
    if (currentScope) { // Ensure currentScope is valid
        currentScope->addSymbol(symbolName, symbolType);
    }
}

void ScopeManager::clearTable()
{
    if(currentScope)
    {
        currentScope->clearTable();
    }
}

string ScopeManager::getType(const std::string& symbolName)
{
    if(currentScope)
    {
        return currentScope->getType(symbolName);
    }
}

optional<string> ScopeManager::lookup(const string& symbolName) const {
    if (currentScope) { // Ensure currentScope is valid
        return currentScope->lookup(symbolName);
    }
    return nullopt;
}
