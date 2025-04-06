#include "Operators.h"

#include "../ast_generator/ASTNode.h"
#include "../util/Const.h"
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;
// NodeScan Implementation
NodeScanByLabel::NodeScanByLabel(string label, string var) : label(label), var(var) {}

void NodeScanByLabel::execute() {
    cout<<"NodeScanByLabel: \n"<<endl;
    cout << "scanning node based on single label: " << label <<" and assigned with variable: "<<var<< endl;
}

// NodeByIdSeek Implementation
NodeByIdSeek::NodeByIdSeek(string id, string var) : id(id), var(var) {}

void NodeByIdSeek::execute() {
    cout<<"NodeByIdSeek: \n"<<endl;
    cout << "seeking node based on id: " << id <<" and assigned with variable: "<<var<< endl;
}

// MultipleNodeScanByLabel Implementation
MultipleNodeScanByLabel::MultipleNodeScanByLabel(vector<string> label, const string& var) : label(label), var(var) {}

void MultipleNodeScanByLabel::execute() {
    cout<<"MultipleNodeScanByLabel: \n"<<endl;

    string label_string = "";
    for(int i=0; i<label.size();i++)
    {
        label_string+= label[i];
        label_string+= ", ";
    }
    cout << "scanning node based on multiple label: " << label_string <<" and assigned with variable: "<<var<< endl;

}

// AllNodeScan Implementation
AllNodeScan::AllNodeScan(const string& var) : var(var) {}

void AllNodeScan::execute() {
    cout<<"AllNodeScan: \n"<<endl;
    cout << "scanning all nodes and assigned with : " << var << endl;
}

// ProduceResults Implementation
ProduceResults::ProduceResults(Operator* opr, vector<ASTNode*> item) : item(item), op(opr) {}

void ProduceResults::execute() {
    if(op){
        op->execute();
    }
    cout<<"ProduceResults: \n"<<endl;

    for(auto* e: item)
    {
        cout<<e->print()<<endl;
    }
}

Operator *ProduceResults::getOperator() {
    return this->op;
}

void ProduceResults::setOperator(Operator *op) {
    this->op = op;
}

// Filter Implementation
Filter::Filter(Operator* input, vector<pair<string,ASTNode*>> filterCases) : input(input), filterCases(filterCases) {}

string Filter::comparisonOperand(ASTNode *ast) {
    json operand;
    if (ast->nodeType == Const::NON_ARITHMETIC_OPERATOR) {
        // there are more cases to handle
        operand["variable"] = ast->elements[0]->value;
        operand["Type"] = Const::PROPERTY_LOOKUP;
        vector<string> property;
        for (auto* prop: ast->elements) {
            if (prop->nodeType == Const::PROPERTY_LOOKUP && prop->elements[0]->nodeType != Const::RESERVED_WORD) {
                property.push_back(prop->elements[0]->value);
            }
        }
        operand["property"] = property;
    } else if (ast->nodeType == Const::PROPERTIES_MAP) {
        operand["Type"] = Const::PROPERTIES_MAP;
        map<string, string> property;
        for (auto* prop: ast->elements) {
            if (prop->elements[0]->nodeType != Const::RESERVED_WORD){
                property.insert(pair<string, string>(prop->elements[0]->value, prop->elements[1]->value));
            }
        }
        operand["property"] = property;
    } else if (ast->nodeType == Const::LIST) {
        operand["Type"] = Const::LIST;
        vector<string> element;
        for (auto* prop: ast->elements) {
            element.push_back(prop->value);
        }

        operand["element"] = element;
    } else if (ast->nodeType == Const::FUNCTION_BODY) {
        operand["Type"] = Const::FUNCTION;
        operand["functionName"] = ast->elements[0]->elements[1]->value;
        vector<string> arguments;
        for (auto* arg: ast->elements[1]->elements) {
            arguments.push_back(arg->value);
        }
        operand["arguments"] = arguments;
    }else {
        operand["Type"] = ast->nodeType;
        operand["value"] = ast->value;
    }

    return operand.dump();
}

