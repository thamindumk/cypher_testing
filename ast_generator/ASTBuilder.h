//
// Created by thamindu on 7/24/24.
//
#include <iostream>
#include <any>
#include <string>
#include "ASTInternalNode.h"
#include "ASTLeafNoValue.h"
#include "ASTLeafValue.h"
#include "../generated/CypherBaseVisitor.h"
#ifndef AST_BUILDER_H
#define AST_BUILDER_H

using namespace std;

class CypherASTBuilder : public CypherBaseVisitor {
public:

  // finished
    std::any visitProg(CypherParser::ProgContext *ctx) override {
      auto *node = new ASTInternalNode("CYPHER");
      node->addElements(any_cast<ASTNode*>(visitOC_Cypher(ctx->oC_Cypher())));
    return static_cast<ASTNode*>(node);
    }

  // finished
    std::any visitOC_Cypher(CypherParser::OC_CypherContext *ctx) override {
      return visitOC_Statement(ctx->oC_Statement());
    }

  // finished
    std::any visitOC_Statement(CypherParser::OC_StatementContext *ctx) override {
      return visitOC_Query(ctx->oC_Query());
    }

  //finished
    std::any visitOC_Query(CypherParser::OC_QueryContext *ctx) override {
      if(ctx->oC_RegularQuery())
      {
        return visitOC_RegularQuery(ctx->oC_RegularQuery());
      }
      return visitOC_StandaloneCall(ctx->oC_StandaloneCall());
    }

