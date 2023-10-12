//
// Created by hzt on 2023/10/12.
//

#include "TableManager.h"

TableManager::TableManager() {//?
    info = nullptr;
    cur = nullptr;
}

bool TableManager::canFind() {
    if(cur->entries.find(info->ident) == cur->entries.end()){
        return false;
    }
    return true;
}


void TableManager::upTable() { //cur 应该作为一个pointer  可以省去重载=  同时方便使用
    cur = cur->Father_Entry;
}

void TableManager::insertIntoTable() { //插入当前表  已经find过  未定义的
    cur->entries.insert(pair<string,Entry*>(info->ident,info));
}

//其实到这里不用参数也可以
void TableManager::downTable() { //进入以标识符唯一标识的符号表  似乎只有函数可能会用到  预先定义的
    if(cur->entries.find(info->ident) != cur->entries.end()){
        //find  and should be find !  before will use can find
        cur = cur->entries.at(info->ident);
    }
}
