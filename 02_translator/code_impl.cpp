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

        pos_ = pos + 1;
        process_insn(insn);

        cout << name << endl;
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


template<>
void code_impl::process_binary<i_t>(Instruction insn)
{
    typedef i_t T;
    
    const T val1 = read<T>();
    const T val2 = read<T>();
    
    T res;

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

    stack_.push(create_val(res));
}

template<>
void code_impl::process_binary<d_t>(Instruction insn)
{
    typedef d_t T;

    const T val1 = read<T>();
    const T val2 = read<T>();

    T res;

    switch(insn)
    {
    case BC_DADD: res = val1 + val2; break;
    case BC_DSUB: res = val1 - val2; break;
    case BC_DMUL: res = val1 * val2; break;
    case BC_DDIV: res = val1 / val2; break;
    
    default: throw unsupported_insn(insn);
    }

    stack_.push(create_val(res));
}

template<typename T>
void code_impl::process_unary(Instruction insn)
{
    const T val = read<T>();

    T res;

    switch(insn)
    {
    case BC_DNEG:
    case BC_INEG: res = -val; break;
    default: throw unsupported_insn(insn);
    }
    
    stack_.push(create_val(res));
}


} // namespace mathvm