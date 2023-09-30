#include <iostream>
#include <fstream>
#include"Lexer.h"
#include "Parser.h"

int main() {
    ifstream input("testfile.txt");
    ofstream output("output.txt");
    Lexer & my_lexer = Lexer::initLexer(input,output);//相当于 放给语法Parser去指导Lexer
    Parser parser(my_lexer);
    parser.CompUnit();
    //my_lexer.analyze();
    return 0;
}
