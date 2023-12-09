//
// Created by hzt on 2023/10/13.
//

#include "Semantic.h"

#include <utility>
#include "TableManager.h"
#include "Parser.h"
//非函数的填表项 可以插入的 插入至当前的孩子中
//也可以是函数
//void Semantic::fillInfoEntry(string ident, Kind kind) {
//    tableManager.info = new Entry;//关于info
//    INFO_ENTRY->ident = ident; //关于C语言的特性  值传递？
//    INFO_ENTRY->type.kind = kind;
//
//    INFO_ENTRY->entries = new map<string, Entry*>;
//    INFO_ENTRY->Father_Entry = tableManager.cur;
//
//    //留给语法GRAMMAR调用tableManager的insert
//}



bool Semantic::isEverDefined(const string& ident , Kind kind,bool isLeft) { //这里的isLEFT就懒得去掉了 已经没用了这个参数  判断左值不能为常量采取单独判断写法
    return tableManager.isEverDefine(ident,kind);
}

//生成一个entry
Entry *Semantic::fillInfoEntry(string ident, Kind kind) const {
    auto * entry = new Entry;
    entry->ident = std::move(ident);
    entry->kind = kind;
    entry->loop_count = 0;
    entry->fParams = nullptr;
    entry->entries = new map<string , Entry*>;//所有都要确定entries不要为空   因为函数创建表项时没有单独设置它
    entry->Father_Entry = tableManager.cur;
    entry->return_error = true;//只有在扫描时是正确的才可以解除错误   int函数最后要return
    return entry;
}

//函数形式参数
//声明
void Semantic::recordEntries(vector<Entry *> &entries) {
    tableManager.insertIntoTable(entries);
}

void Semantic::recordEntries(Entry * entry) {
    tableManager.insertIntoTable(entry);
}


void Semantic::skip(PIECE piece) {
    switch (piece) {
        case CONSTDECL:
        case VARDECL:
           //缺少分号 直接已经指向下一句的字符了
            break;
        case CONSTDEF:
        case VARDEF:
            while(WORD_TYPE != COMMA){ // 因为整一句都会失效
                PRINT_WORD;
                GET_A_WORD;
            }
            break;
        case FUNCDEF:
            break;
        case MAINFUNCDEF:
            break;
        case FUNCFPARAM:
            break;
        case STMT:
            break;
        case LVAL:
            break;
        case UNARYEXP:
            break;
        case FORMATSTRING:
            break;
        default:
            break;

    }
}