string Filter::analyze(ASTNode* ast) {
    json where;
    if (ast->nodeType == Const::OR) {
        where["Type"] = Const::OR;
        vector<json> comparisons;
        for (auto* element: ast->elements) {
            comparisons.push_back(json::parse(analyze(element)));
        }
        where["comparisons"] = comparisons;
    } else if(ast->nodeType == Const::AND) {
        where["Type"] = Const::AND;
        vector<json> comparisons;
        for (auto* element: ast->elements) {
            comparisons.push_back(json::parse(analyze(element)));
        }
        where["comparisons"] = comparisons;
    } else if(ast->nodeType == Const::XOR) {
        where["Type"] = Const::XOR;
        vector<string> comparisons;
        for (auto* element: ast->elements) {
            comparisons.push_back(json::parse(analyze(element)));
        }
        where["comparisons"] = comparisons;
    } else if(ast->nodeType == Const::NOT) {
        where["Type"] = Const::NOT;
        vector<json> comparisons;
        for (auto* element: ast->elements) {
            comparisons.push_back(json::parse(analyze(element)));
        }
        where["comparisons"] = comparisons;
    } else if(ast->nodeType == Const::COMPARISON) {
        where["type"] = Const::COMPARISON;
        auto* left = ast->elements[0];
        auto* oparator = ast->elements[1];
        auto* right = oparator->elements[0];
        where["left"] = json::parse(comparisonOperand(left));
        where["operator"] = oparator->nodeType;
        where["right"] = json::parse(comparisonOperand(right));
    }
    return where.dump();
}

void Filter::execute() {
    input->execute();
    cout<<"Filter: \n"<<endl;
    string condition;
    for(auto item: filterCases){
        if(item.second->nodeType==Const::WHERE){
            cout<<item.second->print(1)<<endl;
            condition = analyze(item.second->elements[0]);
        }else if(item.second->nodeType==Const::PROPERTIES_MAP){
            for(auto* prop: item.second->elements){
                condition+=item.first+"."+prop->elements[0]->value+" = "+prop->elements[1]->value;
                if(prop != item.second->elements.back()){
                    condition+=" AND \n";
                }
            }
        }else if(item.second->nodeType==Const::NODE_LABELS){
            for(auto* prop: item.second->elements){
                condition+=item.first+": "+prop->elements[0]->value;
                if(prop != item.second->elements.back()){
                    condition+=" AND \n";
                }
            }
        }else if(item.second->nodeType==Const::NODE_LABEL){
            condition = item.first+": "+item.second->elements[0]->value;
        }

        if(item != filterCases.back()){
            condition+=" AND \n";
        }
    }
    cout<<condition<<endl;

}

// Projection Implementation
Projection::Projection(Operator* input, const vector<ASTNode*> columns) : input(input), columns(columns) {}

void Projection::execute() {
    input->execute();

    cout << "Projecting columns: ";
    for (auto* col : columns) {
        cout << col->print()<< endl;
    }
    cout << endl;
}

// Join Implementation
Join::Join(Operator* left, Operator* right, const string& joinCondition) : left(left), right(right), joinCondition(joinCondition) {}

void Join::execute() {
    left->execute();
    right->execute();
    cout << "Joining on condition: " << joinCondition << endl;
}

// Aggregation Implementation
Aggregation::Aggregation(Operator* input, const string& aggFunction, const string& column) : input(input), aggFunction(aggFunction), column(column) {}

void Aggregation::execute() {
    input->execute();
    cout << "Performing aggregation: " << aggFunction << " on column: " << column << endl;
}

// Limit Implementation
Limit::Limit(Operator* input, ASTNode* limit) : input(input), limit(limit) {}

void Limit::execute() {
    input->execute();
    cout << "Limiting result to " << limit->print() << " rows." << endl;
}

// Skip Implementation
Skip::Skip(Operator* input, ASTNode* skip) : input(input), skip(skip) {}

void Skip::execute() {
    input->execute();
    cout << "Skipping first" << skip->print() << " rows." << endl;
}

// Sort Implementation
Sort::Sort(Operator* input, const string& sortByColumn, bool ascending) : input(input), sortByColumn(sortByColumn), ascending(ascending) {}

