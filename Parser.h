//
// Created by hzt on 2023/9/25.
//
#ifndef LEX_PARSER_H
#define LEX_PARSER_H
#include "Lexer.h"
/**
 *Parser  的编写规则    注意：1.指向WORD   2.输出WORD
 * 依据文法说明
 * 每一个非终结符的函数进入前 是指向其文法产生式的第一个WORD || 自说明（集中在第一层的多产生式时）   但是还没有输出这个WORD
 *                                      结束后  指向其文法产生式的最后一个WORD的下一个WORD   但是还没有输出这个WORD
 *
 *  PRINT_WORD  是输出当前指向的WORD    输出不能超前输出  意思是 只有当前非终结符对应的右边产生式存在这个终结符时  才能输出
 *  GET_A_WORD 是指向下一个WORD   是可以超前判断的  防止回溯
 *  PEEK 是看下一个字符    赋值给lexer.ch
 */

class Parser {
private:
    bool enablePrint;
    bool isLValInStmt;
    Lexer & lexer;
public:
    Parser(Lexer& lexer1):lexer(lexer1),enablePrint(true),isLValInStmt(false){};
    void Print_Grammar_Output(string s);
    void CompUnit();

    void Decl();
    void ConstDecl();
    void ConstDef();
    void ConstInitVal();
    void VarDecl();
    void VarDef();
    void InitVal();
    void FuncDef();
    void MainFuncDef();
    void FuncType();
    void FuncFParams();
    void FuncFParam();
    void Block();
    void BlockItem();
    void Stmt();
    void FormatString();
//    void NormalChar();
//    void FormalChar();
    void ForStmt();
    void Exp();
    void Cond();
    void LVal();
    void PrimaryExp();
    void Number();
    void UnaryExp();
    void UnaryOp();
    void FuncRParams();
    void MulExp();
    void AddExp();
    void RelExp();
    void EqExp();
    void LAndExp();
    void LOrExp();
    void ConstExp();


};


#endif //LEX_PARSER_H
