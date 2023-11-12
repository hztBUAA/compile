//
// Created by hzt on 2023/11/11.
//

#ifndef LEX_INTERMEDIATECODE_H
#define LEX_INTERMEDIATECODE_H

#include <string>
#include <utility>
#include "vector"
#include "Parser.h"
using namespace std;



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
int tempMemoryAddressTop = 268500992;
int id_generate = 0;


class IEntry{
public:
    int Id{};
    int type{};//0定位元素 1维地址
    string name;
    int imm{};//立即数或者已经算出  不能直接再被用来计算 由于另一个操作区待定
    bool canGetValue{};

    int startAddress;//用来跟MIPS的后端对接  ---由于数组的存在地址不是和id一一对应


    //---------FuncCall---------
    /**
     * 类似于Entry  包含id即可  id是查询符号表时已经有对应的id在对应的Entry（定义该变量时就存在的）
     *
     */
    vector<int > *RParams;
    static int generateId(){
        return id_generate++;
    }
    IEntry(){
        this->Id = generateId();
        this->startAddress = tempMemoryAddressTop;
        this->name = "@"+ to_string(this->Id);
        tempMemoryAddressTop += 4;
//         iEntry->address
    }

    explicit IEntry(int length){
        this->Id = generateId();
        this->startAddress = tempMemoryAddressTop;
        this->name = "@"+ to_string(this->Id);
        tempMemoryAddressTop += length*4;
//         iEntry->address
    }
};

/**
 * Assign是一对一的赋值
 */
enum IntermediateCodeType{
    Add,
    Sub,
    Mult,
    Div,
    Mod,
//    Assign,//还是需要的  LVal = Exp中  ---想了想还是不需要  直接把iEntry放进去即可
    GetInt,//只有dst参数   表示放进dst
    GetArrayElement,
    FuncCall,//调用时要进行参数压栈和临时内存上堆
    FuncDef,//定义函数时 形式参数需要当成local定义   进入符号表生成Entry的同时生成IEntry 并记录两者的映射
};

struct ICode{
    IntermediateCodeType type;
    IEntry* src1;
    IEntry* src2;
    IEntry* dst;
//    int address;
};
static int id = 0;
class IntermediateCode {
    /**
     * 定义一条中间代码类型
     * 定义一条中间代码中的小部件
     */
public:
     vector<IEntry*>IEntries;
    vector<ICode*> mainICodes;
    map<string,vector<ICode*>> otherFuncICodes;
    ICode * iCode;



public:
    void addICode(IntermediateCodeType type,IEntry* src1,IEntry* src2,IEntry* dst){
        iCode = new ICode();
        dst = new IEntry;//最后翻译时 会根据约定的IEntryType去使用IEntry
        iCode->type = type;
        iCode->src1 = src1;
        iCode->src2 = src2;
        iCode->dst = dst;
        if (isInOtherFunc){
            otherFuncICodes.at(funcLabel).push_back(iCode);
        }else{
            mainICodes.push_back(iCode);
        }
    }
    void addICode(IntermediateCodeType type,int src1,IEntry* src2,IEntry* dst){
        iCode = new ICode();
        dst = new IEntry;
        iCode->type = type;
        auto* s1 = new IEntry;
        s1->imm = src1;
        iCode->src1 = s1;
        iCode->src2 = src2;
        iCode->dst = dst;
        if (isInOtherFunc){
            otherFuncICodes.at(funcLabel).push_back(iCode);
        }else{
            mainICodes.push_back(iCode);
        }
    }
//    void addICode(IntermediateCodeType type,IEntry* src1,int src2,IEntry* dst){
//        iCode = new ICode();
//        iCode->type = type;
//        iCode->src1 = src1;
//        auto* s2 = new IEntry;
//        s2->imm = src2;
//        iCode->src2 = s2;
//        iCode->dst = dst;
//        mainICodes.push_back(iCode);
//    }

};


#endif //LEX_INTERMEDIATECODE_H
