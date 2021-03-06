#pragma once

#include "interpreted.h"

namespace mathvm
{

struct interpreter
{
    void interpret(interpreted *code);

private:
    void process_func();

private:
    void process_insn(Instruction insn);

    template<typename T> void process_load();
    template<typename T> void process_load_val(T val);
    template<typename T> void process_binary(Instruction insn);
    template<typename T> void process_unary(Instruction insn);

    template<typename T> T process_binary_impl(Instruction insn, T val1, T val2);
    i_t process_binary_impl(Instruction insn, i_t val_1, i_t val_2);
    d_t process_binary_impl(Instruction insn, d_t val_1, d_t val_2);

    template<typename T> void process_print();
    template<typename T> void process_load_local();
    template<typename T> void process_load_ctx();
    template<typename T> void process_load_var(context_id_t context_id, var_id_t var_id);
    template<typename T> void process_store_local();
    template<typename T> void process_store_ctx();
    template<typename T> void process_store_var(context_id_t context_id, var_id_t var_id);

    void process_call();

private:
    template<typename T> T read()
    {
        const T res = func_->bytecode()->getTyped<T>(pos_);
        pos_ += sizeof(T);
        return res;
    }

    template<>
    const char* read<const char*>()
    {
        const int16_t id = read<int16_t>();
        return code_->get_string_const(id).c_str();
    }

private:
    interpreted *code_;
    function *func_;

    typedef map<std::pair<context_id_t, var_id_t>, Var> vars_t;
    vars_t vars_;

    typedef std::stack<Var> stack_t;
    stack_t stack_;

    size_t pos_;
    context_id_t context_id_;

    bool return_;
};

} // namespace mathvm