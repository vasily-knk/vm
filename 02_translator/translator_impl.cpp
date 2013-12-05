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

translator_impl::ret_type translator_impl::visitFunctionNode( FunctionNode* node )
{
    node->body()->visitChildren(this);
}

translator_impl::ret_type translator_impl::visitBlockNode( BlockNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitLoadNode(LoadNode* node)
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitIfNode( IfNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitNativeCallNode( NativeCallNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitPrintNode( PrintNode* node )
{
    throw error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitBinaryOpNode(BinaryOpNode* node)
{
    node->left()->visit(this);
    const VarType type1 = tos_type_;
    node->right()->visit(this);
    const VarType type2 = tos_type_;

    const Instruction insn = make_instruction(node->kind(), type1, type2);
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
    const std::pair<context_id, var_id> ids = get_var_ids(node->var(), true);

    const VarType var_type = node->var()->type();

    Instruction ins;
    switch(var_type)
    {
    case VT_INT   : ins = BC_STORECTXIVAR; break;
    case VT_DOUBLE: ins = BC_STORECTXDVAR; break;
    case VT_STRING: ins = BC_STORECTXSVAR; break;
    default: assert(false);
    }

    node->value()->visit(this);

    if (tos_type_ != var_type)
        throw error("Typecasts not supported yet");

    const TokenKind op = node->op();

    if (op == tASSIGN)
    {
        bytecode()->addInsn(ins);
        bytecode()->addInt16(ids.first);
        bytecode()->addInt16(ids.second);
    }
    else
    {
        throw error("Unsupported store operation");
    }

    // stub for a = b = 3
    tos_type_ = VT_VOID;
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
{

}

std::pair<context_id, var_id> translator_impl::get_var_ids(AstVar const *var, bool store)
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
    
    return std::make_pair(it->second, var_it->second);
}

} // namespace mathvm