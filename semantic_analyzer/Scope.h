// Scope.h
#ifndef SCOPE_H
#define SCOPE_H

#include <string>
#include <unordered_map>
#include <optional>

class Scope {
public:
    Scope(Scope* parent = nullptr);
    void addSymbol(const std::string& symbolName, const std::string& symbolType);
    void clearTable();
    std::string getType(const std::string& symbolName);
    std::optional<std::string> lookup(const std::string& symbolName) const;
    std::unordered_map<std::string, std::string> *symbolTable;
private:
    Scope* parentScope;
};

#endif // SCOPE_H