void Sort::execute() {
    input->execute();
    cout << "Sorting by column: " << sortByColumn << " in " << (ascending ? "ascending" : "descending") << " order." << endl;
}

// GroupBy Implementation
GroupBy::GroupBy(Operator* input, const vector<  string>& groupByColumns) : input(input), groupByColumns(groupByColumns) {}

void GroupBy::execute() {
    input->execute();
    cout << "Grouping by columns: ";
    for (const auto& col : groupByColumns) {
        cout << col << " ";
    }
    cout << endl;
}

// Distinct Implementation
Distinct::Distinct(Operator* input) : input(input) {}

void Distinct::execute() {
    input->execute();
    cout << "Applying Distinct to remove duplicates." << endl;
}

OrderBy::OrderBy(Operator* input, ASTNode* orderByClause) : input(input), orderByClause(orderByClause){}

void OrderBy::execute() {
    input->execute();
    cout << "Order By : "<<this->orderByClause->print()<< endl;
}

// Union Implementation
Union::Union(Operator* left, Operator* right) : left(left), right(right) {}

void Union::execute() {
    left->execute();
    right->execute();
    cout << "Performing Union of results." << endl;
}

// Intersection Implementation
Intersection::Intersection(Operator* left, Operator* right) : left(left), right(right) {}

void Intersection::execute() {
    left->execute();
    right->execute();
    cout << "Performing Intersection of results." << endl;
}

CacheProperty::CacheProperty(Operator* input, vector<ASTNode*> property) : property(property), input(input){}

void CacheProperty::execute()
{
    input->execute();
    cout<<"CacheProperty: \n"<<endl;

    int i=1;
    for(auto* prop: property)
    {
        string s = prop->elements[0]->value +"."+prop->elements[1]->elements[0]->value;
        cout<<"get property "<<i++<<" and cache here: "<<s<<endl;

    }
}

UndirectedRelationshipTypeScan::UndirectedRelationshipTypeScan(string relType, string relvar, string startVar, string endVar)
        : relType(relType), relvar(relvar), startVar(startVar), endVar(endVar) {}

// Execute method
void UndirectedRelationshipTypeScan::execute() {
    cout<<"UndirectedRelationshipTypeScan: \n"<<endl;
    json undirected;
    undirected["operator"] = "UndirectedRelationshipTypeScan";
    undirected["sourceVariable"] = startVar;
    undirected["destVariable"] = endVar;
    undirected["relVariable"] = relvar;
    undirected["relType"] = relType;
    cout << "("<<startVar<<") -[" << relvar<<" :"<< relType << "]- (" << endVar << ")" << endl;
    cout<<undirected.dump()<<endl;
}

UndirectedAllRelationshipScan::UndirectedAllRelationshipScan(string startVar, string endVar, string relVar)
    : startVar(startVar), endVar(endVar), relVar(relVar) {}


void UndirectedAllRelationshipScan::execute() {
    cout<<"UndirectedAllRelationshipScan: \n"<<endl;
    cout << "("<<startVar<<") -[" << relVar<<"]- (" << endVar << ")" << endl;
}

DirectedRelationshipTypeScan::DirectedRelationshipTypeScan(string direction, string relType, string relvar, string startVar, string endVar)
        : relType(relType), relvar(relvar), startVar(startVar), endVar(endVar), direction(direction) {

}

// Execute method
void DirectedRelationshipTypeScan::execute() {
    cout<<"DirectedRelationshipTypeScan: \n"<<endl;

    if(direction == "right"){
        cout << "("<<startVar<<") -[" << relvar<<" :"<< relType << "]-> (" << endVar << ")" << endl;
    }else{
        cout << "("<<startVar<<") <-[" << relvar<<" :"<< relType << "]- (" << endVar << ")" << endl;
    }
}

DirectedAllRelationshipScan::DirectedAllRelationshipScan(std::string direction, std::string startVar, std::string endVar, std::string relVar)
        : startVar(startVar), endVar(endVar), relVar(relVar), direction(direction) {}

