#include "stdafx.h"
#include "translator_impl.h"
#include "code_impl.h"
#include "parser.h"

namespace mathvm
{

translator_impl::ret_type translator_impl::visitDoubleLiteralNode(DoubleLiteralNode* node)
{
    bytecode()->addInsn(BC_DLOAD);
    bytecode()->addDouble(node->literal()); 
    tos_type_ = VT_DOUBLE;
}

translator_impl::ret_type translator_impl::visitWhileNode( WhileNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitForNode( ForNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitFunctionNode(FunctionNode* node)
{
    node->body()->visit(this);
}

translator_impl::ret_type translator_impl::visitBlockNode(BlockNode* node)
{
    current_scope_ = node->scope();    
    for (uint32_t i = 0; i < node->nodes(); ++i)
    {
        tos_type_ = VT_VOID;
        node->nodeAt(i)->visit(this);
        if (tos_type_ != VT_VOID)
            bytecode()->addInsn(BC_DUMP);
    }
    tos_type_ = VT_VOID;
}


translator_impl::ret_type translator_impl::visitIfNode( IfNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitNativeCallNode( NativeCallNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitPrintNode(PrintNode* node)
{
    for (uint32_t i = 0; i < node->operands(); ++i)
    {
        node->operandAt(i)->visit(this);
        Instruction insn;
        switch(tos_type_)
        {
        case VT_INT   : insn = BC_IPRINT; break;
        case VT_DOUBLE: insn = BC_DPRINT; break;
        case VT_STRING: insn = BC_SPRINT; break;
        default: throw error("Unprintable type");
        }
        bytecode()->addInsn(insn);
    }

    tos_type_ = VT_VOID;
}

translator_impl::ret_type translator_impl::visitBinaryOpNode(BinaryOpNode* node)
{
    node->left()->visit(this);
    const VarType type1 = tos_type_;
    node->right()->visit(this);
    const VarType type2 = tos_type_;

    const TokenKind op = node->kind();
    const Instruction insn = make_instruction(op, type1, type2);
    bytecode()->addInsn(insn);
}


Instruction translator_impl::make_instruction(TokenKind op, VarType type1, VarType type2)
{
    if (type1 != type2)
        throw error("Typecasts not supported yet");
    
    const VarType type = type1;

    if (type == VT_INT)
    {
        switch(op)
        {
            case tADD: return BC_IADD;
            case tSUB: return BC_ISUB;
            case tMUL: return BC_IMUL;
            case tDIV: return BC_IDIV;
            case tMOD: return BC_IMOD;
        }
    }
    else if (type == VT_DOUBLE)
    {
        switch(op)
        {
        case tADD: return BC_DADD;
        case tSUB: return BC_DSUB;
        case tMUL: return BC_DMUL;
        case tDIV: return BC_DDIV;
        }
    }

    throw error("Wrong binary operation");
}


translator_impl::ret_type translator_impl::visitStringLiteralNode(StringLiteralNode* node)
{
    const uint16_t id = dst_code_->makeStringConstant(node->literal());
    
    bytecode()->addInsn(BC_SLOAD);
    bytecode()->addInt16(id); 
    tos_type_ = VT_STRING;
}

translator_impl::ret_type translator_impl::visitCallNode( CallNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitIntLiteralNode(IntLiteralNode* node)
{
    bytecode()->addInsn(BC_ILOAD);
    bytecode()->addInt64(node->literal()); 
    tos_type_ = VT_INT;
}

translator_impl::ret_type translator_impl::visitStoreNode(StoreNode* node)
{
    node->value()->visit(this);

    const TokenKind op = node->op();
    if (op == tASSIGN)
        store_tos_var(node->var());
    else
        throw error("Unsupported store operation");

    // a = b = 3
    load_tos_var(node->var());
    tos_type_ = node->var()->type();
}

translator_impl::ret_type translator_impl::visitLoadNode(LoadNode* node)
{
    load_tos_var(node->var());
    tos_type_ = node->var()->type();
}

translator_impl::ret_type translator_impl::visitReturnNode( ReturnNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitUnaryOpNode( UnaryOpNode* node )
{
    throw error("The method or operation is not implemented.");
}

Status* translator_impl::translate(const string& program, Code **code)
{
    Parser parser;
    Status* status = parser.parseProgram(program);
    if (status && !status->isOk())
        return status;

    dst_code_ = new code_impl();
    try
    {
        parser.top()->node()->visit(this);
        *code = dst_code_;
        return new Status();
    }
    catch (error const &e)
    {
        return new Status(e.what());
    }
}


translator_impl::translator_impl()
    : dst_code_(NULL)
    , current_scope_(NULL)
{

}

std::pair<context_id, var_id> translator_impl::get_var_ids(AstVar const *var, bool store, bool *out_is_local)
{
    Scope *scope = var->owner();

    context_ids_t::iterator it = context_ids_.find(scope);
    if (it == context_ids_.end())
    {
        assert(store);

        const context_id id = dst_code_->new_context();
        it = context_ids_.insert(std::make_pair(scope, id)).first;
    }

    context_t *context = dst_code_->context(it->second);

    context_t::vars_t::iterator var_it = context->vars.find(var->name());
    if (var_it == context->vars.end())
    {
        assert(store);

        const var_id id = context->vars.size();
        var_it = context->vars.insert(std::make_pair(var->name(), id)).first;
    }

    if (out_is_local)
        *out_is_local = (scope == current_scope_);
    
    return std::make_pair(it->second, var_it->second);
}

void translator_impl::load_tos_var( AstVar const *var )
{
    process_var(false, var);
}

void translator_impl::store_tos_var(AstVar const *var)
{
    process_var(true, var);
}

Instruction translator_impl::get_var_insn(bool store, AstVar const *var, bool is_local)
{
    const VarType var_type = var->type();
    if (store)
    {
        if (!is_local)
        {
            switch(var_type)
            {
            case VT_INT   : return BC_STORECTXIVAR;
            case VT_DOUBLE: return BC_STORECTXDVAR;
            case VT_STRING: return BC_STORECTXSVAR;
            }
        }
        else
        {
            switch(var_type)
            {
            case VT_INT   : return BC_STOREIVAR;
            case VT_DOUBLE: return BC_STOREDVAR;
            case VT_STRING: return BC_STORESVAR;
            }
        }
    }
    else
    {
        if (!is_local)
        {
            switch(var_type)
            {
            case VT_INT   : return BC_LOADCTXIVAR;
            case VT_DOUBLE: return BC_LOADCTXDVAR;
            case VT_STRING: return BC_LOADCTXSVAR;
            }
        }
        else
        {
            switch(var_type)
            {
            case VT_INT   : return BC_LOADIVAR;
            case VT_DOUBLE: return BC_LOADDVAR;
            case VT_STRING: return BC_LOADSVAR;
            }
        }
    }
    assert(0);
    return BC_INVALID;
}

void translator_impl::process_var(bool store, AstVar const *var)
{
    bool is_local;
    const std::pair<context_id, var_id> ids = get_var_ids(var, true, /*out*/ &is_local);

    const VarType var_type = var->type();

    if (store && tos_type_ != var_type)
        throw error("Typecasts not supported yet");

    const Instruction ins = get_var_insn(store, var, is_local);

    bytecode()->addInsn(ins);
    if (!is_local)
        bytecode()->addInt16(ids.first);
    bytecode()->addInt16(ids.second);
}

} // namespace mathvm