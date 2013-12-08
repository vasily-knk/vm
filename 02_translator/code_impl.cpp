#include "stdafx.h"
#include "code_impl.h"
#include "interpreter.h"

namespace mathvm
{

    
code_impl::code_impl()
{
    zero_string_id_ = makeStringConstant("");
}

code_impl::~code_impl()
{

}

Status* code_impl::execute(vector<Var*>& vars)
{
    interpreter ir;
    
    try
    {
        ir.interpret(this);
        return new Status();
    }
    catch (error const &e)
    {
        return new Status(e.what());
    }
}



void code_impl::set_context(context_id_t id)
{
    context_ids_[bytecode_.length()] = id;
}

Bytecode *code_impl::dst_bc()
{
    return &bytecode_;
}


} // namespace mathvm