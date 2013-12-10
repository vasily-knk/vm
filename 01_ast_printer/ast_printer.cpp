#include "stdafx.h"
#include "ast_printer.h"

namespace mathvm
{

namespace
{

void print_escape( std::string const &str, std::ostream & out ) {
    out << "\'";
    for (unsigned int i = 0; i < str.length(); ++i)
        switch(str[i]) {
        case '\\':
            out << "\\\\";
            break;
        case '\r':
            out << "\\r";
            break;        
        case '\t':
            out << "\\t";
            break;
        case '\n':
            out << "\\n";
            break;
        default:
            out << str[i];
            break;
    }
    out << "\'";
}

}

#define AST_PRINT(items) { ident(); stream_ << items; }

ast_printer::ast_printer() 
    : indent_(0) 
    , needSemicolon(true)
{
}

ast_printer::~ast_printer() 
{
}

void ast_printer::visitBinaryOpNode(BinaryOpNode* node) 
{    
    node->left()->visit(this);    
    stream_ << " " << tokenOp(node->kind()) << " ";    
    node->right()->visit(this);      
}

void ast_printer::visitUnaryOpNode(UnaryOpNode* node) 
{
    stream_ << tokenOp(node->kind());
    node->operand()->visit(this);
}

void ast_printer::visitStringLiteralNode(StringLiteralNode* node) 
{
    print_escape(node->literal(), stream_);
}

void ast_printer::visitDoubleLiteralNode(DoubleLiteralNode* node) 
{
    stream_ << node->literal();    
}

void ast_printer::visitIntLiteralNode(IntLiteralNode* node) 
{
    stream_ << node->literal();    
}

void ast_printer::visitLoadNode(LoadNode* node) 
{
    stream_ << node->var()->name();  
}

void ast_printer::visitStoreNode(StoreNode* node) 
{
    stream_ << node->var()->name() << " " << tokenOp(node->op()) << " ";
    node->visitChildren(this);
    needSemicolon = true;
}

void ast_printer::visitForNode(ForNode* node) 
{
    stream_ << "for (" << node->var()->name() << " in ";
    node->inExpr()->visit(this);
    stream_ << ")" << endl;

    ident();
    stream_ << "{" << endl;
    node->body()->visit(this);
    
    ident();
    stream_ << "}" << endl;
    needSemicolon = false;
}

void ast_printer::visitWhileNode(WhileNode* node) 
{
    stream_  << "while (";
    node->whileExpr()->visit(this);
    stream_ << ")" << endl;

    ident();
    stream_ << "{" << endl;

    node->loopBlock()->visit(this);
    
    ident();
    stream_ << "}" << endl;
    needSemicolon = false;
}

void ast_printer::visitIfNode( IfNode* node ) 
{
    stream_ <<  "if (";
    node->ifExpr()->visit(this);
    stream_ << ")" << endl;

    ident();
    stream_ << "{" << endl;
    node->thenBlock()->visit(this);

    needSemicolon = false;

    ident();
    stream_ << "}" << endl;

    if (node->elseBlock())
    {
        ident();
        stream_ << "else" << endl;
        ident();
        stream_ << "{" << endl;

        node->elseBlock()->visit(this);
        
        ident();
        stream_ << "}" << endl;
    }
    needSemicolon = false;
}


void ast_printer::visitBlockNode(BlockNode* node) 
{
    ++indent_;

    Scope::VarIterator var_iter(node->scope());
    bool hasVarDeclaration = var_iter.hasNext();

    
    for (Scope::VarIterator it(node->scope(), false); it.hasNext(); )
    {
        AstVar const *var = it.next();
        
        ident();
        stream_ << typeToName(var->type()) << " " << var->name() <<";" << endl;    
    }

    for (Scope::FunctionIterator it(node->scope(), false); it.hasNext(); )
    {
        AstFunction const *fn = it.next();

        ident();
        fn->node()->visit(this);
    }

    for (uint32_t i = 0; i < node->nodes(); ++i)
    {
        ident();
        node->nodeAt(i)->visit(this);

        if (needSemicolon) {
            stream_ << ";";
        }        

        stream_ << endl;
        needSemicolon = true;
    }

    --indent_;    
}

void ast_printer::visitFunctionNode(FunctionNode* node) 
{
    if(is_top(node)) {
        indent_ = -1;
        node->body()->visit(this);
        return;
    }

    stream_ << "function " << typeToName(node->returnType()) << " " << node->name() << "(";

    for (uint32_t i = 0; i < node->parametersNumber(); ++i)
    {
        stream_ << typeToName(node->parameterType(i)) << " " << node->parameterName(i);

        if (i != node->parametersNumber() - 1)
            stream_ << ", ";
    }

    stream_ << ")" << endl;

    ident();
    stream_ << "{" << endl;

    node->body()->visit(this);

    ident();
    stream_ << "}" << endl;
}

void ast_printer::visitReturnNode(ReturnNode* node) 
{
    needSemicolon = false;

    stream_ << "return";
    
    if (node->returnExpr())
    {
        stream_ << " ";
        node->returnExpr()->visit(this);
    }

    needSemicolon = true;
}

void ast_printer::visitCallNode(CallNode* node) 
{
    stream_ << node->name() << "(";

    for (unsigned int i = 0; i < node->parametersNumber(); ++i) {
        node->parameterAt(i)->visit(this);

        if (i != node->parametersNumber() - 1)
            stream_ << ", ";
    }
    
    stream_ << ")";
    needSemicolon = true;
}

void ast_printer::visitNativeCallNode(NativeCallNode* node) 
{
}

void ast_printer::visitPrintNode(PrintNode* node) 
{
    stream_ << "print(";

    for (unsigned int i = 0; i < node->operands(); ++i) {
        node->operandAt(i)->visit(this);

        if (i != node->operands() - 1)
            stream_ << ", ";
    }
    stream_ << ")";
    needSemicolon = true;
}

string ast_printer::print_tree(AstNode *head)
{
    stream_.clear();
    head->visit(this);
    return stream_.str();
}

void ast_printer::ident()
{
    for(int i = 0; i < indent_; ++i) 
        stream_ <<  "    ";
}

bool ast_printer::is_top(FunctionNode const *node)
{
    return node->name() == "<top>";
}



} // namespace mathvm;


