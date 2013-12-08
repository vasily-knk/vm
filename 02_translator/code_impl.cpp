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
    ir.interpret(this);

    return new Status();
}



void code_impl::set_context(context_id_t id)
{
    context_ids_[bytecode_.length()] = id;
}


} // namespace mathvm