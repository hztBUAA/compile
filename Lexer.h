//
// Created by hzt on 2023/9/19.
//

#ifndef LEX_LEXER_H
#define LEX_LEXER_H

#include <fstream>
#include <map>
using namespace std;


enum Type{
    FINISH,
    NOTE,
    ILLEGAL,
    IDENFR,
    INTCON,
    STRCON,
    MAINTK,
    CONSTTK,
    INTTK,
    BREAKTK,
    CONTINUETK,
    IFTK,
    ELSETK,
    NOT,
    AND,
    OR,
    FORTK,
    GETINTTK,
    PRINTFTK,
    RETURNTK,
    PLUS,
    MINU,
    VOIDTK,
    MULT,
    DIV,
    MOD,
    LSS,
    LEQ,
    GRE,
    GEQ,
    EQL,
    NEQ,
    ASSIGN,
    SEMICN,
    COMMA,
    LPARENT,
    RPARENT,
    LBRACK,
    RBRACK,
    LBRACE,
    RBRACE
};

class Lexer {
private:
    ifstream &sourceFile;
    ofstream &outputFile;
    bool enableOutput;
    int line;
    char ch;
    Type token_type;
    union Token{
        int number;
        string *symbol;///?  如果不是指针 似乎有关构造函数会出问题
    }token;
    map<string, Type> wordCategory;
    //私有化-构造函数
    Lexer(ifstream & sourceFile,ofstream & outputFile);
public:
    static Lexer& initLexer(ifstream& sourceFile,ofstream & outputFile);//为什么要对外一个？
    //symbol  是符号  包括标识符、保留字、以及所有其他除了单引号、双引号之间的字符串
    void nextSymbol();
    //单引号之间的
    void nextChar();
    //双引号之间的
    void nextString();

    void analyze  ();//核心函数  进行解析词法  并且根据编译开关进行输出

    void printOutput();//token_type token{number string}


};

#define ENUM_TO_STRING(enumVal) \
    (enumVal == FINISH ? "FINISH" : \
    (enumVal == NOTE ? "NOTE" : \
    (enumVal == ILLEGAL ? "ILLEGAL" : \
    (enumVal == IDENFR ? "IDENFR" : \
    (enumVal == INTCON ? "INTCON" : \
    (enumVal == STRCON ? "STRCON" : \
    (enumVal == MAINTK ? "MAINTK" : \
    (enumVal == CONSTTK ? "CONSTTK" : \
    (enumVal == INTTK ? "INTTK" : \
    (enumVal == BREAKTK ? "BREAKTK" : \
    (enumVal == CONTINUETK ? "CONTINUETK" : \
    (enumVal == IFTK ? "IFTK" : \
    (enumVal == ELSETK ? "ELSETK" : \
    (enumVal == NOT ? "NOT" : \
    (enumVal == AND ? "AND" : \
    (enumVal == OR ? "OR" : \
    (enumVal == FORTK ? "FORTK" : \
    (enumVal == GETINTTK ? "GETINTTK" : \
    (enumVal == PRINTFTK ? "PRINTFTK" : \
    (enumVal == RETURNTK ? "RETURNTK" : \
    (enumVal == PLUS ? "PLUS" : \
    (enumVal == MINU ? "MINU" : \
    (enumVal == VOIDTK ? "VOIDTK" : \
    (enumVal == MULT ? "MULT" : \
    (enumVal == DIV ? "DIV" : \
    (enumVal == MOD ? "MOD" : \
    (enumVal == LSS ? "LSS" : \
    (enumVal == LEQ ? "LEQ" : \
    (enumVal == GRE ? "GRE" : \
    (enumVal == GEQ ? "GEQ" : \
    (enumVal == EQL ? "EQL" : \
    (enumVal == NEQ ? "NEQ" : \
    (enumVal == ASSIGN ? "ASSIGN" : \
    (enumVal == SEMICN ? "SEMICN" : \
    (enumVal == COMMA ? "COMMA" : \
    (enumVal == LPARENT ? "LPARENT" : \
    (enumVal == RPARENT ? "RPARENT" : \
    (enumVal == LBRACK ? "LBRACK" : \
    (enumVal == RBRACK ? "RBRACK" : \
    (enumVal == LBRACE ? "LBRACE" : \
    (enumVal == RBRACE ? "RBRACE" : "Unknown")))))))))))))))))))))))))))))))))))))))))
#endif //LEX_LEXER_H
