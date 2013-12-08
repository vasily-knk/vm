#pragma once

namespace mathvm
{

struct interpreted
{
    virtual ~interpreted() {};

    virtual Bytecode const *bytecode() = 0;
    virtual bool has_local_context(size_t pos) = 0;
    virtual context_id_t local_context(size_t pos) = 0;
    virtual string const& string_const(int16_t id) = 0;
};

} // namespace mathvm