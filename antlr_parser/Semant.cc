//------------------------------------------------------------------------------
// Copyright (C) 2016, Pacific Northwest National Laboratory
// This software is subject to copyright protection under the laws of the
// United States and other countries
//
// All rights in this computer software are reserved by the
// Pacific Northwest National Laboratory (PNNL)
// Operated by Battelle for the U.S. Department of Energy
//
//------------------------------------------------------------------------------

#include "Util.h"
#include "Semant.h"
#include "Error.h"
#include "Entry.h"
#include <set>
#include <cassert>

namespace tamm {

void type_check(const CompilationUnit* const root, SymbolTable* const context) {
    for (auto &ce: root->celist)
        check_CompoundElement(ce, context);
}

void check_CompoundElement(const CompoundElement* const ce, SymbolTable* const context) {
    const ElementList* const element_list = ce->elist;
    for (auto &elem: element_list->elist) 
        check_Element(elem, context);
}

void check_Element(Element* const element, SymbolTable* const context) {    
     if (element == nullptr) return; /// TODO: Can this happen ?

     //if (element->getElementType() == Element::kDeclaration);
     if (DeclarationList* const dl = dynamic_cast<DeclarationList*>(element))
            check_DeclarationList(dl, context);
     else if (Statement* const statement = dynamic_cast<Statement*>(element))
            check_Statement(statement, context);
     else ;
//             std::cerr << "Not a Declaration or Statement!\n";
//             std::exit(EXIT_FAILURE);
}

void check_DeclarationList(const DeclarationList* const decllist, SymbolTable* const context) {
    for (auto &decl: decllist->dlist)
        check_Declaration(decl, context);
}

void check_Statement(Statement* const statement, SymbolTable* const context) {
    if (AssignStatement* const as = dynamic_cast<AssignStatement*>(statement)) 
        check_AssignStatement(as,context);
    else ; //error- cannot happen
}


bool check_tensor_usage(const index_list& def, const index_list& use){
    assert(def.size() == use.size());
    for (int i = 0; i<def.size(); i++)
        if (def[i]!="N") if(def[i]!=use[i]) return false;
    return true;
}

void check_index_reference(Identifier* const index, SymbolTable* const context) {
    if (context->get(index->name) == nullptr){
        std::string index_error = "Index " + index->name + " is not defined";
        Error(index->line, index->position, index_error);
    }
}

bool check_duplicate_indices(const std::vector<Identifier*>& indices){
    std::set<std::string> unique_indices;
    for(auto &index: indices) unique_indices.insert(index->name);
    if (indices.size() > unique_indices.size()) return true;
    return false; 
}

void check_array_reference(Array* const aref, SymbolTable* const context) {
    const std::string tensor_name = aref->tensor_name->name;
    const Entry* const tensor_entry = context->get(tensor_name);
        if (tensor_entry == nullptr){
            const std::string array_decl_error = "Tensor " + tensor_name + " is not defined";
            Error(aref->line, aref->tensor_name->position, array_decl_error);
        }
    for (auto &index: aref->indices) check_index_reference(index, context);

    const bool duplicate_indices = check_duplicate_indices(aref->indices);

    //Check for repetitive indices in an array reference
    if (duplicate_indices){
        const std::string aref_text = aref->getText();
        const std::string duplicate_index_error = "Repetitive index in tensor reference: " + aref_text ;
        Error(aref->line, aref->position, duplicate_index_error);
    }

    //Check if it conforms to the array declaration. 
    Type* const tensor_type = tensor_entry->type;
    index_list index_types; // Specified in Tensor Declaration.

    if (TensorType* const t = dynamic_cast<TensorType*>(tensor_type)){
        for (auto &x: t->upper_indices) index_types.push_back(x->name);
        for (auto &x: t->lower_indices) index_types.push_back(x->name);
    }
    else ; ////Error: tensor_type cannot be of some other type

    index_list usage_indices = get_indices_from_identifiers(aref->indices);
    index_list usage_types;
    for (auto &x: usage_indices){
        const Entry* const entry = context->get(x);
        Type* const rt = entry->type;
        if (IndexType* const it = dynamic_cast<IndexType*>(rt))
            usage_types.push_back(it->range_name->name);
        else ; ///Error: This should not happen.
    }

    const bool tensor_use = check_tensor_usage(index_types,usage_types);
    if (!tensor_use){    
        const std::string aref_text = aref->getText();
        const std::string tensor_usage_error = "Tensor reference " + aref_text 
            + " must have index structure of " + get_index_list_as_string(index_types) + 
            " but is used as " + get_index_list_as_string(usage_types);
    Error(aref->line, aref->position, tensor_usage_error);
    }
    
}

void check_AssignStatement(const AssignStatement* const statement, SymbolTable* const context) {         
        Array* const lhs = statement->lhs;
        Expression* const rhs  = statement->rhs;

        std::vector<Array*> rhs_arefs;
        get_array_refs_from_expression(rhs, rhs_arefs);

        check_array_reference(lhs, context);
        for (auto &ra: rhs_arefs) check_array_reference(ra, context);

        check_expression(rhs, context);

        index_list non_summ_indices = get_non_summation_indices_from_expression(rhs_arefs);
        index_list lhs_indices = get_indices_from_identifiers(lhs->indices);

        if (!compare_index_lists(lhs_indices, non_summ_indices)) {
        // print_index_list(lhs_indices);
        // print_index_list(non_summ_indices);
            std::string error_msg = "LHS and RHS of assignment must have equal (non-summation) index sets";
            Error(statement->line,0,error_msg);
        }

///    TODO: If lhs array ref occurs on rhs (a=a+b*c) complain saying that a+=b*c must be used instead
//    if (exists_index(rhs_arefs->list,rhs_arefs->length,lhs_aref->list[0])){
//        std::cerr << "Error at line " << s->u.AssignStmt.lhs->lineno << ": array " << lhs_aref->list[0] << " cannot be assigned after being previously referenced\n";
//        std::exit(EXIT_FAILURE);
//    }
}


void check_range(const Identifier* const range_var, SymbolTable* const context) {

    const std::string range_name = range_var->name;
        /// RANGE VARIABLE MAY or MAY NOT BE DEFINED
        //   if (context->get(range_name) == nullptr) {
        //         std::string range_var_error = "Range variable " + range_name + " is not defined";
        //         Error(idecl->line, idecl->range_id->position, range_var_error);
        //    }
    const std::vector<std::string> allowed_ranges{"O", "V", "N"};
    if (std::find(allowed_ranges.begin(), allowed_ranges.end(), range_name) == allowed_ranges.end()) {
        std::string range_error = "Range " + range_name + " is not supported. Can only be one of O, V, N";
        Error(range_var->line, range_var->position, range_error);
    }
}

void check_Declaration(Declaration* const declaration, SymbolTable* const context) {
if (RangeDeclaration* const rdecl = dynamic_cast<RangeDeclaration*>(declaration)) {
           const std::string range_var = rdecl->name->name;
           if (context->get(range_var) != nullptr) {
                std::string range_decl_error = "Range " + range_var + " is already defined";
                Error(rdecl->line, rdecl->name->position, range_decl_error);
           }
           assert(rdecl->value > 0); /// This should be fixed in ast builder.
           context->put(range_var, new Entry(new RangeType(rdecl->value)));
}

else if (IndexDeclaration* const idecl = dynamic_cast<IndexDeclaration*>(declaration)) {
           const std::string index_name = idecl->index_name->name;
           const std::string range_name = idecl->range_id->name;
           if (context->get(index_name) != nullptr) {
                std::string index_decl_error = "Index " + index_name + " is already defined";
                Error(idecl->line, idecl->index_name->position, index_decl_error);
           }

           check_range(idecl->range_id, context);
           context->put(index_name, new Entry(new IndexType(idecl->range_id)));
}
else if (ArrayDeclaration* const adecl = dynamic_cast<ArrayDeclaration*>(declaration)) {

        const std::string tensor_name = adecl->tensor_name->name;
        if (context->get(tensor_name) != nullptr){
            std::string array_decl_error = "Tensor " + tensor_name + " is already defined";
            Error(adecl->line, adecl->tensor_name->position, array_decl_error);
        }

        for (auto &upper: adecl->upper_indices) check_range(upper, context);
        for (auto &lower: adecl->lower_indices) check_range(lower, context);

        context->put(tensor_name, new Entry(new TensorType(adecl->upper_indices,adecl->lower_indices)));
    }
}


void check_expression(Expression* const exp, SymbolTable* const context) {
        // case Exp::is_Parenth: {
        //     check_Exp(exp->u.Parenth.exp, symtab);
        // }
    if (Array* const a = dynamic_cast<Array*>(exp)) {
           ;
    }    
    
    else if (Addition* const add = dynamic_cast<Addition*>(exp)) ;       
    else if (Multiplication* const mult = dynamic_cast<Multiplication*>(exp)); 
 
        //     break;
        // case Exp::is_Addition: {
        //     check_ExpList(exp->u.Addition.subexps, symtab);
        //     inames = getIndices(exp);
        //     el = exp->u.Addition.subexps;
        //     while (el != nullptr) {
        //         tamm_string_array op_inames = getIndices(el->head);
        //         if (!compare_index_lists(inames, op_inames)) {
        //             std::cerr << "Error at line " << clno
        //                       << ": subexpressions of an addition must have equal index sets\n";
        //             std::exit(EXIT_FAILURE);
        //         }

        //         el = el->tail;
        //     }
        //     break;
        //     case Exp::is_Multiplication:
        //         check_ExpList(exp->u.Multiplication.subexps, symtab);
        //     el = exp->u.Multiplication.subexps;
        //     tamm_string_array all_ind;

        //     while (el != nullptr) {
        //         tamm_string_array se = getIndices(el->head);
        //         for (auto i: se)
        //             all_ind.push_back(i);
        //         el = el->tail;
        //     }

        //     tamm_string_array uind;
        //     for (auto i: all_ind) {
        //         if (!exists_index(uind, i))
        //             uind.push_back(i);
        //     }

        //     for (auto i:uind) {
        //         if (count_index(all_ind, i) > 2) {
        //             std::cerr << "Error at line " << clno << ": summation index " << i <<
        //                       " must occur exactly twice in a multiplication\n";
        //             std::exit(EXIT_FAILURE);
        //         }
        //     }
        // }
        //     break;
        // default: {
        //     std::cerr << "Not a valid Expression!\n";
        //     std::exit(EXIT_FAILURE);
        // }
    }

}