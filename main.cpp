#include <iostream>
#include <fstream>
#include"Lexer.h"

int main() {
    ifstream input("testfile.txt");
    ofstream output("output.txt");
    Lexer & my_lexer = Lexer::initLexer(input,output);
    my_lexer.analyze();
    return 0;
}
