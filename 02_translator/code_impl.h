#pragma once

#include "mathvm.h"
#include "defs.h"
#include "interpreted.h"

namespace mathvm
{

struct code_impl
    : Code
    , private interpreted
{
    
    code_impl();
    ~code_impl();
    
    Status* execute(vector<Var*>& vars) OVERRIDE;

public:
    void set_context(context_id_t id);
    Bytecode *dst_bc() { return &bytecode_; }


private:
    //interpreted

    Bytecode const *bytecode() OVERRIDE { return &bytecode_; } 
    bool has_local_context(size_t pos) OVERRIDE { return context_ids_.count(pos) != 0; }
    context_id_t local_context(size_t pos) OVERRIDE { return context_ids_.at(pos); }
    string const& string_const(int16_t id) OVERRIDE { return constantById(id); }

private:
    Bytecode bytecode_;
    int16_t zero_string_id_;

    typedef map<size_t, context_id_t> context_ids_t;
    context_ids_t context_ids_;
};



} //namespace mathvm