//
// Created by hzt on 2023/9/25.
//
#ifndef LEX_PARSER_H
#define LEX_PARSER_H
#include "Lexer.h"
#include "TableManager.h"
#include "ErrorHandler.h"
#include "Semantic.h"
#include "IntermediateCode.h"
#define WORD_TYPE lexer.token_type
#define GET_A_WORD lexer.nextSymbol()
#define PEEK_A_LETTER lexer.sourceFile.peek()
#define PRINT_WORD lexer.printOutput()
#define WORD_DISPLAY (*lexer.token.symbol)
#define INFO_ENTRY (tableManager.info)

#define ISGLOBAL  (tableManager.cur->kind ==Kind::CompUnit)
extern bool isInOtherFunc;
extern string funcLabel;


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
    bool func_rParams_not_define = false;
    int printf_line;//for printf arguments error
    int Exp_type;//0 1 2-------为了鉴别函数实参类型是整数 一级地址 二级地址
    string func_name;//保存函数调用时的名字  懒得再传进去名字了
    Lexer & lexer;   //为什么都要加 &
    TableManager & tableManager;
    ErrorHandler& errorHandler; //如果漏了&号  在构造函数报了warning
    Semantic & semantic;
    IntermediateCode & intermediateCode;
public:

    Parser(Lexer& lexer1,TableManager& tableManager1,ErrorHandler &errorHandler1,Semantic& semantic1,IntermediateCode & intermediateCode1):lexer(lexer1),tableManager(tableManager1),errorHandler(errorHandler1),semantic(semantic1),intermediateCode(intermediateCode1),enablePrint(false),isLValInStmt(false){};
    void Print_Grammar_Output(const string& s);
    void CompUnit();
    int Kind2Exp_type (Kind kind);
    void Decl();
    void ConstDecl();
    void ConstDef(vector<Entry*> & entries);
    void ConstInitVal(IEntry *iEntry,int&nums);
    void VarDecl();
    void VarDef(vector<Entry*> & entries);
    void InitVal(IEntry * iEntry,int & nums);
    void FuncDef(Kind func_type);
    void MainFuncDef();
    void FuncType();
    void FuncFParams(vector<Entry*> & entries);
    void FuncFParam(vector<Entry*> & entries);
    void Block();
    void BlockItem();
    void Stmt();
  void FormatString(IEntry * strings);
//    void NormalChar();
//    void FormalChar();
    void ForStmt();
    void Exp(IEntry *iEntry,int&value,bool isInOtherFunc);
    void Cond();
    void LVal(IEntry ** iEntry,int & value,bool inOtherFunc);
    void PrimaryExp(IEntry * iEntry,int & value,bool InOtherFunc);
    void Number(IEntry *iEntry,int & value,bool InOtherFunc);
    void UnaryExp(IEntry *iEntry,int&value,bool InOtherFunc);
    void UnaryOp(int &op);
    void FuncRParams(int func_ident_line,vector<int> *RParams);
    void MulExp(IEntry *iEntry,int&value,bool InOtherFunc);
    void AddExp(IEntry *iEntry,int&value,bool InOtherFunc);
    void RelExp(IEntry * iEntry,bool InOtherFunc);
    void EqExp(IEntry * iEntry,bool InOtherFunc);
    void LAndExp(IEntry * iEntry,bool InOtherFunc);
    void LOrExp(IEntry * iEntry,bool InOtherFunc);
    void ConstExp(IEntry *iEntry,int&value,bool InOtherFunc);
    bool inArguments(const vector<Entry *>& arguments,string ident);

};


#endif //LEX_PARSER_H