  //finished
    std::any visitOC_RegularQuery(CypherParser::OC_RegularQueryContext *ctx) override {
      if(!ctx->oC_Union().empty())
      {
        auto *node = new ASTInternalNode("UNION");
        node->addElements(any_cast<ASTNode*>(visitOC_SingleQuery(ctx->oC_SingleQuery())));
        for(CypherParser::OC_UnionContext* element : ctx->oC_Union())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_Union(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_SingleQuery(ctx->oC_SingleQuery());
    }

  //finished
    std::any visitOC_Union(CypherParser::OC_UnionContext *ctx) override {
      if(ctx->ALL())
      {
        auto *node = new ASTInternalNode("ALL");
        node->addElements(any_cast<ASTNode*>(visitOC_SingleQuery(ctx->oC_SingleQuery())));
        return static_cast<ASTNode*>(node);
      }
      return visitOC_SingleQuery(ctx->oC_SingleQuery());
    }

  //finished
    std::any visitOC_SingleQuery(CypherParser::OC_SingleQueryContext *ctx) override {
      if(ctx->oC_MultiPartQuery())
      {
        return visitOC_MultiPartQuery(ctx->oC_MultiPartQuery());
      }
      return visitOC_SinglePartQuery(ctx->oC_SinglePartQuery());
    }

  //finished
    std::any visitOC_SinglePartQuery(CypherParser::OC_SinglePartQueryContext *ctx) override {
      auto *queryNode = new ASTInternalNode("SINGLE_QUERY");
      for (CypherParser::OC_ReadingClauseContext* element : ctx->oC_ReadingClause()) {
        queryNode->addElements(any_cast<ASTNode*>(visitOC_ReadingClause(element)));
      }

      for(CypherParser::OC_UpdatingClauseContext* element : ctx->oC_UpdatingClause()){
        queryNode->addElements(any_cast<ASTNode*>(visitOC_UpdatingClause(element)));
      }

      if(ctx->oC_Return())
      {
        queryNode->addElements(any_cast<ASTNode*>(visitOC_Return(ctx->oC_Return())));
      }
      return static_cast<ASTNode*>(queryNode);
    }


  //TODO: try to solve a efficient way
  //finished
    std::any visitOC_MultiPartQuery(CypherParser::OC_MultiPartQueryContext *ctx) override {
      auto *node = new ASTInternalNode("MULTIPLE_QUERY");
      for(CypherParser::OC_WithContext* withElement : ctx->oC_With())
      {
        size_t const with = withElement->getStart()->getTokenIndex();
        for(CypherParser::OC_ReadingClauseContext* element : ctx->oC_ReadingClause())
        {
          if(element->getStop()->getTokenIndex() < with)
          {
            node->addElements(any_cast<ASTNode*>(visitOC_ReadingClause(element)));
          }
        }
        for(CypherParser::OC_UpdatingClauseContext* element : ctx->oC_UpdatingClause())
        {
          if(element->getStop()->getTokenIndex() < with)
          {
            node->addElements(any_cast<ASTNode*>(visitOC_UpdatingClause(element)));
          }

        }
      }
      node->addElements(any_cast<ASTNode*>(visitOC_SinglePartQuery(ctx->oC_SinglePartQuery())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_UpdatingClause(CypherParser::OC_UpdatingClauseContext *ctx) override {

      if(ctx->oC_Create())
      {
        return visitOC_Create(ctx->oC_Create());
      }else if(ctx->oC_Delete())
      {
        return visitOC_Delete(ctx->oC_Delete());
      }else if(ctx->oC_Merge())
      {
        return visitOC_Merge(ctx->oC_Merge());
      }else if(ctx->oC_Remove())
      {
        return visitOC_Remove(ctx->oC_Remove());
      }
      return visitOC_Set(ctx->oC_Set());
    }
  //finished
    std::any visitOC_ReadingClause(CypherParser::OC_ReadingClauseContext *ctx) override {
      if(ctx->oC_Match())
      {
        return visitOC_Match(ctx->oC_Match());
      }else if(ctx->oC_Unwind())
      {
        return visitOC_Unwind(ctx->oC_Unwind());
      }else
      {
        return visitOC_InQueryCall(ctx->oC_InQueryCall());
      }
    }

  //finished
    std::any visitOC_Match(CypherParser::OC_MatchContext *ctx) override {
      auto *node = new ASTInternalNode("MATCH");
      if(ctx->OPTIONAL())
      {
        node->addElements(new ASTLeafNoValue("OPTIONAL"));
      }
      node->addElements(any_cast<ASTNode*>(visitOC_Pattern(ctx->oC_Pattern())));
      if (ctx->oC_Where()) {
        node->addElements(any_cast<ASTNode*>(visitOC_Where(ctx->oC_Where())));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Unwind(CypherParser::OC_UnwindContext *ctx) override {
      auto *node = new ASTInternalNode("UNWIND");
      auto *nodeAS = new ASTInternalNode("AS");
      nodeAS->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
      nodeAS->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
      node->addElements(nodeAS);
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Merge(CypherParser::OC_MergeContext *ctx) override {
      auto *node = new ASTInternalNode("MERGE");
      node->addElements(any_cast<ASTNode*>(visitOC_PatternPart(ctx->oC_PatternPart())));
      for(CypherParser::OC_MergeActionContext* element : ctx->oC_MergeAction())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_MergeAction(element)));
      }

      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_MergeAction(CypherParser::OC_MergeActionContext *ctx) override {
      if(ctx->CREATE())
      {
        auto *node = new ASTInternalNode("ON_CREATE");
        node->addElements(any_cast<ASTNode*>(visitOC_Set(ctx->oC_Set())));
        return static_cast<ASTNode*>(node);
      }
      auto *node = new ASTInternalNode("ON_MATCH");
      node->addElements(any_cast<ASTNode*>(visitOC_Set(ctx->oC_Set())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Create(CypherParser::OC_CreateContext *ctx) override {
      auto *node = new ASTInternalNode("CREATE");
      node->addElements(any_cast<ASTNode*>(visitOC_Pattern(ctx->oC_Pattern())));
      return static_cast<ASTNode*>(node);
    }


  //finished
    std::any visitOC_Set(CypherParser::OC_SetContext *ctx) override {
      if(ctx->oC_SetItem().size()>1)
      {
        auto *node = new ASTInternalNode("MULTIPLE_SET");
        for(CypherParser::OC_SetItemContext* element: ctx->oC_SetItem())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_SetItem(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_SetItem(ctx->oC_SetItem()[0]);
    }

  //finished
    std::any visitOC_SetItem(CypherParser::OC_SetItemContext *ctx) override {
      if(ctx->oC_PropertyExpression())
      {
        auto *node = new ASTInternalNode("SET");
        node->addElements(any_cast<ASTNode*>(visitOC_PropertyExpression(ctx->oC_PropertyExpression())));
        node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
        return static_cast<ASTNode*>(node);
      } else
      {
        if(ctx->getText().find("+=") != std::string::npos)
        {
          auto *node = new ASTInternalNode("SET_+=");
          node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
          node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
          return static_cast<ASTNode*>(node);
        }else if (ctx->getText().find("=") != std::string::npos)
        {
          auto *node = new ASTInternalNode("SET_=");
        node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
        node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
        return static_cast<ASTNode*>(node);
        }
        auto *node = new ASTInternalNode("SET");
        node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
        node->addElements(any_cast<ASTNode*>(visitOC_NodeLabels(ctx->oC_NodeLabels())));
        return static_cast<ASTNode*>(node);
      }
    }

  //finished
    std::any visitOC_Delete(CypherParser::OC_DeleteContext *ctx) override {
      auto *deleteNode = new ASTInternalNode("DELETE");
      for(CypherParser::OC_ExpressionContext* element : ctx->oC_Expression())
      {
        deleteNode->addElements(any_cast<ASTNode*>(visitOC_Expression(element)));
      }
      if(ctx->DETACH())
      {
        auto *node = new ASTInternalNode("DETACH");
        node->addElements(deleteNode);
        return static_cast<ASTNode*>(node);
      }
      return static_cast<ASTNode*>(deleteNode);
    }

  //finished
    std::any visitOC_Remove(CypherParser::OC_RemoveContext *ctx) override {
      if(ctx->oC_RemoveItem().size()>1)
      {
        auto *node = new ASTInternalNode("REMOVE_LIST");
        for(CypherParser::OC_RemoveItemContext* element : ctx->oC_RemoveItem())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_RemoveItem(element)));
        }
        return static_cast<ASTNode*>(node);
      }

      return visitOC_RemoveItem(ctx->oC_RemoveItem()[0]);
    }


  //finished
    std::any visitOC_RemoveItem(CypherParser::OC_RemoveItemContext *ctx) override {
      auto *node = new ASTInternalNode("REMOVE");
      if(ctx->oC_Variable())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
        node->addElements(any_cast<ASTNode*>(visitOC_NodeLabels(ctx->oC_NodeLabels())));
        return static_cast<ASTNode*>(node);
      }
      node->addElements(any_cast<ASTNode*>(visitOC_PropertyExpression(ctx->oC_PropertyExpression())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_InQueryCall(CypherParser::OC_InQueryCallContext *ctx) override {
      auto *node = new ASTInternalNode("CALL");
      node->addElements(any_cast<ASTNode*>(visitOC_ExplicitProcedureInvocation(ctx->oC_ExplicitProcedureInvocation())));

      if(ctx->oC_YieldItems())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_YieldItems(ctx->oC_YieldItems())));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_StandaloneCall(CypherParser::OC_StandaloneCallContext *ctx) override {
      if(ctx->oC_ExplicitProcedureInvocation())
      {
        auto *node = new ASTInternalNode("CALL");
        node->addElements(any_cast<ASTNode*>(visitOC_ExplicitProcedureInvocation(ctx->oC_ExplicitProcedureInvocation())));
        if(ctx->oC_YieldItems())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_YieldItems(ctx->oC_YieldItems())));
        }else if(ctx->getText().find('*') != std::string::npos)
        {
          auto *star = new ASTLeafNoValue("*");
          node->addElements(star);
        }
        return static_cast<ASTNode*>(node);
      }

      auto *node = new ASTInternalNode("CALL");
      node->addElements(any_cast<ASTNode*>(visitOC_ImplicitProcedureInvocation(ctx->oC_ImplicitProcedureInvocation())));
      if(ctx->oC_YieldItems())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_YieldItems(ctx->oC_YieldItems())));
      }else if(ctx->getText().find('*') != std::string::npos)
      {
        auto *star = new ASTLeafNoValue("*");
        node->addElements(star);
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_YieldItems(CypherParser::OC_YieldItemsContext *ctx) override {

      auto *node = new ASTInternalNode("YIELD_ITEMS");
      for(CypherParser::OC_YieldItemContext* element : ctx->oC_YieldItem())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_YieldItem(element)));
      }
      if(ctx->oC_Where())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Where(ctx->oC_Where())));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_YieldItem(CypherParser::OC_YieldItemContext *ctx) override {
      auto *node = new ASTInternalNode("YIELD");
      if(ctx->oC_ProcedureResultField())
      {
        auto *as = new ASTInternalNode("AS");
        as->addElements(any_cast<ASTNode*>(visitOC_ProcedureResultField(ctx->oC_ProcedureResultField())));
        as->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
        node->addElements(as);
      }
      node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_With(CypherParser::OC_WithContext *ctx) override {
      auto *node = new ASTInternalNode("WITH");
      node->addElements(any_cast<ASTNode*>(visitOC_ProjectionBody(ctx->oC_ProjectionBody())));
      if(ctx->oC_Where())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Where(ctx->oC_Where())));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Return(CypherParser::OC_ReturnContext *ctx) override {
      return visitOC_ProjectionBody(ctx->oC_ProjectionBody());
    }

  //finished
    std::any visitOC_ProjectionBody(CypherParser::OC_ProjectionBodyContext *ctx) override {
      auto *node = new ASTInternalNode("RETURN");
      if(ctx->DISTINCT())
      {
        node->addElements(new ASTLeafNoValue("DISTINCTS"));
      }
      if(ctx->oC_Limit())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Limit(ctx->oC_Limit())));
      }
      if(ctx->oC_Order())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Order(ctx->oC_Order())));
      }
      if(ctx->oC_Skip())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Skip(ctx->oC_Skip())));
      }

      node->addElements(any_cast<ASTNode*>(visitOC_ProjectionItems(ctx->oC_ProjectionItems())));

      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_ProjectionItems(CypherParser::OC_ProjectionItemsContext *ctx) override {
      auto *node = new ASTInternalNode("RETURN_BODY");
      if(ctx->getText().find('*') != std::string::npos)
      {
        node->addElements(new ASTLeafNoValue("*"));
      }
      for(CypherParser::OC_ProjectionItemContext* element : ctx->oC_ProjectionItem())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_ProjectionItem(element)));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_ProjectionItem(CypherParser::OC_ProjectionItemContext *ctx) override {
      if(ctx->oC_Variable())
      {
        auto *node = new ASTInternalNode("AS");
        node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
        node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
        return static_cast<ASTNode*>(node);
      }
      return visitOC_Expression(ctx->oC_Expression());
    }

  //finished
    std::any visitOC_Order(CypherParser::OC_OrderContext *ctx) override {
      auto *node = new ASTInternalNode("ORDERED BY");
      for(CypherParser::OC_SortItemContext* element : ctx->oC_SortItem())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_SortItem(element)));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Skip(CypherParser::OC_SkipContext *ctx) override {
      auto *node = new ASTInternalNode("SKIP");
      node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Limit(CypherParser::OC_LimitContext *ctx) override {
      auto *node = new ASTInternalNode("LIMIT");
      node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
      return static_cast<ASTNode*>(node);
    }


  //finished
    std::any visitOC_SortItem(CypherParser::OC_SortItemContext *ctx) override {
      if(ctx->ASC() or ctx->ASCENDING())
      {
        auto *node = new ASTInternalNode("ASC");
        node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
        return static_cast<ASTNode*>(node);
      }else
      {
        auto *node = new ASTInternalNode("DESC");
        node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
        return static_cast<ASTNode*>(node);
      }
    }

  //finished
    std::any visitOC_Where(CypherParser::OC_WhereContext *ctx) override {
      auto *node = new ASTInternalNode("WHERE");
      node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Pattern(CypherParser::OC_PatternContext *ctx) override {
      auto *node = new ASTInternalNode("PATTERN");
      for(CypherParser::OC_PatternPartContext* element : ctx->oC_PatternPart())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_PatternPart(element)));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_PatternPart(CypherParser::OC_PatternPartContext *ctx) override {
      if(ctx->oC_Variable())
      {
        auto *node = new ASTInternalNode("=");

        node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
        node->addElements(any_cast<ASTNode*>(visitOC_AnonymousPatternPart(ctx->oC_AnonymousPatternPart())));

        return static_cast<ASTNode*>(node);
      }
      return visitOC_AnonymousPatternPart(ctx->oC_AnonymousPatternPart());
    }

  //finished
    std::any visitOC_AnonymousPatternPart(CypherParser::OC_AnonymousPatternPartContext *ctx) override {
      return visitOC_PatternElement(ctx->oC_PatternElement());
    }

  //finished
    std::any visitOC_PatternElement(CypherParser::OC_PatternElementContext *ctx) override {
      if(ctx->oC_PatternElement())
      {
        return visitOC_PatternElement(ctx->oC_PatternElement());
      }

      if(!ctx->oC_PatternElementChain().empty())
      {
        auto *node = new ASTInternalNode("PATTERN_ELEMENTS");
        node->addElements(any_cast<ASTNode*>(visitOC_NodePattern(ctx->oC_NodePattern())));
        for(CypherParser::OC_PatternElementChainContext* element : ctx->oC_PatternElementChain())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_PatternElementChain(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_NodePattern(ctx->oC_NodePattern());
    }

  //finsihed
    std::any visitOC_RelationshipsPattern(CypherParser::OC_RelationshipsPatternContext *ctx) override {
      auto *node = new ASTInternalNode("PATTERN_ELEMENTS");
      node->addElements(any_cast<ASTNode*>(visitOC_NodePattern(ctx->oC_NodePattern())));
      for(CypherParser::OC_PatternElementChainContext* element : ctx->oC_PatternElementChain())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_PatternElementChain(element)));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_NodePattern(CypherParser::OC_NodePatternContext *ctx) override {
      auto *node = new ASTInternalNode("NODE_PATTERN");
      if(ctx->oC_Variable())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
      }
      if(ctx->oC_NodeLabels())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_NodeLabels(ctx->oC_NodeLabels())));
      }
      if(ctx->oC_Properties())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Properties(ctx->oC_Properties())));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_PatternElementChain(CypherParser::OC_PatternElementChainContext *ctx) override {
      auto *node = new ASTInternalNode("PATTERN_ELEMENT_CHAIN");
      node->addElements(any_cast<ASTNode*>(visitOC_RelationshipPattern(ctx->oC_RelationshipPattern())));
      node->addElements(any_cast<ASTNode*>(visitOC_NodePattern(ctx->oC_NodePattern())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_RelationshipPattern(CypherParser::OC_RelationshipPatternContext *ctx) override {
      auto *node = new ASTInternalNode("RELATIONSHIP_PATTTERN");
      if(ctx->oC_LeftArrowHead() && !ctx->oC_RightArrowHead())
      {
        node->addElements(new ASTLeafNoValue("LEFT_ARROW"));
      }else if(!ctx->oC_LeftArrowHead() && ctx->oC_RightArrowHead())
      {
        node->addElements(new ASTLeafNoValue("RIGHT_ARROW"));
      }else
      {
        node->addElements(new ASTLeafNoValue("UNIDIRECTION_ARROW"));
      }
      if(ctx->oC_RelationshipDetail())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_RelationshipDetail(ctx->oC_RelationshipDetail())));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_RelationshipDetail(CypherParser::OC_RelationshipDetailContext *ctx) override {
      auto *node = new ASTInternalNode("RELATIONSHIP_DETAILS");
      if(ctx->oC_Variable())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Variable(ctx->oC_Variable())));
      }
      if(ctx->oC_RelationshipTypes())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_RelationshipTypes(ctx->oC_RelationshipTypes())));
      }
      if(ctx->oC_RangeLiteral())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_RangeLiteral(ctx->oC_RangeLiteral())));
      }
      if(ctx->oC_Properties())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_Properties(ctx->oC_Properties())));
      }
      return static_cast<ASTNode*>(node); ;
    }

  //finished
    std::any visitOC_Properties(CypherParser::OC_PropertiesContext *ctx) override {
      if(ctx->oC_Parameter())
      {
        return visitOC_Parameter(ctx->oC_Parameter());
      }
      return visitOC_MapLiteral(ctx->oC_MapLiteral());
    }

  //finished
    std::any visitOC_RelationshipTypes(CypherParser::OC_RelationshipTypesContext *ctx) override {

      if(ctx->oC_RelTypeName().size()>1)
      {
        auto *node = new ASTInternalNode("RELATIONSHIP_TYPES");
        for(CypherParser::OC_RelTypeNameContext* element : ctx->oC_RelTypeName())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_RelTypeName(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_RelTypeName(ctx->oC_RelTypeName()[0]);
    }

  //finished
    std::any visitOC_NodeLabels(CypherParser::OC_NodeLabelsContext *ctx) override {
      if(ctx->oC_NodeLabel().size()>1)
      {
        auto *node = new ASTInternalNode("NODE_LABELS");
        for(CypherParser::OC_NodeLabelContext* element : ctx->oC_NodeLabel())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_NodeLabel(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_NodeLabel(ctx->oC_NodeLabel()[0]);
    }

  //finished
    std::any visitOC_NodeLabel(CypherParser::OC_NodeLabelContext *ctx) override {
      auto *node = new ASTInternalNode("NODE_LABEL");
      node->addElements(any_cast<ASTNode*>(visitOC_LabelName(ctx->oC_LabelName())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_RangeLiteral(CypherParser::OC_RangeLiteralContext *ctx) override {
      auto *node = new ASTInternalNode("RANGE");
      if(ctx->oC_IntegerLiteral().size()>1)
      {
        node->addElements(any_cast<ASTNode*>(visitOC_IntegerLiteral(ctx->oC_IntegerLiteral()[0])));
        node->addElements(any_cast<ASTNode*>(visitOC_IntegerLiteral(ctx->oC_IntegerLiteral()[1])));
        return static_cast<ASTNode*>(node);
      }
      node->addElements(any_cast<ASTNode*>(visitOC_IntegerLiteral(ctx->oC_IntegerLiteral()[0])));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_LabelName(CypherParser::OC_LabelNameContext *ctx) override {
      return visitOC_SchemaName(ctx->oC_SchemaName());
    }

  //finished
    std::any visitOC_RelTypeName(CypherParser::OC_RelTypeNameContext *ctx) override {
      return visitOC_SchemaName(ctx->oC_SchemaName());
    }

  //finished
    std::any visitOC_PropertyExpression(CypherParser::OC_PropertyExpressionContext *ctx) override {
      auto *node = new ASTInternalNode("PROPERTY");
      node->addElements(any_cast<ASTNode*>(visitOC_Atom(ctx->oC_Atom())));
      for(CypherParser::OC_PropertyLookupContext* element : ctx->oC_PropertyLookup())
      {
        node->addElements(any_cast<ASTNode*>(visitOC_PropertyLookup(element)));
      }
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Expression(CypherParser::OC_ExpressionContext *ctx) override {
      return visitOC_OrExpression(ctx->oC_OrExpression());
    }

  //finished
    std::any visitOC_OrExpression(CypherParser::OC_OrExpressionContext *ctx) override {
      if(ctx->oC_XorExpression().size()>1)
      {
        auto *node = new ASTInternalNode("OR");
        for(CypherParser::OC_XorExpressionContext* element : ctx->oC_XorExpression())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_XorExpression(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_XorExpression(ctx->oC_XorExpression()[0]);
    }

  //finished
    std::any visitOC_XorExpression(CypherParser::OC_XorExpressionContext *ctx) override {
      if(ctx->oC_AndExpression().size()>1)
      {
        auto *node = new ASTInternalNode("XOR");
        for(CypherParser::OC_AndExpressionContext* element : ctx->oC_AndExpression())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_AndExpression(element)));
        }
      }
      return visitOC_AndExpression(ctx->oC_AndExpression()[0]);
    }

  //finished
    std::any visitOC_AndExpression(CypherParser::OC_AndExpressionContext *ctx) override {
      if(ctx->oC_NotExpression().size()>1)
      {
        auto *node = new ASTInternalNode("AND");
        for(CypherParser::OC_NotExpressionContext* element : ctx->oC_NotExpression())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_NotExpression(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_NotExpression(ctx->oC_NotExpression()[0]);
    }

  //finished
    std::any visitOC_NotExpression(CypherParser::OC_NotExpressionContext *ctx) override {
      if(!ctx->NOT().empty())
      {
        if(ctx->NOT().size() % 2 != 0)
        {
          auto *node = new ASTInternalNode("NOT");
          node->addElements(any_cast<ASTNode*>(visitOC_ComparisonExpression(ctx->oC_ComparisonExpression())));
          return static_cast<ASTNode*>(node);
        }
      }
      return visitOC_ComparisonExpression(ctx->oC_ComparisonExpression());
    }


  //finished
    std::any visitOC_ComparisonExpression(CypherParser::OC_ComparisonExpressionContext *ctx) override {
      if(!ctx->oC_PartialComparisonExpression().empty())
      {
        auto *node = new ASTInternalNode("COMPARISON");
        node->addElements(any_cast<ASTNode*>(visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression())));
        for(CypherParser::OC_PartialComparisonExpressionContext* element : ctx->oC_PartialComparisonExpression())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_PartialComparisonExpression(element)));
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression());
    }

  //finished
    std::any visitOC_PartialComparisonExpression(CypherParser::OC_PartialComparisonExpressionContext *ctx) override {
      if(ctx->getText().find(">") != string::npos)
      {
        auto *node = new ASTInternalNode(">");
        node->addElements(any_cast<ASTNode*>(visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression())));
        return static_cast<ASTNode*>(node);
      }else if(ctx->getText().find("<>") != string::npos)
      {
        auto *node = new ASTInternalNode("<>");
        node->addElements(any_cast<ASTNode*>(visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression())));
        return static_cast<ASTNode*>(node);
      }else if(ctx->getText().find("=") != string::npos)
      {
        auto *node = new ASTInternalNode("==");
        node->addElements(any_cast<ASTNode*>(visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression())));
        return static_cast<ASTNode*>(node);
      }else if(ctx->getText().find("<") != string::npos)
      {
        auto *node = new ASTInternalNode("<");
        node->addElements(any_cast<ASTNode*>(visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression())));
        return static_cast<ASTNode*>(node);
      }else if(ctx->getText().find(">=") != string::npos)
      {
        auto *node = new ASTInternalNode(">=");
        node->addElements(any_cast<ASTNode*>(visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression())));
        return static_cast<ASTNode*>(node);
      }else
      {
        auto *node = new ASTInternalNode("<=");
        node->addElements(any_cast<ASTNode*>(visitOC_StringListNullPredicateExpression(ctx->oC_StringListNullPredicateExpression())));
        return static_cast<ASTNode*>(node);
      }
    }

  //finished
    std::any visitOC_StringListNullPredicateExpression(CypherParser::OC_StringListNullPredicateExpressionContext *ctx) override {
      auto *node = new ASTInternalNode("PREDICATE_EXPRESSIONS");
      if(!ctx->oC_StringPredicateExpression().empty())
      {
        auto *strPredicate = new ASTInternalNode("STRING_PREDICATES");
        for(CypherParser::OC_StringPredicateExpressionContext* element: ctx->oC_StringPredicateExpression())
        {
          strPredicate->addElements(any_cast<ASTNode*>(visitOC_StringPredicateExpression(element)));
        }
        node->addElements(strPredicate);
      }
      if(!ctx->oC_ListPredicateExpression().empty())
      {
        auto *listPredicate = new ASTInternalNode("LIST_PREDICATES");
        for(CypherParser::OC_ListPredicateExpressionContext* element: ctx->oC_ListPredicateExpression())
        {
          listPredicate->addElements(any_cast<ASTNode*>(visitOC_ListPredicateExpression(element)));
        }
        node->addElements(listPredicate);
      }
      if(!ctx->oC_NullPredicateExpression().empty())
      {
        auto *nullPredicate = new ASTInternalNode("NULL_PREDICATES");
        for(CypherParser::OC_NullPredicateExpressionContext* element: ctx->oC_NullPredicateExpression())
        {
          nullPredicate->addElements(any_cast<ASTNode*>(visitOC_NullPredicateExpression(element)));
        }
        node->addElements(nullPredicate);
      }
      if(!node->elements.empty())
      {
        return static_cast<ASTNode*>(node);
      }else
      {
        return visitOC_AddOrSubtractExpression(ctx->oC_AddOrSubtractExpression());
      }
    }

  //finished
    std::any visitOC_StringPredicateExpression(CypherParser::OC_StringPredicateExpressionContext *ctx) override {
      if(ctx->STARTS())
      {
        auto *node = new ASTInternalNode("STARTS_WITH");
        node->addElements(any_cast<ASTNode*>(visitOC_AddOrSubtractExpression(ctx->oC_AddOrSubtractExpression())));
        return static_cast<ASTNode*>(node);
      }else if(ctx->ENDS())
      {
        auto *node = new ASTInternalNode("ENDS_WITH");
        node->addElements(any_cast<ASTNode*>(visitOC_AddOrSubtractExpression(ctx->oC_AddOrSubtractExpression())));
        return static_cast<ASTNode*>(node);
      }else
      {
        auto *node = new ASTInternalNode("CONTAINS");
        node->addElements(any_cast<ASTNode*>(visitOC_AddOrSubtractExpression(ctx->oC_AddOrSubtractExpression())));
        return static_cast<ASTNode*>(node);
      }
    }


  //finished
    std::any visitOC_ListPredicateExpression(CypherParser::OC_ListPredicateExpressionContext *ctx) override {
      auto *node = new ASTInternalNode("IN");
      node->addElements(any_cast<ASTNode*>(visitOC_AddOrSubtractExpression(ctx->oC_AddOrSubtractExpression())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_NullPredicateExpression(CypherParser::OC_NullPredicateExpressionContext *ctx) override {
      if(ctx->NOT())
      {
        auto *node = new ASTLeafNoValue("IS_NOT_NULL");
        return static_cast<ASTNode*>(node);
      }
      auto *node = new ASTLeafNoValue("IS_NULL");
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_AddOrSubtractExpression(CypherParser::OC_AddOrSubtractExpressionContext *ctx) override {
      if(ctx->oC_MultiplyDivideModuloExpression().size()>1)
      {
        auto *node = new ASTInternalNode("ADD_OR_SUBSTRACT");
        node->addElements(any_cast<ASTNode*>(visitOC_MultiplyDivideModuloExpression(ctx->oC_MultiplyDivideModuloExpression(0))));

        for(int i=0; i<ctx->children.size();i++)
        {
          if(ctx->children[i]->getText() == "+")
          {
            auto *plus = new ASTInternalNode("+");
            plus->addElements(any_cast<ASTNode*>(visitOC_MultiplyDivideModuloExpression(ctx->oC_MultiplyDivideModuloExpression(i))));
            node->addElements(plus);
          }else if(ctx->children[i]->getText() == "-")
          {
            auto *minus = new ASTInternalNode("-");
            minus->addElements(any_cast<ASTNode*>(visitOC_MultiplyDivideModuloExpression(ctx->oC_MultiplyDivideModuloExpression(i))));
            node->addElements(minus);
          }
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_MultiplyDivideModuloExpression(ctx->oC_MultiplyDivideModuloExpression(0));
    }

  //finished
    std::any visitOC_MultiplyDivideModuloExpression(CypherParser::OC_MultiplyDivideModuloExpressionContext *ctx) override {
      if(ctx->oC_PowerOfExpression().size()>1)
      {
        auto *node = new ASTInternalNode("MULTIPLY_DIVID_MODULO");
        node->addElements(any_cast<ASTNode*>(visitOC_PowerOfExpression(ctx->oC_PowerOfExpression(0))));

        for(int i=0; i<ctx->children.size();i++)
        {
          if(ctx->children[i]->getText() == "*")
          {
            auto *plus = new ASTInternalNode("*");
            plus->addElements(any_cast<ASTNode*>(visitOC_PowerOfExpression(ctx->oC_PowerOfExpression(i))));
            node->addElements(plus);
          }else if(ctx->children[i]->getText() == "/")
          {
            auto *minus = new ASTInternalNode("/");
            minus->addElements(any_cast<ASTNode*>(visitOC_PowerOfExpression(ctx->oC_PowerOfExpression(i))));
            node->addElements(minus);
          }
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_PowerOfExpression(ctx->oC_PowerOfExpression(0));
    }

  //finished
    std::any visitOC_PowerOfExpression(CypherParser::OC_PowerOfExpressionContext *ctx) override {
      if(ctx->oC_UnaryAddOrSubtractExpression().size()>1)
      {
        auto *node = new ASTInternalNode("POWER_OF");
        node->addElements(any_cast<ASTNode*>(visitOC_UnaryAddOrSubtractExpression(ctx->oC_UnaryAddOrSubtractExpression(0))));

        for(int i=0; i<ctx->children.size();i++)
        {
          if(ctx->children[i]->getText() == "^")
          {
            auto *plus = new ASTInternalNode("^");
            plus->addElements(any_cast<ASTNode*>(visitOC_UnaryAddOrSubtractExpression(ctx->oC_UnaryAddOrSubtractExpression(i))));
            node->addElements(plus);
          }
        }
        return static_cast<ASTNode*>(node);
      }
      return visitOC_UnaryAddOrSubtractExpression(ctx->oC_UnaryAddOrSubtractExpression(0));
    }


  //finished
    std::any visitOC_UnaryAddOrSubtractExpression(CypherParser::OC_UnaryAddOrSubtractExpressionContext *ctx) override {
      if(ctx->getText().find("+") != string::npos)
      {
        auto *node = new ASTInternalNode("UNARY_+");
        node->addElements(any_cast<ASTNode*>(visitOC_NonArithmeticOperatorExpression(ctx->oC_NonArithmeticOperatorExpression())));
        return static_cast<ASTNode*>(node);
      }else if(ctx->getText().find("-") != string::npos)
      {
        auto *node = new ASTInternalNode("UNARY_-");
        node->addElements(any_cast<ASTNode*>(visitOC_NonArithmeticOperatorExpression(ctx->oC_NonArithmeticOperatorExpression())));
        return static_cast<ASTNode*>(node);
      }else
      {
        return visitOC_NonArithmeticOperatorExpression(ctx->oC_NonArithmeticOperatorExpression());
      }
    }

  //finished
    std::any visitOC_NonArithmeticOperatorExpression(CypherParser::OC_NonArithmeticOperatorExpressionContext *ctx) override {
      if(!ctx->oC_ListOperatorExpression().empty() | !ctx->oC_PropertyLookup().empty())
      {
        auto *node = new ASTInternalNode("NON_ARITHMETIC_OPERATOR");
        for(auto *abc : ctx->children)
        {
          std::string typeName = typeid(*abc).name();
          cout<<typeName.substr(17)<<endl;
          if(typeName.substr(17) == "OC_AtomContextE")
          {
            node->addElements(any_cast<ASTNode*>(visitOC_Atom(ctx->oC_Atom())));
          }else if(typeName.substr(17) == "OC_PropertyLookupContextE")
          {
            node->addElements(any_cast<ASTNode*>(visitOC_PropertyLookup(any_cast<>())));
          }
        }
        for(CypherParser::OC_ListOperatorExpressionContext* element : ctx->oC_ListOperatorExpression())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_ListOperatorExpression(element)));
        }
        for(CypherParser::OC_PropertyLookupContext* element : ctx->oC_PropertyLookup())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_PropertyLookup(element)));
        }
        if(ctx->oC_NodeLabels())
        {
          node->addElements(any_cast<ASTNode*>(visitOC_NodeLabels(ctx->oC_NodeLabels())));
        }

        return static_cast<ASTNode*>(node);
      }
      return visitOC_Atom(ctx->oC_Atom());
    }

  //finished
    std::any visitOC_ListOperatorExpression(CypherParser::OC_ListOperatorExpressionContext *ctx) override {
      if(ctx->oC_Expression().size() == 2)
      {
        auto *node = new ASTInternalNode("LIST_INDEX_RANGE");
        node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression(0))));
        node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression(1))));
        return static_cast<ASTNode*>(node);
      }
      auto *node = new ASTInternalNode("LIST_INDEX");
      node->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression(0))));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_PropertyLookup(CypherParser::OC_PropertyLookupContext *ctx) override {
      return visitOC_PropertyKeyName(ctx->oC_PropertyKeyName());
    }

  //finished
    std::any visitOC_Atom(CypherParser::OC_AtomContext *ctx) override {
      if(ctx->oC_Literal())
      {
        return visitOC_Literal(ctx->oC_Literal());
      }else if(ctx->oC_Parameter())
      {
        return visitOC_Parameter(ctx->oC_Parameter());
      }else if(ctx->oC_Quantifier())
      {
        return visitOC_Quantifier(ctx->oC_Quantifier());
      }else if(ctx->oC_Variable())
      {
        return visitOC_Variable(ctx->oC_Variable());
      }else if(ctx->oC_CaseExpression())
      {
        return visitOC_CaseExpression(ctx->oC_CaseExpression());
      }else if(ctx->oC_ExistentialSubquery())
      {
        return visitOC_ExistentialSubquery(ctx->oC_ExistentialSubquery());
      }else if(ctx->oC_FunctionInvocation())
      {
        return visitOC_FunctionInvocation(ctx->oC_FunctionInvocation());
      }else if(ctx->oC_ListComprehension())
      {
        return visitOC_ListComprehension(ctx->oC_ListComprehension());
      }else if(ctx->oC_ParenthesizedExpression())
      {
        return visitOC_ParenthesizedExpression(ctx->oC_ParenthesizedExpression());
      }else if(ctx->oC_PatternComprehension())
      {
        return visitOC_PatternComprehension(ctx->oC_PatternComprehension());
      }else if(ctx->oC_PatternPredicate())
      {
        return visitOC_PatternPredicate(ctx->oC_PatternPredicate());
      }else
      {
        auto *node = new ASTLeafValue("COUNT", "*");
        return static_cast<ASTNode*>(node);
      }
    }


    std::any visitOC_CaseExpression(CypherParser::OC_CaseExpressionContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_CaseAlternative(CypherParser::OC_CaseAlternativeContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_ListComprehension(CypherParser::OC_ListComprehensionContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_PatternComprehension(CypherParser::OC_PatternComprehensionContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_Quantifier(CypherParser::OC_QuantifierContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_FilterExpression(CypherParser::OC_FilterExpressionContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_PatternPredicate(CypherParser::OC_PatternPredicateContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_ParenthesizedExpression(CypherParser::OC_ParenthesizedExpressionContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_IdInColl(CypherParser::OC_IdInCollContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_FunctionInvocation(CypherParser::OC_FunctionInvocationContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_FunctionName(CypherParser::OC_FunctionNameContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_ExistentialSubquery(CypherParser::OC_ExistentialSubqueryContext *ctx) override {
      return visitChildren(ctx);
    }

  //finished
    std::any visitOC_ExplicitProcedureInvocation(CypherParser::OC_ExplicitProcedureInvocationContext *ctx) override {
      auto *node = new ASTInternalNode("EXPLICIT_PROCEDURE");
      node->addElements(any_cast<ASTNode*>(visitOC_ProcedureName(ctx->oC_ProcedureName())));
      if(ctx->oC_Expression().size()>1)
      {
        auto *arg = new ASTInternalNode("ARGUMENTS");
        for(CypherParser::OC_ExpressionContext* elements : ctx->oC_Expression())
        {
          arg->addElements(any_cast<ASTNode*>(visitOC_Expression(elements)));
        }
        node->addElements(arg);
        return static_cast<ASTNode*>(node);
      }
      return static_cast<ASTNode*>(node);
    }

    std::any visitOC_ImplicitProcedureInvocation(CypherParser::OC_ImplicitProcedureInvocationContext *ctx) override {
      return visitChildren(ctx);
    }

  //finished
    std::any visitOC_ProcedureResultField(CypherParser::OC_ProcedureResultFieldContext *ctx) override {
      auto *node = new ASTLeafValue("PROCEDURE_RESULT", any_cast<string>(visitOC_SymbolicName(ctx->oC_SymbolicName())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_ProcedureName(CypherParser::OC_ProcedureNameContext *ctx) override {
      auto *node = new ASTInternalNode("PROCEDURE");
      node->addElements(any_cast<ASTNode*>(visitOC_Namespace(ctx->oC_Namespace())));
      auto *name = new ASTLeafValue("PROCEDURE_NAME", any_cast<string>(visitOC_SymbolicName(ctx->oC_SymbolicName())));
      node->addElements(name);
      return static_cast<ASTNode*>(node);
    }


    std::any visitOC_Namespace(CypherParser::OC_NamespaceContext *ctx) override {
      auto *node = new ASTInternalNode("NAMESPACE");
      for(CypherParser::OC_SymbolicNameContext* element : ctx->oC_SymbolicName())
      {
        auto *name = new ASTLeafNoValue(any_cast<string>(visitOC_SymbolicName(element)));
        node->addElements(name);
      }
      return static_cast<ASTNode*>(node);
    }

    std::any visitOC_Variable(CypherParser::OC_VariableContext *ctx) override {
      auto *node = new ASTLeafValue("VARIABLE", any_cast<string>(visitOC_SymbolicName(ctx->oC_SymbolicName())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_Literal(CypherParser::OC_LiteralContext *ctx) override {
      if(ctx->oC_BooleanLiteral())
      {
        return visitOC_BooleanLiteral(ctx->oC_BooleanLiteral());
      }else if(ctx->oC_ListLiteral())
      {
        return visitOC_ListLiteral(ctx->oC_ListLiteral());
      }else if(ctx->oC_MapLiteral())
      {
        return visitOC_MapLiteral(ctx->oC_MapLiteral());
      }else if(ctx->oC_NumberLiteral())
      {
        return visitOC_NumberLiteral(ctx->oC_NumberLiteral());
      }else if(ctx->NULL_())
      {
        auto *node = new ASTLeafNoValue("NULL");
        return static_cast<ASTNode*>(node);
      }else
      {
        auto *node = new ASTLeafValue("STRING", ctx->getText());
        return static_cast<ASTNode*>(node);
      }
    }

    std::any visitOC_BooleanLiteral(CypherParser::OC_BooleanLiteralContext *ctx) override {
      return visitChildren(ctx);
    }
  //finished
    std::any visitOC_NumberLiteral(CypherParser::OC_NumberLiteralContext *ctx) override {
      if(ctx->oC_DoubleLiteral())
      {
        return visitOC_DoubleLiteral(ctx->oC_DoubleLiteral());
      }
      return visitOC_IntegerLiteral(ctx->oC_IntegerLiteral());
    }

  //finished
    std::any visitOC_IntegerLiteral(CypherParser::OC_IntegerLiteralContext *ctx) override {
      if(ctx->DecimalInteger())
      {
        auto *node = new ASTLeafValue("DECIMAL", ctx->getText());
        return static_cast<ASTNode*>(node);
      } else if(ctx->HexInteger())
      {
        auto *node = new ASTLeafValue("HEX", ctx->getText());
        return static_cast<ASTNode*>(node);
      }else
      {
        auto *node = new ASTLeafValue("OCTAL", ctx->getText());
        return static_cast<ASTNode*>(node);
      }

    }

  //finished
    std::any visitOC_DoubleLiteral(CypherParser::OC_DoubleLiteralContext *ctx) override {
      if(ctx->ExponentDecimalReal())
      {
        auto *node = new ASTLeafValue("EXP_DECIMAL", ctx->getText());
        return static_cast<ASTNode*>(node);
      }else
      {
        auto *node = new ASTLeafValue("REGULAR_DECIMAL", ctx->getText());
        return static_cast<ASTNode*>(node);
      }
    }

    std::any visitOC_ListLiteral(CypherParser::OC_ListLiteralContext *ctx) override {
      return visitChildren(ctx);
    }

  //finished
    std::any visitOC_MapLiteral(CypherParser::OC_MapLiteralContext *ctx) override {
      auto *node = new ASTInternalNode("PROPERIES_MAP");
      for(int i=0; i<ctx->oC_PropertyKeyName().size();i++)
      {
        auto *propNode = new ASTInternalNode("PROPERTY");
        propNode->addElements(any_cast<ASTNode*>(visitOC_PropertyKeyName(ctx->oC_PropertyKeyName()[i])));
        propNode->addElements(any_cast<ASTNode*>(visitOC_Expression(ctx->oC_Expression()[i])));
        node->addElements(propNode);
      }

      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_PropertyKeyName(CypherParser::OC_PropertyKeyNameContext *ctx) override {
      return visitOC_SchemaName(ctx->oC_SchemaName());
    }


    std::any visitOC_Parameter(CypherParser::OC_ParameterContext *ctx) override {
      return visitChildren(ctx);
    }

  //finished
    std::any visitOC_SchemaName(CypherParser::OC_SchemaNameContext *ctx) override {
      if(ctx->oC_ReservedWord())
      {
        return visitOC_ReservedWord(ctx->oC_ReservedWord());
      }
      auto *node = new ASTLeafValue("SYMBOLIC_WORD",any_cast<string>(visitOC_SymbolicName(ctx->oC_SymbolicName())));
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_ReservedWord(CypherParser::OC_ReservedWordContext *ctx) override {
      auto *node = new ASTLeafValue("RESERVED_WORD", ctx->getText());
      return static_cast<ASTNode*>(node);
    }

  //finished
    std::any visitOC_SymbolicName(CypherParser::OC_SymbolicNameContext *ctx) override {
      return ctx->getText();
    }

    std::any visitOC_LeftArrowHead(CypherParser::OC_LeftArrowHeadContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_RightArrowHead(CypherParser::OC_RightArrowHeadContext *ctx) override {
      return visitChildren(ctx);
    }

    std::any visitOC_Dash(CypherParser::OC_DashContext *ctx) override {
      return visitChildren(ctx);
    }
};


#endif //AST_BUILDER_H
