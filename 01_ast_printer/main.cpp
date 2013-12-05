#include "stdafx.h"

#include "parser.h"
#include "mathvm.h"
#include "ast_printer.h"

int main() 
{
    const char* buffer = mathvm::loadFile("1.mvm");
    const string text(buffer);
    delete[] buffer;

    cout << text << endl;

    mathvm::Parser parser;
    mathvm::Status* status = parser.parseProgram(text);

    if(status != NULL && !status->isOk()) {
        cerr << status->getError() << endl;
        return 1;       
    }

    mathvm::ast_printer printer;
    cout << printer.print_tree(parser.top()->node());

    return 0;
}