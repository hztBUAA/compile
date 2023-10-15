//
// Created by hzt on 2023/10/12.
//
/**
 * 有点像没有思想的傀儡  思想集中在语法部分
 * 单一职责：
 * 提供错误类型
 * 打印错误
 */
#ifndef LEX_ERRORHANDLER_H
#define LEX_ERRORHANDLER_H
#include "Lexer.h"
enum ErrorType{
    NORMAL,//错误结束
    REDEFINE,//重定义
    NOT_DEFINE,//从未定义过
    SEMICOLON_MISSING,
    RBRACK_MISSING,
    RPARENT_MISSING,
    RBRACE_MISSING,
    ILLEGAL_STRING,
    PRINTF_NOT_EQUAL_COUNT,
    VOID_FUNC_HAS_RETURN,
    INT_FUNC_NO_RETURN,
    CONST_LEFT,
    NOT_LOOP_USING_BC

};


class ErrorHandler {
private:

    bool enable;
    Lexer &lexer;
public:
    int error_line;//对于ijk类型的缺少句末符号错误   line_lastWord    其他类型 还不一样可能就是当前行
    ErrorType error_type;
    explicit ErrorHandler(Lexer &lexer1) :lexer(lexer1),enable(true),error_line(0),error_type(NORMAL){};//构造.h文件中直接定义  需要是一个（）{}的结构
    void Print_Error(ErrorType errorType);
    static char transformType2Char(ErrorType);//can be made static?   不用到实例变量？

};


#endif //LEX_ERRORHANDLER_H
