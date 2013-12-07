#pragma once

#include "mathvm.h"

namespace mathvm
{
typedef double d_t;
typedef int64_t i_t;
typedef const char *s_t;

typedef uint16_t context_id_t;
typedef uint16_t var_id_t;

template<typename T> T get_val(Var const &var);

template<> inline i_t get_val<i_t>(Var const &var) { return var.getIntValue   (); }
template<> inline d_t get_val<d_t>(Var const &var) { return var.getDoubleValue(); }
template<> inline s_t get_val<s_t>(Var const &var) { return var.getStringValue(); }

inline void set_val(Var &var, i_t val) { var.setIntValue   (val); }
inline void set_val(Var &var, d_t val) { var.setDoubleValue(val); }
inline void set_val(Var &var, s_t val) { var.setStringValue(val); }

inline Var create_val(i_t val) { Var var(VT_INT   , ""); var.setIntValue   (val); return var; }
inline Var create_val(d_t val) { Var var(VT_DOUBLE, ""); var.setDoubleValue(val); return var; }
inline Var create_val(s_t val) { Var var(VT_STRING, ""); var.setStringValue(val); return var; }

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

    struct error
        : std::runtime_error
    {
        error(string const &msg)
            : std::runtime_error(msg)
        { }
    };

    struct unsupported_insn 
        : error
    {
        unsupported_insn(Instruction insn)
            : error(string("Unsupported instruction: ") + bytecodeName(insn))
        { }
    };


private:
    void process_insn(Instruction insn);

    template<typename T> T read()
    {
        const T res = bytecode_.getTyped<T>(pos_);
        pos_ += sizeof(T);
        return res;
    }

    template<>
    const char* read<const char*>()
    {
        const int16_t id = read<int16_t>();
        return constantById(id).c_str();
    }

private:

    template<typename T> void process_load();
    template<typename T> void process_load_val(T val);
    template<typename T> void process_binary(Instruction insn);
    template<typename T> void process_unary(Instruction insn);

    template<> void process_binary<i_t>(Instruction insn);
    template<> void process_binary<d_t>(Instruction insn);

private:
    Bytecode bytecode_;
    typedef map<std::pair<context_id_t, var_id_t>, Var> vars_t;
    typedef std::stack<Var> stack_t;
    vars_t vars_;
    stack_t stack_;
    size_t pos_;
};

} //namespace mathvm