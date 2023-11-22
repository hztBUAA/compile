//
// Created by hzt on 2023/10/12.
//
#include<iostream>
using namespace std;
#include "ErrorHandler.h"
void ErrorHandler::Insert_Error(ErrorType errorType_from_GRAMMAR,int line) {
    if (errors->find(line) != errors->end()){
        return;
    }
    errors->insert(pair(line,errorType_from_GRAMMAR));

    error_type = errorType_from_GRAMMAR;//输出后 表示错误
}
void ErrorHandler::Insert_Error(ErrorType errorType_from_GRAMMAR) {
    switch (errorType_from_GRAMMAR) {
        case REDEFINE:
        case NOT_DEFINE:
        case ILLEGAL_STRING://format string illegal
        case INT_FUNC_NO_RETURN:
        case NOT_LOOP_USING_BC:
            error_line = lexer.line;
            break;
        case SEMICOLON_MISSING:
        case RPARENT_MISSING:
        case RBRACE_MISSING:
        case RBRACK_MISSING:
        case VOID_FUNC_HAS_RETURN://需要下一个字符才判断 但是输出的是return所在行
            error_line = lexer.line_lastWord;
            break;
        case PRINTF_NOT_EQUAL_COUNT:
        case FUNC_RPARAMS_COUNT_ERROR:
        case FUNC_RPARAMS_TYPE_ERROR:
        case CONST_LEFT:
            // 输出error_line就行  in SOMEWHERE has organized the error_line
            break;
        default:
            ;
            break;
    }
    if (errors->find(error_line) != errors->end()){
        return;
    }
    errors->insert(pair(error_line,errorType_from_GRAMMAR));

    error_type = errorType_from_GRAMMAR;//输出后 表示错误
}

char ErrorHandler::transformType2Char(ErrorType errorType) {
    switch (errorType) {
        case REDEFINE:return 'b';
        case NOT_DEFINE:return 'c';
        case ILLEGAL_STRING:return 'a';//format string illegal
        case VOID_FUNC_HAS_RETURN:return 'f';
        case INT_FUNC_NO_RETURN:return 'g';
        case CONST_LEFT:return 'h';
        case NOT_LOOP_USING_BC:return 'm';
        case SEMICOLON_MISSING:return 'i';
        case RPARENT_MISSING:return 'j';
        case RBRACE_MISSING:return 'z';//not exists
        case RBRACK_MISSING:return 'k';
        case PRINTF_NOT_EQUAL_COUNT:return 'l';
        case FUNC_RPARAMS_COUNT_ERROR:return 'd';
        case FUNC_RPARAMS_TYPE_ERROR:return 'e';
        default:return 'z';
    }
}

void ErrorHandler::Print_Errors() {
for(auto  error:*errors){
        if(enable){
//        ouputFile << error.first << " "<<transformType2Char(error.second)<<endl;
            errorFile << error.first << " "<<transformType2Char(error.second)<<endl;
    }
}
}

