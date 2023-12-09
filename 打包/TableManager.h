//
// Created by hzt on 2023/10/12.
//
#include <string>
#include <map>
#include <vector>
#include "MipsCode.h"

using namespace std;
#ifndef LEX_TABLEMANAGER_H
#define LEX_TABLEMANAGER_H
#include "ErrorHandler.h"
//struct fParam{ //形参
//    //    Id_Type  c++不支持嵌套结构体 互相包含定义？
//    int op;//0 1 2
//    string ident;
//};

enum Kind{
    CompUnit,
    VAR,
    CONST,
    ARRAY_1_CONST,
    ARRAY_2_CONST,
    ARRAY_1_VAR,
    ARRAY_2_VAR,
    FUNC_INT,
    FUNC_VOID,

    FOR,
    IF,
    ELSE,

    BLOCK
    //函数形式参数 采取用vector计数
};
//struct Id_Type{
//    Kind kind;
//    vector<fParam> *fParams;
//};

//Entry
class Entry{
public:
    string ident;//标识符名称
    Kind kind;
    vector<Entry*> *fParams;
    int loop_count;
    bool return_error;
//    Id_Type type;//标识符类型

//暂且不实现值的符号记录
    int value;
    vector<int> values; //值
    int dim1_length;

    //中间代码生成时的
    int id;

    map<string, Entry*>  *entries;
    struct Entry * Father_Entry;
};



//整个程序的初始表项  树的root   不需要定义在这里
class TableManager {
private:

    ErrorHandler & errorHandler;
    //当前表(info 相当于查表时是比cur小一级的一个表项的    填表时 也是)

public:
    // 表项指针，用于符号表管理器与外界交换信息。
    // 查表时，将查到的表项放在这里；
    // 填表时，将待填的表项放在这里。
    Entry * info;//下一条即将插入的表项 用来存储信息
    Entry * cur;
    explicit TableManager(ErrorHandler &);//构造函数
    void downTable(string & ident);//向下一级  用于main函数调用全局函数时进入该函数的符号表  亦或是一个block

    void upTable();//向上一级 用于从被调用函数返回  符号表联动 在顶层进行联动

    [[nodiscard]] bool isRedefine(const string& ident) const;//检查重定义的错误
    //检查未定义就引用的错误
    bool isEverDefine(const string& ident,Kind kind) ;

    void insertIntoTable(vector<Entry*> &entries) const;//插入当前层的符号表  统一都是插入到这一级Entry的entries
    void insertIntoTable(Entry * entry) const;//插入entry层
};


#endif //LEX_TABLEMANAGER_H
