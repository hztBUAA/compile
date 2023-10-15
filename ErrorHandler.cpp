//
// Created by hzt on 2023/10/12.
//
#include<iostream>
using namespace std;
#include "ErrorHandler.h"

void ErrorHandler::Print_Error(ErrorType errorType_from_GRAMMAR) {
    switch (errorType_from_GRAMMAR) {
        case REDEFINE:
        case NOT_DEFINE:
            error_line = lexer.line;
            break;
        case SEMICOLON_MISSING:
            case RPARENT_MISSING:
            case RBRACE_MISSING:
            case RBRACK_MISSING:
            error_line = lexer.line_lastWord;
            break;
        case ILLEGAL_STRING:
            //PRINTf
            break;
        default:
            break;
    }
    if(enable){
        cout << error_line << " "<<transformType2Char(errorType_from_GRAMMAR)<<endl;
    }
    error_type = NORMAL;//输出后 表示错误结束
}

char ErrorHandler::transformType2Char(ErrorType errorType) {
    switch (errorType) {
        case REDEFINE:
            return 'b';
        case SEMICOLON_MISSING:
            return 'i';

        default:
            return 'z';
    }
}

