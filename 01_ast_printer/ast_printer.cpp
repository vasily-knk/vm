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

ast_printer::ast_printer() 
    : indent_(0) 
    , needSemicolon(true)
{
}

ast_printer::~ast_printer() 
{
}


string ast_printer::get_indent_line() const 
{
    string indent_line;
    for(int i = 0; i < indent_; ++i) 
        indent_line += "    ";

    return indent_line;
}

void ast_printer::visitBinaryOpNode( BinaryOpNode* node ) 
{    
    node->left()->visit(this);    
    stream_ << " " << tokenOp(node->kind()) << " ";    
    node->right()->visit(this);      
}

void ast_printer::visitUnaryOpNode( UnaryOpNode* node ) 
{
    stream_ << tokenOp(node->kind());
    node->operand()->visit(this);
}

void ast_printer::visitStringLiteralNode( StringLiteralNode* node ) 
{
    print_escape(node->literal(), stream_);
}

void ast_printer::visitDoubleLiteralNode( DoubleLiteralNode* node ) 
{
    stream_ << node->literal();    
}

void ast_printer::visitIntLiteralNode( IntLiteralNode* node ) 
{
    stream_ << node->literal();    
}

void ast_printer::visitLoadNode( LoadNode* node ) 
{
    stream_ << node->var()->name();  
}

void ast_printer::visitStoreNode( StoreNode* node ) 
{
    stream_ << node->var()->name() << " " << tokenOp(node->op()) << " ";
    node->visitChildren(this);
    needSemicolon = true;
}

void ast_printer::visitForNode( ForNode* node ) 
{
    stream_ << "for (" << node->var()->name() << " in ";
    node->inExpr()->visit(this);
        
    stream_ << ")" << endl << "{" << endl;

    node->body()->visit(this);

    stream_ << "}";
    stream_ << endl;
    needSemicolon = false;
}

void ast_printer::visitWhileNode( WhileNode* node ) {

    stream_  << "while (";
    node->whileExpr()->visit(this);
    stream_ << ") {" << endl;

    node->loopBlock()->visit(this);
    stream_ << "}";
    stream_ << endl;
    needSemicolon = false;
}

void ast_printer::visitIfNode( IfNode* node ) {

    stream_ <<  "if (";
    node->ifExpr()->visit(this);
    stream_ << ") {" << endl;

    node->thenBlock()->visit(this);
    stream_ << get_indent_line() << "}";
    needSemicolon = false;

    if (node->elseBlock() == 0) {
        stream_ << endl;
        return;
    }

    stream_ << " else {" << endl;    
    node->elseBlock()->visit(this);
    stream_ << get_indent_line() << "}";
    stream_ << endl;
    needSemicolon = false;

}


void ast_printer::visitBlockNode( BlockNode* node ) {
    //Border border("blockNode", stream_);
    ++indent_;
    Scope::VarIterator var_iter(node->scope());
    bool hasVarDeclaration = var_iter.hasNext();

    while(var_iter.hasNext()) {
        AstVar const * var = var_iter.next();
        stream_ << get_indent_line() << typeToName(var->type()) << " " << var->name() <<";" << endl;    
    }

    if(hasVarDeclaration)
        stream_ << endl;

    Scope::FunctionIterator func(node->scope());
    while(func.hasNext()) func.next()->node()->visit(this);
    for (uint32_t i = 0; i < node->nodes(); ++i)
    {
        stream_ << get_indent_line();
        node->nodeAt(i)->visit(this);

        if (needSemicolon) {
            stream_ << ";";
        }        

        stream_ << endl;
        needSemicolon = true;
    }   
    --indent_;    
}

void ast_printer::visitFunctionNode( FunctionNode* node ) {
    if(node->name() == "<top>") {
        indent_ = -1;
        node->body()->visit(this);
        return;
    }

    stream_ << endl;
    stream_ << get_indent_line() << "function " << typeToName(node->returnType()) << " " << node->name() << "(";

    for (unsigned int j = 0; j < node->parametersNumber(); ++j) {
        if (j > 0) stream_ << ", ";

        stream_ << typeToName(node->parameterType(j)) << " " << node->parameterName(j);
    }

    stream_ << ") ";

    if (node->body()->nodes() > 0 && node->body()->nodeAt(0)->isNativeCallNode()) {
        node->body()->nodeAt(0)->visit(this);
    } else {
        stream_ << "{" << endl;

        node->body()->visit(this);

        stream_ << "}" << endl;
        needSemicolon = false;
    }
}

void ast_printer::visitReturnNode( ReturnNode* node ) {
    needSemicolon = false;

    if(node->returnExpr() != 0) {
        stream_ << "return ";
        node->returnExpr()->visit(this);
        needSemicolon = true;
    }        
}

void ast_printer::visitCallNode( CallNode* node ) {
    stream_ << node->name() << "(";

    for (unsigned int i = 0; i < node->parametersNumber(); ++i) {
        if (i > 0) stream_ << ", ";
        node->parameterAt(i)->visit(this);
    }
    stream_ << ")";
    needSemicolon = true;
}

void ast_printer::visitNativeCallNode( NativeCallNode* node ) {
    stream_ << "native '"<< node->nativeName() << "';" << endl;
}


void ast_printer::visitPrintNode( PrintNode* node ) {
    stream_ << "print(";

    for (unsigned int i = 0; i < node->operands(); ++i) {
        if (i > 0) stream_ << ", ";
        AstNode *pNode = node->operandAt(i);
        pNode->visit(this);
    }
    stream_ << ")";
    needSemicolon = true;
}

string ast_printer::print_tree( AstNode *head )
{
    stream_.clear();
    head->visit(this);
    return stream_.str();
}



} // namespace mathvm;


