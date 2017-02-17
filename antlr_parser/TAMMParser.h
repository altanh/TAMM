
// Generated from TAMM.g4 by ANTLR 4.6

#pragma once


#include "antlr4-runtime.h"




class  TAMMParser : public antlr4::Parser {
public:
  enum {
    RANGE = 1, INDEX = 2, ARRAY = 3, SCALAR = 4, EXPAND = 5, VOLATILE = 6, 
    ITERATION = 7, PLUS = 8, MINUS = 9, TIMES = 10, EQUALS = 11, TIMESEQUAL = 12, 
    PLUSEQUAL = 13, MINUSEQUAL = 14, LPAREN = 15, RPAREN = 16, LBRACE = 17, 
    RBRACE = 18, LBRACKET = 19, RBRACKET = 20, COMMA = 21, COLON = 22, SEMI = 23, 
    ID = 24, ICONST = 25, FRAC = 26, FCONST = 27, Whitespace = 28, Newline = 29, 
    BlockComment = 30, LineComment = 31
  };

  enum {
    RuleTranslation_unit = 0, RuleCompound_element_list = 1, RuleCompound_element = 2, 
    RuleElement_list = 3, RuleElement = 4, RuleDeclaration = 5, RuleScalar_declaration = 6, 
    RuleId_list_opt = 7, RuleId_list = 8, RuleNum_list = 9, RuleIdentifier = 10, 
    RuleNumerical_constant = 11, RuleRange_declaration = 12, RuleIndex_declaration = 13, 
    RuleArray_declaration = 14, RuleArray_structure = 15, RuleArray_structure_list = 16, 
    RuleStatement = 17, RuleAssignment_statement = 18, RuleAssignment_operator = 19, 
    RuleUnary_expression = 20, RuleArray_reference = 21, RulePlusORminus = 22, 
    RuleExpression = 23, RuleMultiplicative_expression = 24
  };

  TAMMParser(antlr4::TokenStream *input);
  ~TAMMParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class Translation_unitContext;
  class Compound_element_listContext;
  class Compound_elementContext;
  class Element_listContext;
  class ElementContext;
  class DeclarationContext;
  class Scalar_declarationContext;
  class Id_list_optContext;
  class Id_listContext;
  class Num_listContext;
  class IdentifierContext;
  class Numerical_constantContext;
  class Range_declarationContext;
  class Index_declarationContext;
  class Array_declarationContext;
  class Array_structureContext;
  class Array_structure_listContext;
  class StatementContext;
  class Assignment_statementContext;
  class Assignment_operatorContext;
  class Unary_expressionContext;
  class Array_referenceContext;
  class PlusORminusContext;
  class ExpressionContext;
  class Multiplicative_expressionContext; 

  class  Translation_unitContext : public antlr4::ParserRuleContext {
  public:
    Translation_unitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Compound_element_listContext *compound_element_list();
    antlr4::tree::TerminalNode *EOF();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Translation_unitContext* translation_unit();

