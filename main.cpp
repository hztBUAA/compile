#include <iostream>
#include <fstream>
#include"Lexer.h"
#include "Parser.h"


//要按照错误的先后顺序  对着文法来
int main() {
    ifstream input("testfile.txt");
    ofstream output("output.txt");
    ofstream errorFile("error.txt");
    ofstream mipsFile("mips.txt");
    Lexer & my_lexer = Lexer::initLexer(input,output);//相当于 放给语法Parser去指导Lexer
    IntermediateCode intermediateCode{};
    MipsCode mipsCode(intermediateCode,mipsFile);

    ErrorHandler errorHandler(my_lexer,errorFile);
    TableManager tableManager(errorHandler);
    Semantic semantic(my_lexer,tableManager,errorHandler);
    Parser parser(my_lexer,tableManager,errorHandler,semantic,intermediateCode);
    parser.CompUnit();
    //errorHandler.Print_Errors();
//    intermediateCode.debug_print();
    mipsCode.translate();
//mipsCode.testRe();
    //my_lexer.analyze();
    return 0;
}