void DirectedAllRelationshipScan::execute() {
    cout<<"DirectedAllRelationshipScan: \n"<<endl;

    if(direction == "right"){
        cout << "("<<startVar<<") -[" << relVar<<"]-> (" << endVar << ")" << endl;
    }else{
        cout << "("<<startVar<<") <-[" << relVar<<"]- (" << endVar << ")" << endl;
    }
}

ExpandAll::ExpandAll(Operator *input, std::string startVar, std::string destVar, std::string relVar,
                     std::string relType, std::string direction): input(input), relType(relType), relVar(relVar), startVar(startVar), destVar(destVar), direction(direction){}

void ExpandAll::execute() {
    input->execute();
    cout<<"ExpandAll: \n"<<endl;

    string line;
    json expandAll;
    expandAll["operator"] = "ExpandAll";
    if(relType == "null" && direction == ""){
        expandAll["sourceVariable"] = startVar;
        expandAll["destVariable"] = destVar;
        expandAll["relVariable"] = relVar;
        line = "("+startVar+") -["+relVar+"]- ("+destVar+")";
        cout<<line<<endl;
    }else if(relType != "null" && direction == ""){
        expandAll["sourceVariable"] = startVar;
        expandAll["destVariable"] = destVar;
        expandAll["relVariable"] = relVar;
        expandAll["relType"] = relType;
        line = "("+startVar+") -["+relVar+" :"+relType+"]- ("+destVar+")";
        cout<<line<<endl;
    }else if(relType == "null" && direction == "right") {
        expandAll["sourceVariable"] = startVar;
        expandAll["destVariable"] = destVar;
        expandAll["relVariable"] = relVar;
        expandAll["direction"] = direction;
        line = "("+startVar+") -["+relVar+"]-> ("+destVar+")";
        cout << "(" << startVar << ") -[" << relVar << "]-> (" << destVar << ")" << endl;
    }else if(relType != "null" && direction == "right"){
        expandAll["sourceVariable"] = startVar;
        expandAll["destVariable"] = destVar;
        expandAll["relVariable"] = relVar;
        expandAll["relType"] = relType;
        expandAll["direction"] = direction;
        cout << "(" << startVar << ") -[" << relVar << " :" << relType << "]-> (" << destVar << ")" << endl;
    }else if(relType == "null" && direction == "left") {
        expandAll["sourceVariable"] = startVar;
        expandAll["destVariable"] = destVar;
        expandAll["relVariable"] = relVar;
        expandAll["direction"] = direction;
        cout << "(" << startVar << ") <-[" << relVar << "]- (" << destVar << ")" << endl;
    }else if(relType != "null" && direction == "left"){
        expandAll["sourceVariable"] = startVar;
        expandAll["destVariable"] = destVar;
        expandAll["relVariable"] = relVar;
        expandAll["relType"] = relType;
        expandAll["direction"] = direction;
        cout << "(" << startVar << ") <-[" << relVar << " :" << relType << "]- (" << destVar << ")" << endl;
    }
    cout<<expandAll.dump()<<endl;
}

Apply::Apply(Operator* opr): opr1(opr){}

void Apply::addOperator(Operator *opr) {
    this->opr2 = opr;
}


// Execute method
void Apply::execute() {
    if (opr1 != nullptr){
        cout<<"     left of Apply"<<endl;
        opr1->execute();
    }
    if (opr2 != nullptr){
        cout<<"     right of Apply"<<endl;
        opr2->execute();
    }

    cout<<"Apply: result merged"<<endl;
}

EagerFunction::EagerFunction(Operator *input, ASTNode *ast, std::string functionName): input(input), ast(ast), functionName(functionName){}

void EagerFunction::execute() {
    input->execute();
    cout<<"Eager Function name: "<<functionName<<endl;
    cout<<ast->print()<<endl;
}

CartesianProduct::CartesianProduct(Operator* left, Operator* right) : left(left), right(right) {}

void CartesianProduct::execute() {
    left->execute();
    right->execute();
    cout << "Performing Cartesian Product." << endl;
}

Create::Create(Operator *input, ASTNode *ast) : ast(ast), input(input){}