  class  Compound_element_listContext : public antlr4::ParserRuleContext {
  public:
    Compound_element_listContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Compound_elementContext *> compound_element();
    Compound_elementContext* compound_element(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Compound_element_listContext* compound_element_list();

  class  Compound_elementContext : public antlr4::ParserRuleContext {
  public:
    Compound_elementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *LBRACE();
    Element_listContext *element_list();
    antlr4::tree::TerminalNode *RBRACE();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Compound_elementContext* compound_element();

  class  Element_listContext : public antlr4::ParserRuleContext {
  public:
    Element_listContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ElementContext *> element();
    ElementContext* element(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Element_listContext* element_list();

  class  ElementContext : public antlr4::ParserRuleContext {
  public:
    ElementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DeclarationContext *declaration();
    StatementContext *statement();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ElementContext* element();

  class  DeclarationContext : public antlr4::ParserRuleContext {
  public:
    DeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Range_declarationContext *range_declaration();
    Index_declarationContext *index_declaration();
    Scalar_declarationContext *scalar_declaration();
    Array_declarationContext *array_declaration();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeclarationContext* declaration();

  class  Scalar_declarationContext : public antlr4::ParserRuleContext {
  public:
    Scalar_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SCALAR();
    antlr4::tree::TerminalNode *ID();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Scalar_declarationContext* scalar_declaration();

  class  Id_list_optContext : public antlr4::ParserRuleContext {
  public:
    Id_list_optContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Id_listContext *id_list();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Id_list_optContext* id_list_opt();

  class  Id_listContext : public antlr4::ParserRuleContext {
  public:
    Id_listContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<IdentifierContext *> identifier();
    IdentifierContext* identifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Id_listContext* id_list();

  class  Num_listContext : public antlr4::ParserRuleContext {
  public:
    Num_listContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Numerical_constantContext *> numerical_constant();
    Numerical_constantContext* numerical_constant(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Num_listContext* num_list();

  class  IdentifierContext : public antlr4::ParserRuleContext {
  public:
    IdentifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdentifierContext* identifier();

  class  Numerical_constantContext : public antlr4::ParserRuleContext {
  public:
    Numerical_constantContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ICONST();
    antlr4::tree::TerminalNode *FCONST();
    antlr4::tree::TerminalNode *FRAC();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Numerical_constantContext* numerical_constant();

  class  Range_declarationContext : public antlr4::ParserRuleContext {
  public:
    Range_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RANGE();
    Id_listContext *id_list();
    antlr4::tree::TerminalNode *EQUALS();
    Numerical_constantContext *numerical_constant();
    antlr4::tree::TerminalNode *SEMI();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Range_declarationContext* range_declaration();

  class  Index_declarationContext : public antlr4::ParserRuleContext {
  public:
    Index_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INDEX();
    Id_listContext *id_list();
    antlr4::tree::TerminalNode *EQUALS();
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *SEMI();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Index_declarationContext* index_declaration();

  class  Array_declarationContext : public antlr4::ParserRuleContext {
  public:
    Array_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ARRAY();
    Array_structure_listContext *array_structure_list();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *COLON();
    IdentifierContext *identifier();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Array_declarationContext* array_declaration();

  class  Array_structureContext : public antlr4::ParserRuleContext {
  public:
    Array_structureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    std::vector<antlr4::tree::TerminalNode *> LBRACKET();
    antlr4::tree::TerminalNode* LBRACKET(size_t i);
    std::vector<Id_list_optContext *> id_list_opt();
    Id_list_optContext* id_list_opt(size_t i);
    std::vector<antlr4::tree::TerminalNode *> RBRACKET();
    antlr4::tree::TerminalNode* RBRACKET(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Array_structureContext* array_structure();

  class  Array_structure_listContext : public antlr4::ParserRuleContext {
  public:
    Array_structure_listContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Array_structureContext *> array_structure();
    Array_structureContext* array_structure(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Array_structure_listContext* array_structure_list();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Assignment_statementContext *assignment_statement();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  Assignment_statementContext : public antlr4::ParserRuleContext {
  public:
    Assignment_statementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Array_referenceContext *array_reference();
    Assignment_operatorContext *assignment_operator();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *SEMI();
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *COLON();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Assignment_statementContext* assignment_statement();

  class  Assignment_operatorContext : public antlr4::ParserRuleContext {
  public:
    Assignment_operatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EQUALS();
    antlr4::tree::TerminalNode *TIMESEQUAL();
    antlr4::tree::TerminalNode *PLUSEQUAL();
    antlr4::tree::TerminalNode *MINUSEQUAL();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Assignment_operatorContext* assignment_operator();

  class  Unary_expressionContext : public antlr4::ParserRuleContext {
  public:
    Unary_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Numerical_constantContext *numerical_constant();
    Array_referenceContext *array_reference();
    antlr4::tree::TerminalNode *LPAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RPAREN();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Unary_expressionContext* unary_expression();

  class  Array_referenceContext : public antlr4::ParserRuleContext {
  public:
    Array_referenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *LBRACKET();
    Id_listContext *id_list();
    antlr4::tree::TerminalNode *RBRACKET();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Array_referenceContext* array_reference();

  class  PlusORminusContext : public antlr4::ParserRuleContext {
  public:
    PlusORminusContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PLUS();
    antlr4::tree::TerminalNode *MINUS();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PlusORminusContext* plusORminus();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Multiplicative_expressionContext *> multiplicative_expression();
    Multiplicative_expressionContext* multiplicative_expression(size_t i);
    std::vector<PlusORminusContext *> plusORminus();
    PlusORminusContext* plusORminus(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();

  class  Multiplicative_expressionContext : public antlr4::ParserRuleContext {
  public:
    Multiplicative_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Unary_expressionContext *> unary_expression();
    Unary_expressionContext* unary_expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> TIMES();
    antlr4::tree::TerminalNode* TIMES(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Multiplicative_expressionContext* multiplicative_expression();


private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

