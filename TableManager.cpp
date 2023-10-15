//
// Created by hzt on 2023/10/12.
//
/**
 *
 * 遵循单一职责原则：
 * 符号表管理类  毕竟叫管理了   所以也要有对符号表的操作
 * 提供一整套Entry树形结构
 * 由Semantic进行调用其中的函数操作 ：
 * 1.向下一层扩充指定子符号表  ident
 * 2.向上一层回到父符号表
 * 3.检查当前符号表是否已经存在ident名字
 * 4.检查是否存在已经定义的对应符号 包括ident 和type  ??
 * 5.将若干表项插入当前父符号表  vector<Entry*> arguments
 */
#include <iostream>
#include "TableManager.h"
#include "Semantic.h"//怎么让符号表管理器和语义分析共同包含各自  只能后续赋值？

TableManager::TableManager(ErrorHandler & errorHandler1) :errorHandler(errorHandler1){//?
    info = nullptr;//fill会初始化
    cur = new Entry;
    //initialize cur      info can be initialized in semantic's fill_
    cur->entries = new map<string , Entry*>;
    cur->ident = "CompUnit";
    cur->loop_count = 0;
    cur->fParams = nullptr;
    cur->kind = CompUnit;
    cur->Father_Entry = nullptr;
    cur->return_error = false;
}

bool TableManager::isRedefine(const string &ident) const{//can be made const????
    //如果是函数  局部变量区entries和参数区cur->arguments
    //
    if(cur->entries->find(ident) == cur->entries->end()){
        return false;
    }
    return true;
}


void TableManager::upTable() { //cur 应该作为一个pointer  可以省去重载=  同时方便使用
    cur = cur->Father_Entry;
}

void TableManager::insertIntoTable(vector<Entry *> & entries) const { //插入当前表  已经find过  未定义的
    if (cur->entries == nullptr){
        cur->entries = new map<string,Entry*>;
    }
    for (auto & entry : entries) {
        cur->entries->insert(pair<string,Entry*>(entry->ident, entry));
    }
}



//其实到这里不用参数也可以
//为什么一定需要有一个downTable函数呢  也就是为什么需要记录一个块里面的符号  是因为这个块我门不确定是否还会有内嵌块可能会用到这个块的变量 所以需要记录
void TableManager::downTable(string& ident) { //进入以标识符唯一标识的符号表  似乎只有函数可能会用到  预先定义的
    if(cur->entries->find(ident) != cur->entries->end()){
        cur = cur->entries->at(info->ident);
    }
}

bool TableManager::isEverDefine(const string& ident,Kind kind,bool isLeft)  {
    Entry * temp = cur;
    Entry * e;
    while(temp != nullptr){//设置顶层entry的父节点为空
        if(temp->entries->find(ident) != temp->entries->end() ){//对于函数  形参放进了entries中   设置完参数后会将设置cur的函数的参数类型设置
            e = temp->entries->at(ident);

            if (e->kind == kind
            || kind == FUNC_INT && e->kind == FUNC_VOID
            || kind == VAR && e->kind == CONST
            || kind == ARRAY_1_VAR && e->kind == ARRAY_1_CONST
            || kind == ARRAY_2_VAR && e->kind == ARRAY_2_CONST){ //函数类型返回值的错误  先认为都是函数即可
                if(isLeft &&
                (kind == VAR && e->kind == CONST
                   || kind == ARRAY_1_VAR && e->kind == ARRAY_1_CONST
                   || kind == ARRAY_2_VAR && e->kind == ARRAY_2_CONST)){
                    errorHandler.Print_Error(CONST_LEFT);//查找定义时直接完成左值不能是常量的报错
                }
                return true;
            }
        }
    }
    return false;
}

void TableManager::insertIntoTable(Entry *entry) const {
    if (cur->entries == nullptr){
        cur->entries = new map<string,Entry*>;
    }
   cur->entries->insert(pair<string,Entry*>(entry->ident,entry));
}
