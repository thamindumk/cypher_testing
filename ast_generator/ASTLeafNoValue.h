#include "ASTNode.h"
#ifndef ASTLEAFNOVALUE_H
#define ASTLEAFNOVALUE_H

class ASTLeafNoValue : public ASTNode {
public:
    ASTLeafNoValue(const std::string& name)
    {
        this->nodeType = name;
    };

};

#endif //ASTLEAFNOVALUE_H
