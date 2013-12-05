#pragma once

#include "mathvm.h"

namespace mathvm
{
typedef uint16_t context_id;
typedef uint16_t var_id;

struct context_t
{
    typedef map<string, var_id> vars_t;
    vars_t vars;
};

struct code_impl
    : Code
{
    
    code_impl();
    ~code_impl();
    
    Status* execute(vector<Var*>& vars);

    Bytecode *bytecode()
    {
        return &bytecode_;
    }

    context_t *context(context_id id)
    {
        assert(id < contexts_.size());
        return &(contexts_[id]);
    }

    context_id new_context()
    {
        contexts_.push_back(context_t());
        return contexts_.size() - 1;
    }

private:
    Bytecode bytecode_;
    vector<context_t> contexts_;
};

} //namespace mathvm