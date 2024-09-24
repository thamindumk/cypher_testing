#include "SemanticAnalyzer.h"
#include "ScopeManager.h"

using namespace std;
// Constructor
SemanticAnalyzer::SemanticAnalyzer() {
    // Initialize the scope manager with the global scope
    scopeManager = new ScopeManager();
    temp = new unordered_map<string, string>();// Enter global scope
}

// Destructor
SemanticAnalyzer::~SemanticAnalyzer() {
    // Ensure to exit all scopes upon destruction
    scopeManager->exitScope();
}


// Main method to analyze the AST
bool SemanticAnalyzer::analyze(ASTNode* root, bool canDefine, string type) {
    if(root->nodeType == "VARIABLE" && canDefine)
    {
        if(!checkVariableDeclarations(root, type))
        {
            return false;
        }
    }else if(root->nodeType == "VARIABLE")
    {
        if(!checkVariableUsage(root, type))
        {
            return false;
        }
    }else if (root->nodeType == "AS")
    {
        string ntype;
        if (root->elements[0]->nodeType == "LIST" || root->elements[0]->nodeType == "LIST_COMPREHENSION")
        {
            ntype = "LIST";
        }
        else if (root->elements[0]->nodeType == "PROPERIES_MAP")
        {
            ntype = "MAP";
        }
        else
        {
            ntype = "ANY";
        }
        if(!analyze(root->elements[0]))
        {
            return false;
        }
        if(root->elements[1]->nodeType != "VARIABLE" || !analyze(root->elements[1],true, ntype))
        {
            return false;
        }
    }else if(root->nodeType == "YIELD")
    {
        if(root->elements[0]->nodeType == "AS" && !analyze(root->elements[0], "STRING"))
        {
            return false;
        }
        if(!analyze(root->elements[0],true, "STRING"))
        {
            return false;
        }
    }else if(root->nodeType == "=")
    {
        string ntype = root->nodeType == "PATTERN_ELEMENTS" ? "PATH_PATTERN" : "ANY";
        if(!analyze(root->elements[0],true, ntype))
        {
            return false;
        }
        if(!analyze(root->elements[1]))
        {
            return false;
        }
    }else if(root->nodeType == "NODE_PATTERN" && !root->elements.empty())
    {
        if(root->elements[0]->nodeType == "VARIABLE" && !analyze(root->elements[0],true,"NODE"))
        {
            return false;
        }
        for(int i=1; i<root->elements.size(); i++)
        {
            if(!analyze(root->elements[i]))
            {
                return false;
            }
        }
    }else if(root->nodeType == "RELATIONSHIP_DETAILS" && !root->elements.empty())
    {
        if(root->elements[0]->nodeType == "VARIABLE" && !analyze(root->elements[0],true,"RELATIONSHIP"))
        {
            return false;
        }
        for(int i=1; i<root->elements.size(); i++)
        {
            if(!analyze(root->elements[i]))
            {
                return false;
            }
        }
    }else if(root->nodeType == "LIST_ITERATOR")
    {
        if(!analyze(root->elements[0],true))
        {
            return false;
        }
        if(!analyze(root->elements[1]))
        {
            return false;
        }
    }else if(root->nodeType == "NON_ARITHMETIC_OPERATOR" && root->elements[0]->nodeType == "VARIABLE")
    {
        if(root->elements[1]->nodeType == "LIST_INDEX_RANGE" && !analyze(root->elements[0], false, "LIST"))
        {
            return false;
        }else if(root->elements[1]->nodeType == "LIST_INDEX" && root->elements[1]->elements[0]->nodeType == "DECIMAL" && !analyze(root->elements[0], false, "LIST"))
        {
            return false;
        }else if (root->elements[1]->nodeType == "PROPERTY_LOOKUP" && !analyze(root->elements[0], false, "LOOKUP"))
        {
            return false;
        }
    }else if(root->nodeType == "EXISTS")
    {
        scopeManager->enterScope();
        for(int i=0; i<root->elements.size(); i++)
        {
            if(!analyze(root->elements[i]))
            {
                return false;
            }
        }
        scopeManager->exitScope();

    }else if(root->nodeType == "WITH")
    {
        for(int i=0; i<root->elements.size(); i++)
        {
            if(!analyze(root->elements[i]))
            {
                return false;
            }
        }

        clearTemp();
        auto *node = root->elements[0]->elements[0];
        for (auto* child: node->elements)
        {
            if(child->nodeType == "AS")
            {
                string tempType;
                if(child->elements[0]->nodeType == "VARIABLE")
                {
                    tempType = scopeManager->getType(child->elements[0]->value);
                }else
                {
                    tempType = "ANY";
                }
                pair<string,string> x = pair<string,string>(child->elements[0]->value,tempType);
                temp->insert(x);
            } else if(child->nodeType == "VARIABLE")
            {
                string tempType = scopeManager->getType(child->value);
                pair<string,string> x = pair<string,string>(child->value,tempType);
                temp->insert(x);
            }
            else
            {
                reportError("use 'as' keyword to assign it to new variable" + node->value, node);
                return false;
            }
        }
        scopeManager->clearTable();
        for (auto tempNode = temp->begin(); tempNode != temp->end(); ++tempNode) {
            scopeManager->addSymbol(tempNode->first,tempNode->second);
        }
    }
    else
    {
        for(int i=0; i<root->elements.size(); i++)
        {
            if(!analyze(root->elements[i]))
            {
                return false;
            }
        }
    }
    return true;
}

// Check for variable declarations
bool SemanticAnalyzer::checkVariableDeclarations(ASTNode* node, string type) {
     if (!scopeManager->lookup(node->value)){
        // If variable is not in current scope, add it
        scopeManager->addSymbol(node->value, type);
    }else if(scopeManager->lookup(node->value) == type)
    {
        return true;
    }
    else {
        reportError("Variable already declared: " + node->value, node);
        return false;
    }

    return true;
}

// Check for function definitions
void SemanticAnalyzer::clearTemp()
{
    temp->clear();
}

bool SemanticAnalyzer::checkVariableUsage(ASTNode* node, string type)
{
    if(!(scopeManager->lookup(node->value)))
    {
        reportError("Variable is not defined in this scope: " + node->value, node);
        return false;
    }else if(scopeManager->lookup(node->value) && type == "ANY")
    {
        return true;
    }else if (scopeManager->lookup(node->value) && scopeManager->lookup(node->value) == type)
    {
        return true;
    }
    else if (type == "LOOKUP" && (scopeManager->lookup(node->value) == "NODE" ||
                                  scopeManager->lookup(node->value) == "RELATIONSHIP" ||
                                  scopeManager->lookup(node->value) == "MAP"))
    {
        return true;
    }
    else
    {
        reportError("Variable type mismatch: " + node->value, node);
        return false;
    }

}

// Report errors
void SemanticAnalyzer::reportError(const std::string &message, ASTNode* node) {
    std::cerr << "Error: " << message << std::endl;
}
