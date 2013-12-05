#include "stdafx.h"
#include "code_impl.h"

namespace mathvm
{

code_impl::code_impl()
{

}

code_impl::~code_impl()
{

}

Status* code_impl::execute( vector<Var*>& vars )
{
    bytecode_.dump(cout);
    return new Status();
}

} // namespace mathvm