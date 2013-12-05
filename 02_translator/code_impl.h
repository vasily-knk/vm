#pragma once

#include "mathvm.h"

namespace mathvm
{

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
private:
    Bytecode bytecode_;
};

} //namespace mathvm