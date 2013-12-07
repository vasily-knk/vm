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

Status* code_impl::execute(vector<Var*>& vars)
{
    for (size_t pos = 0; pos < bytecode_.length();)
    {
        size_t length;
        const Instruction insn = bytecode_.getInsn(pos);
        const char* name = bytecodeName(insn, &length);

        if (context_ids_.count(pos))
            context_id_ = context_ids_.at(pos);

        pos_ = pos + 1;
        process_insn(insn);

        //cout << name << endl;
        pos += length;
    }

    return new Status();
}

void code_impl::process_insn(Instruction insn)
{
    switch(insn)
    {
    case BC_DLOAD: process_load<d_t>(); break;
    case BC_ILOAD: process_load<i_t>(); break;
    case BC_SLOAD: process_load<s_t>(); break;
    case BC_DLOAD0: process_load_val<d_t>(0); break;
    case BC_ILOAD0: process_load_val<i_t>(0); break;
    //case BC_SLOAD0: process_load_val<s_t>(""); break;
    case BC_DLOAD1: process_load_val<d_t>(1); break;
    case BC_ILOAD1: process_load_val<i_t>(1); break;
    case BC_DLOADM1: process_load_val<d_t>(-1); break;
    case BC_ILOADM1: process_load_val<i_t>(-1); break;

    case BC_DADD: process_binary<d_t>(insn); break;
    case BC_IADD: process_binary<i_t>(insn); break;
    case BC_DSUB: process_binary<d_t>(insn); break;
    case BC_ISUB: process_binary<i_t>(insn); break;
    case BC_DMUL: process_binary<d_t>(insn); break;
    case BC_IMUL: process_binary<i_t>(insn); break;
    case BC_DDIV: process_binary<d_t>(insn); break;
    case BC_IDIV: process_binary<i_t>(insn); break;
    case BC_IMOD: process_binary<i_t>(insn); break;

    case BC_DNEG: process_unary<d_t>(insn); break;
    case BC_INEG: process_unary<i_t>(insn); break;

    case BC_IAOR: process_binary<i_t>(insn); break;
    case BC_IAAND: process_binary<i_t>(insn); break;
    case BC_IAXOR: process_binary<i_t>(insn); break;

    case BC_IPRINT: process_print<i_t>(); break;
    case BC_DPRINT: process_print<d_t>(); break;
    case BC_SPRINT: process_print<s_t>(); break;

    case BC_POP: stack_.pop(); break;

    case BC_LOADDVAR: process_load_local<d_t>(); break;
    case BC_LOADIVAR: process_load_local<i_t>(); break;
    case BC_LOADSVAR: process_load_local<s_t>(); break;
    case BC_STOREDVAR: process_store_local<d_t>(); break;
    case BC_STOREIVAR: process_store_local<i_t>(); break;
    case BC_STORESVAR: process_store_local<s_t>(); break;

    case BC_LOADCTXDVAR: process_load_ctx<d_t>(); break;
    case BC_LOADCTXIVAR: process_load_ctx<i_t>(); break;
    case BC_LOADCTXSVAR: process_load_ctx<s_t>(); break;
    case BC_STORECTXDVAR: process_store_ctx<d_t>(); break;
    case BC_STORECTXIVAR: process_store_ctx<i_t>(); break;
    case BC_STORECTXSVAR: process_store_ctx<s_t>(); break;


    default: throw unsupported_insn(insn);
    }
}

template<typename T>
void code_impl::process_load()
{
    const T val = read<T>();
    stack_.push(create_val(val));
}


template<typename T>
void code_impl::process_load_val(T val)
{
    stack_.push(create_val(val));
}


template<typename T>
void code_impl::process_binary(Instruction insn)
{
    const T val1 = get_val<T>(stack_.top());
    stack_.pop();
    const T val2 = get_val<T>(stack_.top());
    stack_.pop();

    const T res = process_binary_impl<T>(insn, val1, val2);
    stack_.push(create_val(res));
}

i_t code_impl::process_binary_impl(Instruction insn, i_t val1, i_t val2)
{
    i_t res;

    switch(insn)
    {
    case BC_IADD: res = val1 + val2; break;
    case BC_ISUB: res = val1 - val2; break;
    case BC_IMUL: res = val1 * val2; break;
    case BC_IDIV: res = val1 / val2; break;

    case BC_IMOD: res = val1 % val2; break;

    case BC_IAOR : res = val1 | val2; break;
    case BC_IAAND: res = val1 & val2; break;
    case BC_IAXOR: res = val1 ^ val2; break;

    default: throw unsupported_insn(insn);
    }

    return res;
}

d_t code_impl::process_binary_impl(Instruction insn, d_t val1, d_t val2)
{
    d_t res;

    switch(insn)
    {
    case BC_DADD: res = val1 + val2; break;
    case BC_DSUB: res = val1 - val2; break;
    case BC_DMUL: res = val1 * val2; break;
    case BC_DDIV: res = val1 / val2; break;
    
    default: throw unsupported_insn(insn);
    }

    return res;
}

template<typename T>
void code_impl::process_unary(Instruction insn)
{
    const T val = get_val<T>(stack_.top());
    stack_.pop();

    T res;

    switch(insn)
    {
    case BC_DNEG:
    case BC_INEG: res = -val; break;
    default: throw unsupported_insn(insn);
    }
    
    stack_.push(create_val(res));
}

template<typename T>
void code_impl::process_print()
{
    cout << get_val<T>(stack_.top()) << endl;
    stack_.pop();
}



template<typename T>
void mathvm::code_impl::process_load_local()
{
    const var_id_t var_id = read<var_id_t>();
    process_load_var<T>(context_id_, var_id);
}

template<typename T>
void code_impl::process_load_ctx()
{
    const context_id_t context_id = read<context_id_t>();
    const var_id_t var_id = read<var_id_t>();
    process_load_var<T>(context_id, var_id);
}

template<typename T>
void code_impl::process_load_var(context_id_t context_id, var_id_t var_id)
{
    Var const &var = vars_.at(std::make_pair(context_id, var_id));
    stack_.push(var);
}

template<typename T>
void mathvm::code_impl::process_store_local()
{
    const var_id_t var_id = read<var_id_t>();
    process_store_var<T>(context_id_, var_id);
}

template<typename T>
void code_impl::process_store_ctx()
{
    const context_id_t context_id = read<context_id_t>();
    const var_id_t var_id = read<var_id_t>();
    process_store_var<T>(context_id, var_id);
}

template<typename T>
void code_impl::process_store_var(context_id_t context_id, var_id_t var_id)
{
    vars_.insert(std::make_pair(std::make_pair(context_id, var_id), stack_.top()));
    stack_.pop();
}

void code_impl::set_context(context_id_t id)
{
    context_ids_[bytecode_.length()] = id;
}


} // namespace mathvm