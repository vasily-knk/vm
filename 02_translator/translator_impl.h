#pragma once

#include "ast.h"
#include "code_impl.h"

namespace mathvm
{

struct translator_impl
    : Translator
    , AstVisitor
{
    translator_impl();
    
    
    Status* translate(const string& program, Code **code);

    typedef void ret_type;

#define VISITOR_FUNCTION(type, name) ret_type visit##type(type * node);
    FOR_NODES(VISITOR_FUNCTION)
#undef VISITOR_FUNCTION

private:
    Instruction make_instruction(TokenKind op, VarType type1, VarType type2);
    std::pair<context_id, var_id> get_var_ids(AstVar const *var, bool store);

private:
    Bytecode *bytecode()
    {
        return dst_code_->bytecode();
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
    VarType tos_type_;
    code_impl* dst_code_;
    
    typedef map<Scope*, context_id> context_ids_t;
    context_ids_t context_ids_;
};

} // namespace mathvm