void Create::execute() {
    json create;
    if (input != nullptr) {
        input->execute();
    }
    create["operator"] = "Create";
    vector<json> list;
    for (auto* e: ast->elements[0]->elements) {
        if (e->nodeType == Const::NODE_PATTERN) {
            json data;
            data["type"] = "Node";
            for (auto* element: e->elements) {
                if (element->nodeType == Const::NODE_LABEL) {
                    data["label"] = element->elements[0]->value;
                } else if (element->nodeType == Const::VARIABLE) {
                    data["variable"] = element->value;
                } else if (element->nodeType == Const::PROPERTIES_MAP) {
                    map<string, string> property;
                    for (auto* prop: element->elements) {
                        if (prop->elements[0]->nodeType != Const::RESERVED_WORD){
                            property.insert(pair<string, string>(prop->elements[0]->value, prop->elements[1]->value));
                        }
                    }
                    data["properties"] = property;
                }
            }
            list.push_back(data);
        } else if (e->nodeType == Const::PATTERN_ELEMENTS) {
            json data;
            data["type"] = "Relationships";
            vector<json> relationships;
            json relationship;
            json source;
            json rel;
            json dest;
            for (auto* patternElement: e->elements) {
                if (patternElement->nodeType == Const::NODE_PATTERN){
                    for (auto* element: patternElement->elements) {
                        if (element->nodeType == Const::NODE_LABEL) {
                            source["label"] = element->elements[0]->value;
                        } else if (element->nodeType == Const::VARIABLE) {
                            source["variable"] = element->value;
                        } else if (element->nodeType == Const::PROPERTIES_MAP) {
                            map<string, string> property;
                            for (auto* prop: element->elements) {
                                if (prop->elements[0]->nodeType != Const::RESERVED_WORD){
                                    property.insert(pair<string, string>(prop->elements[0]->value, prop->elements[1]->value));
                                }
                            }
                            source["properties"] = property;
                        }
                    }
                } else if (patternElement->nodeType == Const::PATTERN_ELEMENT_CHAIN) {
                    for (auto* element: patternElement->elements[0]->elements[1]->elements) {
                        if (element->nodeType == Const::RELATIONSHIP_TYPE) {
                            rel["type"] = element->elements[0]->value;
                        } else if (element->nodeType == Const::VARIABLE) {
                            rel["variable"] = element->value;
                        } else if (element->nodeType == Const::PROPERTIES_MAP) {
                            map<string, string> property;
                            for (auto* prop: element->elements) {
                                if (prop->elements[0]->nodeType != Const::RESERVED_WORD){
                                    property.insert(pair<string, string>(prop->elements[0]->value, prop->elements[1]->value));
                                }
                            }
                            rel["properties"] = property;
                        }
                    }

                    for (auto* element: patternElement->elements[1]->elements) {
                        if (element->nodeType == Const::NODE_LABEL) {
                            dest["label"] = element->elements[0]->value;
                        } else if (element->nodeType == Const::VARIABLE) {
                            dest["variable"] = element->value;
                        } else if (element->nodeType == Const::PROPERTIES_MAP) {
                            map<string, string> property;
                            for (auto* prop: element->elements) {
                                if (prop->elements[0]->nodeType != Const::RESERVED_WORD){
                                    property.insert(pair<string, string>(prop->elements[0]->value, prop->elements[1]->value));
                                }
                            }
                            dest["properties"] = property;
                        }
                    }

                    if (patternElement->elements[0]->elements[0]->nodeType == Const::RIGHT_ARROW) {
                        relationship["source"] = source;
                        relationship["dest"] = dest;
                        relationship["rel"] = rel;
                    } else {
                        relationship["source"] = dest;
                        relationship["dest"] = source;
                        relationship["rel"] = rel;
                    }
                    relationships.push_back(relationship);
                    source.clear();
                    rel.clear();
                    source = dest;
                    dest.clear();
                }
            }
            data["relationships"] = relationships;
            list.push_back(data);
        }
    }
    create["elements"] = list;
    cout << create.dump() << endl;

}
