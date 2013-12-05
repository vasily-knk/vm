#include "stdafx.h"
#include "translator_impl.h"
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
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitForNode( ForNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitFunctionNode( FunctionNode* node )
{
    node->body()->visitChildren(this);
}

translator_impl::ret_type translator_impl::visitBlockNode( BlockNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitLoadNode( LoadNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitIfNode( IfNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitNativeCallNode( NativeCallNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitPrintNode( PrintNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitBinaryOpNode(BinaryOpNode* node)
{
    node->left()->visit(this);
    const VarType type1 = tos_type_;
    node->right()->visit(this);
    const VarType type2 = tos_type_;

    bytecode()->addInsn(make_instruction(node->kind(), type1, type2));
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
    throw std::logic_error("The method or operation is not implemented.");
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
}

translator_impl::ret_type translator_impl::visitReturnNode( ReturnNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitUnaryOpNode( UnaryOpNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

Status* translator_impl::translate(const string& program, Code **code)
{
    Parser parser;
    Status* status = parser.parseProgram(program);
    if (status && !status->isOk())
        return status;

    
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

} // namespace mathvm