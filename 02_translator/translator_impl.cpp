#include "stdafx.h"
#include "translator_impl.h"
#include "parser.h"

namespace mathvm
{

translator_impl::ret_type translator_impl::visitDoubleLiteralNode( DoubleLiteralNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
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

translator_impl::ret_type translator_impl::visitBinaryOpNode( BinaryOpNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitStringLiteralNode( StringLiteralNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitCallNode( CallNode* node )
{
    throw std::logic_error("The method or operation is not implemented.");
}

translator_impl::ret_type translator_impl::visitIntLiteralNode( IntLiteralNode* node )
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

    parser.top()->node()->visit(this);
    
    return new Status();
}

} // namespace mathvm