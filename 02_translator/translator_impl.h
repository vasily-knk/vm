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
    Instruction make_instruction(TokenKind op, VarType type1, VarType type2);

private:
    Bytecode *bytecode() 
    {
        return &bytecode_;
    }

    struct error
        : std::runtime_error
    {
        error(string const &msg)
            : std::runtime_error(msg)
        {

        }
    };

private:
    Bytecode bytecode_;
    VarType tos_type_;
    Code* dst_code_;
};

} // namespace mathvm