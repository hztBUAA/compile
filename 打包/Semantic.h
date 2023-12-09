//
// Created by hzt on 2023/10/13.
//

#ifndef LEX_SEMANTIC_H
#define LEX_SEMANTIC_H
#include "TableManager.h"
#include"Lexer.h"
#include "ErrorHandler.h"

/**
 * 对于函数定义时的记录：
 * Functype FuncName
 *
 *
 * 遵循单一职责原则
 * 语义分析器：操作符号表管理器进行
 * 1.记录符号
 *  recordEntries(vector<Entry>)
 *
 * 2.查询符号
 *  2-1. 是否重定义   isRedefine(string ident)
 *  2-2. 是否定义过 检查类型  hasEverDefine(string ident,Kind kind)
 */

/**
 * 重定义的错误 放在了ErrorHandler中  （因为认为不需要上下文
 * 未定义的错误则放在了这里  由Semantic封装调用ErrorHandler进行语义上下文判断曾经是否定义过
 */
class Semantic {

private:
    TableManager& tableManager;
    Lexer &lexer;//用来错误局部化处理
    ErrorHandler & errorHandler;
public:
    Semantic(Lexer&lexer1,TableManager & tableManager1,ErrorHandler& errorHandler1): lexer(lexer1),tableManager(tableManager1),
                                                                                   errorHandler(errorHandler1){};
//    void fillInfoEntry(string,);
    void recordEntries(vector<Entry*> &entries);
    void recordEntries(Entry * entry);
    [[nodiscard]] Entry * fillInfoEntry(string ident, Kind kind) const;

    bool isEverDefined(const string& ident, Kind kind,bool isLeft);
    void skip(PIECE piece);
};


#endif //LEX_SEMANTIC_H
