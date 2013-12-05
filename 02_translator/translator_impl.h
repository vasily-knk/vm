#pragma once

#include "ast.h"

namespace mathvm
{

struct translator_impl
    : Translator
    , AstVisitor
{
    Status* translate(const string& program, Code **code);

    typedef void ret_type;

#define VISITOR_FUNCTION(type, name) ret_type visit##type(type * node);
    FOR_NODES(VISITOR_FUNCTION)
#undef VISITOR_FUNCTION

private:
    Bytecode *bytecode() 
    {
        return &bytecode_;
    }

private:
    Bytecode bytecode_;
    VarType tos_type_;
};

} // namespace mathvm