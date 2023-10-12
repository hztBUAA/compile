//
// Created by hzt on 2023/10/12.
//
#include <string>
#include <map>
#include <vector>

using namespace std;
#ifndef LEX_TABLEMANAGER_H
#define LEX_TABLEMANAGER_H

struct fParam{ //形参
    //    Id_Type  c++不支持嵌套结构体 互相包含定义？
    int op;//0 1 2
    string ident;
};


struct Id_Type{
    bool isVariety;
    int op;//0 1 2  维数
    bool isFunc;
    bool retIsInt;
    vector<fParam> fParams;
};

//Entry
struct Entry{
    string ident;//标识符名称
    Id_Type type;//标识符类型
    map<string, Entry*>  entries;
    struct Entry * Father_Entry;
};

//整个程序的初始表项  树的root   不需要定义在这里
class TableManager {
private:
    // 表项指针，用于符号表管理器与外界交换信息。
    // 查表时，将查到的表项放在这里；
    // 填表时，将待填的表项放在这里。
    Entry * info;//下一条即将插入的表项 用来存储信息

    //当前表(info 相当于查表时是比cur小一级的一个表项的    填表时 也是)
    Entry * cur;
public:
    TableManager();//构造函数
    void downTable();//向下一级  用于main函数调用全局函数时进入该函数的符号表  亦或是一个block

    void upTable();//向上一级 用于从被调用函数返回  符号表联动 在顶层进行联动

    bool canFind();//检查未定义就引用的错误

    void insertIntoTable();//插入当前层的符号表  统一都是插入到这一级Entry的entries

};


#endif //LEX_TABLEMANAGER_H
