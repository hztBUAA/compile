#include <iostream>
#include <fstream>
#include"Lexer.h"
#include "Parser.h"


//要按照错误的先后顺序  对着文法来
int main() {
    ifstream input("testfile.txt");
    ofstream output("output.txt");
    Lexer & my_lexer = Lexer::initLexer(input,output);//相当于 放给语法Parser去指导Lexer
    TableManager tableManager;
    ErrorHandler errorHandler(my_lexer);
    Semantic semantic(my_lexer,tableManager,errorHandler);
    Parser parser(my_lexer,tableManager,errorHandler,semantic);
    parser.CompUnit();
    //my_lexer.analyze();
    return 0;
}
