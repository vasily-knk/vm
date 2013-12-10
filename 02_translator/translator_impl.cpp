#include "stdafx.h"
#include "translator_impl.h"
#include "code_impl.h"
#include "parser.h"

namespace mathvm
{

void translator_impl::visitDoubleLiteralNode(DoubleLiteralNode* node)
{
    bytecode()->addInsn(BC_DLOAD);
    bytecode()->addDouble(node->literal()); 
    tos_type_ = VT_DOUBLE;
}

void translator_impl::visitWhileNode( WhileNode* node )
{
    throw error("The method or operation is not implemented.");
}

void translator_impl::visitForNode( ForNode* node )
{
    throw error("The method or operation is not implemented.");
}

void translator_impl::visitFunctionNode(FunctionNode* node)
{
    context_t const &context = contexts_.at(current_scope_);
    const function_id_t id = context.functions.at(node->name());
    
    dst_func_id_ = id;

    signature_ = &node->signature();
    node->body()->visit(this);
}

void translator_impl::visitBlockNode(BlockNode* node)
{
    add_context(node->scope(), signature_);
    signature_ = NULL;

    typedef vector<AstNode*> nodes_vector_t;
    nodes_vector_t to_visit;

    for (Scope::FunctionIterator it(node->scope(), false); it.hasNext(); )
    {
        AstFunction *fn = it.next();
        to_visit.push_back(fn->node());
    }

    for (uint32_t i = 0; i < node->nodes(); ++i)
        to_visit.push_back(node->nodeAt(i));

    for (size_t i = 0; i < to_visit.size(); ++i)
    {
        current_scope_ = node->scope();
        dst_code_->get_function_dst(dst_func_id_)->set_context(contexts_.at(current_scope_).id);
        tos_type_ = VT_VOID;

        to_visit.at(i)->visit(this);
        if (tos_type_ != VT_VOID)
            bytecode()->addInsn(BC_POP);
    }

    current_scope_ = node->scope();
    dst_code_->get_function_dst(dst_func_id_)->set_context(contexts_.at(current_scope_).id);
    tos_type_ = VT_VOID;
    
    assert(current_scope_ == node->scope());
    contexts_.erase(current_scope_);
}

void translator_impl::add_context(Scope *scope, Signature const *signature)
{
    context_t context(contexts_.size());

    if (signature)
    {
        assert(context.vars.size() == 0);

        for (var_id_t i = 0; i < signature->size(); ++i)
        {
            const string name = signature->at(i).second;
            const bool inserted = context.vars.insert(make_pair(name, i)).second;
            assert(inserted);
        }
        
        assert(context.vars.size() == signature->size());
    }

    for (Scope::VarIterator it(scope, false); it.hasNext();)
    {
        AstVar const *var = it.next();
        const bool insertion = context.vars.insert(std::make_pair(var->name(), context.vars.size())).second;
        assert(insertion);
    }

    for (Scope::FunctionIterator it(scope, false); it.hasNext();)
    {
        AstFunction const *fn = it.next();
        const function_id_t id = dst_code_->add_function();
        const bool insertion = context.functions.insert(make_pair(fn->name(), id)).second;
        assert(insertion);
    }

    contexts_.insert(make_pair(scope, context));
}


void translator_impl::visitIfNode( IfNode* node )
{
    throw error("The method or operation is not implemented.");
}

void translator_impl::visitNativeCallNode( NativeCallNode* node )
{
    throw error("The method or operation is not implemented.");
}

void translator_impl::visitPrintNode(PrintNode* node)
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

void translator_impl::visitBinaryOpNode(BinaryOpNode* node)
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


void translator_impl::visitStringLiteralNode(StringLiteralNode* node)
{
    const uint16_t id = dst_code_->makeStringConstant(node->literal());
    
    bytecode()->addInsn(BC_SLOAD);
    bytecode()->addInt16(id); 
    tos_type_ = VT_STRING;
}

void translator_impl::visitCallNode( CallNode* node )
{
    throw error("The method or operation is not implemented.");
}

void translator_impl::visitIntLiteralNode(IntLiteralNode* node)
{
    bytecode()->addInsn(BC_ILOAD);
    bytecode()->addInt64(node->literal()); 
    tos_type_ = VT_INT;
}

void translator_impl::visitStoreNode(StoreNode* node)
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

void translator_impl::visitLoadNode(LoadNode* node)
{
    load_tos_var(node->var());
    tos_type_ = node->var()->type();
}

void translator_impl::visitReturnNode(ReturnNode* node)
{
    if (node->returnExpr()) 
        node->returnExpr()->visit(this);
    
    bytecode()->addInsn(BC_RETURN);
}

void translator_impl::visitUnaryOpNode( UnaryOpNode* node )
{
    throw error("The method or operation is not implemented.");
}



translator_impl::translator_impl()
    : dst_code_(NULL)
    , current_scope_(NULL)
    , signature_(NULL)
{

}

std::pair<context_id_t, var_id_t> translator_impl::get_var_ids(AstVar const *var, bool store, bool *out_is_local)
{
    Scope const *scope = var->owner();
    
    if (out_is_local)
        *out_is_local = (scope == current_scope_);

    context_t const &context = contexts_.at(scope);
    const var_id_t var_id = context.vars.at(var->name());
    return std::make_pair(context.id, var_id);
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
    const std::pair<context_id_t, var_id_t> ids = get_var_ids(var, true, /*out*/ &is_local);

    const VarType var_type = var->type();

    if (store && tos_type_ != var_type)
        throw error("Typecasts not supported yet");

    const Instruction ins = get_var_insn(store, var, is_local);

    bytecode()->addInsn(ins);
    if (!is_local)
        bytecode()->addInt16(ids.first);
    bytecode()->addInt16(ids.second);
}

function_id_t translator_impl::find_function(string const &name) const
{
    AstFunction const * ast_fn = current_scope_->lookupFunction(name, true);
    context_t const &context = contexts_.at(ast_fn->owner());
    return context.functions.at(name);
}

void translator_impl::init()
{
    tos_type_ = VT_INVALID;
    dst_code_ = NULL;
    dst_func_id_ = -1;
    current_scope_ = NULL;
    signature_ = NULL;
    contexts_.clear();
}

void translator_impl::prepare(AstFunction *top)
{
    dst_code_ = new code_impl();
    current_scope_ = top->scope();
    add_context(current_scope_, NULL);
}

Status* translator_impl::translate(const string& program, Code **code)
{
    init();

    Parser parser;
    Status* status = parser.parseProgram(program);
    if (status && !status->isOk())
        return status;

    try
    {
        prepare(parser.top());
        parser.top()->node()->visit(this);
        *code = dst_code_;
        return new Status();
    }
    catch (error const &e)
    {
        return new Status(e.what());
    }
}

} // namespace mathvm