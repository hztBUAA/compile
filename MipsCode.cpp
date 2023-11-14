//
// Created by hzt on 2023/11/11.
//


#include <map>
#include <string>
#include "IntermediateCode.h"
#include "MipsCode.h"
using namespace std;

void MipsCode::translate() const {
    vector<ICode *> mainCodes = intermediateCode.mainICodes;
    map<string, vector<ICode *>> otherFuncICodes = intermediateCode.otherFuncICodes;
    vector<ICode *> globalDefs = intermediateCode.globalDef;

    /**
     * 输出全局的变量定义data段  以及全局变量的初始化
     */
    cout << ".data 0x10010000\n";

    cout << "temp:  .space  160000\n\n";  // 临时内存区，起始地址为0x10010000 (16) or 268500992 (10)

    for (auto def:globalDefs) {
        //形如 def src1
        IntermediateCodeType type = def->type;
        //TODO:名字需要注意  是否在语法分析时准备好名字
        switch (type) {
            case VAR_Def_Has_Value:
                cout<< "var_@"+ to_string(def->src1->Id) <<":  .word  " ;
                for (auto id_init_value:*(def->src1->values_Id)) {
                    cout << IEntries.at(id_init_value )->imm<< " ";
                }
                cout << endl;
                break;
            case VAR_Def_No_Value:
                cout<< "var_@"+ to_string(def->src1->Id) <<":  .word  " ;
                for (int i = 0;i<def->src1->total_length;i++) {
                    cout << "0 ";
                }
                cout << endl;
                break;
            case ARRAY_VAR_Def_Has_Value:
                cout<< "array_@"+ to_string(def->src1->Id) <<":  .word  " ;
                for (auto id_init_value:*(def->src1->values_Id)) {
                    cout << IEntries.at(id_init_value)->imm << " ";
                }
                cout << endl;
                break;
            case ARRAY_Def_No_Value:
                cout<< "array_@"+ to_string(def->src1->Id) <<":  .word  " ;
                for (int i = 0;i<def->src1->total_length;i++) {
                    cout << "0 ";
                }
                cout << endl;
                break;
            case Const_Def_Has_Value:
                cout<< "const_@"+ to_string(def->src1->Id) <<":  .word  " ;
                for (auto init_value:*(def->src1->values)) {
                    cout << init_value << " ";
                }
                cout << endl;
                break;
            case ARRAY_CONST_Def_Has_Value:
                cout<< "array_@"+ to_string(def->src1->Id) <<":  .word  " ;
                for (auto init_value:*(def->src1->values)) {
                    cout <<init_value << " ";
                }
                cout << endl;
                break;
            default:
                break;
        }
    }
    /**
     * 输出主函数main的代码ICode
     */
     cout<<"#主函数main的代码ICode\n";
     cout<<"\nmain:\n";
    for (auto ICode: mainCodes) {
        IntermediateCodeType type = ICode->type;
        IEntry *src1 = ICode->src1;
        IEntry *src2 = ICode->src2;
        IEntry *dst= ICode->dst;

        switch (type) {
            case Add:{
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm + src2->imm;
                }else {
                    if (src1->canGetValue){
                        cout << "li " << "$t0" << ", " << src1->imm << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        cout << "add " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        cout << "li " << "$t0" << ", " << src2->imm << endl;
                        cout << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        cout << "add " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        cout << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        cout << "add " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                    }
                }
            }
                break;
            case Sub:{
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm - src2->imm;
                }else {
                    if (src1->canGetValue){
                        cout << "li " << "$t0" << ", " << src1->imm << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        cout << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        cout << "li " << "$t0" << ", " << src2->imm << endl;
                        cout << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        cout << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        cout << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        cout << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                    }
                }
            }
                break;
            case Mult:{
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm * src2->imm;
                }else {
                    if (src1->canGetValue){
                        cout << "li " << "$t0" << ", " << src1->imm << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        cout << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        cout << "li " << "$t0" << ", " << src2->imm << endl;
                        cout << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        cout << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        cout << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        cout << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                    }
                }
            }
                break;
            //除法：HI存放余数，LO存放除法结果
            case Div:{
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm / src2->imm;
                }else {
                    if (src1->canGetValue){
                        cout << "li " << "$t0" << ", " << src1->imm << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        cout << "div " << "$t0" << ", " << "$t1" << endl;
                        cout << "mflo " << "$t2" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        cout << "li " << "$t0" << ", " << src2->imm << endl;
                        cout << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        cout << "div " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "mflo " << "$t2" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        cout << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        cout << "div " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "mflo " << "$t2" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                    }
                }
            }
                break;
            //TODO:优化
            case Mod:{
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm % src2->imm;
                }else {
                    if (src1->canGetValue){
                        cout << "li " << "$t0" << ", " << src1->imm << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        cout << "div " << "$t0" << ", " << "$t1" << endl;
                        cout << "mfhi " << "$t2" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        cout << "li " << "$t0" << ", " << src2->imm << endl;
                        cout << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        cout << "div " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "mfhi " << "$t2" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        cout << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        cout << "div " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        cout << "mfhi " << "$t2" << endl;
                        cout << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                    }
                }
            }
                break;
            //FIXME:一定是地址？
            case Assign:
                if (src1->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm;
                }else {
                    cout << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                    cout << "sw " << "$t0" << ", " << dst->startAddress << "($zero)"<< endl;
                }
                break;
                //FIXME:总是容易陷入误区 得到v0的值已经是运行时  编译的极限块也做不到预知~
            case GetInt:
                cout << "\nli $v0, 5\n";
                cout<<"syscall\n";
                cout << "sw " << "$v0" << ", " << dst->startAddress << "($zero)"<< endl;
                break;
                //TODO：检查格式统一 全都是IEntry格式   可以进行一个canGetElement的优化
            case GetArrayElement:{
                if (src2->canGetValue){
                    //TODO:   mistake:   src2是基地址base   直接编译时取到了对应index的array元素值  则不会生成现在GetArrayElement代码
                    ;
                }else if (src1->canGetValue ){
                    //TODO：需要分辨全局数组  要用标签   其他则可以直接编译放在temp内存
//                    cout << "li " << "$t0" << ", " << src1->imm << endl;
                    if (src2->isGlobal){
                        cout << "lw " << "$t1" << ", " << "array@"+ to_string(src2->Id) << endl;
                    }else{
                        cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                    }
                    //TODO：这里关于数组取元素 理清楚src1   src2 dst的原理
                    //FIXME:丑陋
                    cout << "lw " << "$t2" << ", " << (src2->startAddress + src1->imm * 4) << "($zero)" << endl;
                    cout << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                }else {
                    cout << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                    cout << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                    cout << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                    cout << "lw " << "$t2" << ", " << "0($t2)" << endl;
                    cout << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                }
            }
                break;
            //TODO:函数的格式理解  sp  压栈~虚拟？  IEntry:has_return?
            case FuncCall:
                cout << "funcCall " << ICode->dst->name << ", " << ICode->src1->name << ", " << ICode->src2->name << endl;
                break;
            case FuncDef:
                cout << "funcDef " << ICode->dst->name << ", " << ICode->src1->name << ", " << ICode->src2->name << endl;
                break;
                /**
                 * 非全局变量的初始化定义
                 */
            case VAR_Def_Has_Value:
                cout<< "#local_var_@"+ to_string(ICode->src1->Id) <<"_def:  " ;
                if(IEntries.at(src1->values_Id->at(0))->canGetValue){
                    //                    src1->imm = IEntries.at(src1->values_Id->at(0))->imm;
                    //                    src1->canGetValue = true;
                    cout<<IEntries.at(src1->values_Id->at(0))->imm << " ";
                }else{
                    //                    cout << "";
                    //                    cout << "lw " << "$t0" << ", " << (IEntries.at(src1->values_Id->at(0))->startAddress) << "($zero)" << endl;
                    //                    cout << "sw " << "$t0" << ", " << (src1->startAddress) << "($zero)" << endl;
                    cout<<"@("<<src1->values_Id->at(0)<<")"<< " ";
                }
                cout <<endl;
                break;
            case VAR_Def_No_Value:
                cout<< "#local_var_@"+ to_string(ICode->src1->Id) <<"no_value_def\n  " ;
                break;
            case ARRAY_VAR_Def_Has_Value:
                cout<< "#local_array_@"+ to_string(ICode->src1->Id) <<"_def:  " ;
                for (auto id_init_value:*(src1->values_Id)) {
                    if (IEntries.at(id_init_value)->canGetValue){
                        ;//FIXME:编译时可以得出的值  以后要用就再按需取
                        cout<<IEntries.at(id_init_value)->imm<<" ";
                    }else{
                        cout<<"@("<<id_init_value<<")" << " ";
                        ;//FIXME:编译时不可以得出的值  说明运行后通过getint的变量间接得到值  以后要用就再按需lw sw取  初始值IEntry是有若干个IEntry组成的 可以进行canGetValue的判断
                    }
                }
                cout<<endl;
                break;
            case ARRAY_Def_No_Value:
                cout<< "#local_array_@"+ to_string(ICode->src1->Id) <<"_def\n  " ;
                break;
            case Const_Def_Has_Value:
                cout<< "#const_@"+ to_string(ICode->src1->Id) <<"_def:  " ;
                for (auto init_value:*(src1->values)) {
                    cout<<init_value<<" ";
                }
                cout<<endl;
                break;
            case ARRAY_CONST_Def_Has_Value:
                cout<< "#array_const@"+ to_string(ICode->src1->Id) <<"def   " ;
                for (auto init_value:*(src1->values)) {
                    cout<<init_value<<" ";
                }
                cout<<endl;
                break;
            default:
                break;

        }
    }


    /**
     * 输出其他函数的代码ICode
     */
    cout<<"#自定义函数main的代码ICode\n";
    for (auto ICode: otherFuncICodes) {
    ;
    }

}


