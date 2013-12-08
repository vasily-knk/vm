#pragma once

#include "mathvm.h"
#include "defs.h"
#include "function.h"
#include "interpreted.h"

namespace mathvm
{

struct code_impl
    : Code
    , private interpreted
    , private function
{
    
    code_impl();
    ~code_impl();
    
    Status* execute(vector<Var*>& vars) OVERRIDE;

public:
    void set_context(context_id_t id);
    Bytecode *dst_bc() { return &bytecode_; }


private:
    //interpreted

    function_id_t get_top_function() OVERRIDE { return 0; }
    function_id_t num_functions() OVERRIDE  { return 1; }
    function *get_function(function_id_t id) OVERRIDE { assert (id == 0); return this; }
    string const& get_string_const(int16_t id) OVERRIDE { return constantById(id); }

private:
    //function
    Bytecode const *bytecode() OVERRIDE { return &bytecode_; } 
    bool has_local_context(size_t pos) OVERRIDE { return context_ids_.count(pos) != 0; }
    context_id_t local_context(size_t pos) OVERRIDE { return context_ids_.at(pos); }



private:
    Bytecode bytecode_;

    typedef map<size_t, context_id_t> context_ids_t;
    context_ids_t context_ids_;

    int16_t zero_string_id_;
};



} //namespace mathvm