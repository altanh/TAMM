//
// Absyn
//  |
//  +-- TranslationUnit
//  |
//  +-- CompoundElem
//  |
//  +-- Elem
//  |    |
//  |    +-- Decl
//  |    |    |
//  |    |    +-- RangeDecl
//  |    |    +-- IndexDecl
//  |    |    +-- ArrayDecl
//  |    |    +-- ExpandDecl
//  |    |    +-- VolatileDecl
//  |    |    +-- IterationDecl
//  |    |
//  |    +-- Stmt
//  |         |
//  |         +-- AssignStmt
//  |
//  +-- Identifier
//  |
//  +-- Exp
//       |
//       +-- Parenth
//       +-- NumConst
//       +-- Array
//       +-- Multiplication
//       +-- Addition
//
//-----------------------------------------

#ifndef ABSYN_H_
#define ABSYN_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef char* string;
typedef char* Symbol;

/* Forward Declarations */

typedef struct Absyn_ *Absyn;
typedef struct Exp_ *Exp;
typedef struct Stmt_ *Stmt;
typedef struct Decl_ *Decl;
typedef struct Elem_ *Elem;
typedef struct IDList_ *IDList;
typedef struct ExpList_ *ExpList;
typedef struct ElemList_ *ElemList;
typedef struct DeclList_ *DeclList;
typedef struct Identifier_ *Identifier;
typedef struct CompoundElem_ *CompoundElem;
typedef struct CompoundElemList_ *CompoundElemList;
typedef struct TranslationUnit_ *TranslationUnit;

void *tce_malloc(int length);
string mkString(char *s);
string* mkIndexList(string *indices, int length);

Exp make_Parenth(int pos, Exp e);
Exp make_NumConst(int pos, float value);
Exp make_Addition(int pos, ExpList subexps);
Exp make_Multiplication(int pos, ExpList subexps);
Exp make_Array(int pos, Symbol name, string* indices);


Stmt make_AssignStmt(int pos, Exp lhs, Exp rhs);
Decl make_RangeDecl(int pos, Symbol name, int value);
Decl make_IndexDecl(int pos, Symbol name, Symbol rangeID);
Decl make_ArrayDecl(int pos, Symbol name, string* upperIndices, string* lowerIndices); //TODO: permute and vertex symmetry

Identifier make_Identifier(int pos, Symbol name);

Elem make_Elem_Stmt(Stmt s);
Elem make_Elem_DeclList(DeclList d);

ElemList make_ElemList(Elem head, ElemList tail);
DeclList make_DeclList(Decl head, DeclList tail); // only used for list of array decls
IDList make_IDList(Identifier head, IDList tail);
int count_IDList(IDList idl);
ExpList make_ExpList(Exp head, ExpList tail);
CompoundElem make_CompoundElem(ElemList elist);
CompoundElemList make_CompoundElemList(CompoundElem head, CompoundElemList tail);
TranslationUnit make_TranslationUnit(CompoundElemList celist);


void addTail_ElemList(Elem newtail, ElemList origList);
void addTail_DeclList(Decl newtail, DeclList origList);
void addTail_IDList(Identifier newtail, IDList origList);
void addTail_ExpList(Exp newtail, ExpList origList);
void addTail_CompoundElemList(CompoundElem newtail, CompoundElemList origList);


/* The Absyn Hierarchy */

struct Absyn_ //Root of the AST
{
    enum { is_TranslationUnit, is_CompoundElem, is_Elem, is_Identifier, is_Exp } kind;

};

struct ElemList_ //group of declarations and statements corresponding to a single input
{
    Elem head; ElemList tail;
};

struct IDList_
{
    Identifier head; IDList tail;
};

struct ExpList_
{
    Exp head; ExpList tail; // Exp is a numconst
};

struct DeclList_
{
    Decl head; DeclList tail; // Decl is ArrayDecl for now
};

struct CompoundElem_  //represents a single input enclosed in { .. }
{
    ElemList elist;
};

struct CompoundElemList_ //multiple input equations in a single file
{
    CompoundElem head; CompoundElemList tail;
};

struct TranslationUnit_
{
    CompoundElemList celist;
};

struct Identifier_
{
    int pos;
    Symbol name;
};


struct Elem_
{
    enum { is_DeclList, is_Statement } kind;
    //int pos;
    union{
        DeclList d;
        Stmt s;
    }u;
};

struct Decl_
{
    enum { is_RangeDecl, is_IndexDecl, is_ArrayDecl, is_ExpandDecl, is_VolatileDecl, is_IterationDecl } kind;
    int pos;
    union{
        struct
        {
            int value;
            Symbol name;
        }RangeDecl;
        struct
        {
            Symbol name;
            Symbol rangeID;
        }IndexDecl;
        struct
        {
            Symbol name;
            int ulen, llen;
            string* upperIndices;
            string* lowerIndices;
        }ArrayDecl;
        //TODO: ExpandDecl, IterationDecl, VolatileDecl
    }u;
};


struct Stmt_
{
    enum { is_AssignStmt } kind;
    int pos;
    union {
        struct
        {
            Exp lhs; Exp rhs; string astype;
        }AssignStmt;
    }u;
};


struct Exp_
{
    enum { is_Parenth, is_NumConst, is_ArrayRef, is_Addition, is_Multiplication } kind;
    int pos;
    float coef;
    union {
        struct
        {
            Exp exp;
        }Parenth;

        struct
        {
            float value;
        }NumConst;

        struct
        {
            Symbol name;
            int length;
            string *indices;

        }Array;

        struct
        {
            ExpList subexps;
        }Addition;

        struct
        {
            ExpList subexps;
        }Multiplication;
    }u;
};

#endif
