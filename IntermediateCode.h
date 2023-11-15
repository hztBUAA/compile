//
// Created by hzt on 2023/11/11.
//

#ifndef LEX_INTERMEDIATECODE_H
#define LEX_INTERMEDIATECODE_H

#include <string>
#include <utility>
#include <ostream>
#include <iostream>
#include <map>
#include "vector"

using namespace std;
extern bool isInOtherFunc;
extern string funcLabel;


/**
 * Assign是一对一的赋值
 */
enum IntermediateCodeType{
    ARRAY_CONST_Def_Has_Value,
    ARRAY_VAR_Def_Has_Value,
    VAR_Def_Has_Value,
    Const_Def_Has_Value,
    ARRAY_Def_No_Value,
    VAR_Def_No_Value,

    Printf,
    Add,
    Sub,
    Mult,
    Div,
    Mod,
    Assign,//还是需要的  LVal = Exp中  ---想了想还是不需要  直接把iEntry放进去即可 TODO:really?  好像不是这样子的！  会引起按值传递时的错误
    GetInt,//只有dst参数   表示放进dst
    GetArrayElement,
    FuncCall,//调用时要进行参数压栈和临时内存上堆
    FuncDef,//定义函数时 形式参数需要当成local定义   进入符号表生成Entry的同时生成IEntry 并记录两者的映射
};


//为了方便运算的中间变量  可以为值（算出来的时候） 也可以
//只会在一条高级语言的生成过程中存活
/**
 * IEntry需要记录所有在MIPS中生成最终代码的所有信息   我可以认为id就是信息
 * 需要把定义array时的generate_id存进符号表项时
 * IEntry需要存 1.源程序不出现的临时计算中间变量
 *                  不需要存（不需要拷贝）2.源程序中定义时加进符号表的定义（有名字变量）可以把当时分配的IEntry‘s id 放进Entry中  记录内存抽象（IEntry是存在的 但它只负责内存抽象）
 *                                                      3.但是需要在中间代码生成器中有一个数据结构   这里面涉及到一些需要手动设置是temp的？   直接在IEntry里面存放Entry
 *                             这一块暂时是不需要中间代码阶段去担心的        IEntry抽象了临时数据区的栈结构 内存
 *
 *                             中间代码ICode的内部组件即使IEntry
 */

class IEntry{
private:
    int tempMemoryAddressTop = 268500992;
public:
    int Id;
    int type;//0定位元素 1维地址 2表示函数调用FuncCALL--
    string name;
    int imm;//立即数或者已经算出  不能直接再被用来计算 由于另一个操作区待定
    bool canGetValue{};

    int total_length;
    int dim1_length;
    vector<int> *values;//存储数组的值   -----假想的会放在对应的Address位置上
    vector<int > *values_Id;
    int startAddress;//用来跟MIPS的后端对接  ---由于数组的存在地址不是和id一一对应  FIXME:type = 1时只能使用startAddress 和offset  startAddress是数组的首地址  offset是偏移量  函数形参中的数组也是这样的  type判断1时要加上offset+startAddress  否则就是直接对应的地址
    IEntry * offset_IEntry;//配合地址使用  只有type = 1时成立
    bool isGlobal;

    //---------FuncCall---------
    /**
     * 类似于Entry  包含id即可  id是查询符号表时已经有对应的id在对应的Entry（定义该变量时就存在的）
     *
     */
    //--------FuncDef------
    //FIXME:函数形参 需要留好位置IEntry并与Entry进行映射   函数调用时 会将实参对应IEntry（已经放进函数调用中间代码的IEntry中的value_Id）的数据Assign到形参区有后端完成
    bool has_return{};
    string original_Name;

    //------printf-------//
    vector<int> * strings_iEntry_id;
    string str;

    static int generateId();
    IEntry();
    explicit IEntry(int length);

};

struct ICode{
    IntermediateCodeType type;
    IEntry* src1;
    IEntry* src2;
    IEntry* dst;
//    int address;
};


class IntermediateCode {
private:

    /**
     * 定义一条中间代码类型
     * 定义一条中间代码中的小部件
     */
public:
    vector<int> *strings = new std::vector<int>;
    vector<ICode *> mainICodes;
    map<string, vector<ICode *>> otherFuncICodes;
    ICode *iCode;

    vector<ICode *> globalDef;

    static string iCode2str(ICode *iCode);

    void addDef(bool isGlobal, IntermediateCodeType type, IEntry *src1, IEntry *src2, IEntry *dst);

    void addICode(IntermediateCodeType type, IEntry *src1, IEntry *src2, IEntry *dst);

    void addICode(IntermediateCodeType type, int src1, IEntry *src2, IEntry *dst);

    void debug_print();
};
extern vector<IEntry *> IEntries;
#endif //LEX_INTERMEDIATECODE_H
