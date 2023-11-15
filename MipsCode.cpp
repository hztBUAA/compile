//
// Created by hzt on 2023/11/11.
//


#include <map>
#include <string>
#include "IntermediateCode.h"
#include "MipsCode.h"
using namespace std;


void MipsCode::assign(IEntry *src1,IEntry *src2,IEntry *dst) {
    /*
     * 值传递  地址赋
     * */
    if (src1->type == 0){
        if (src1->canGetValue){
            dst->canGetValue = true;
            dst->imm = src1->imm;
        }else{
            cout << "lw " << "$t0, " << src1->startAddress<<"$(zero)"<<endl;
            cout << "sw " << "$t0, " << dst->startAddress<<"$(zero)"<<endl;
        }
    }else{
        dst->canGetValue = false;
        dst->values_Id = src1->values_Id;
        dst->values = src1->values;//none sense
        dst->type = 1;
        dst->total_length = src1->total_length;
        dst->dim1_length = src1->dim1_length;
        dst->offset_IEntry = src1->offset_IEntry;
        //FIXME:其他属性就不用拷贝的？   理清楚？ isGlobal保持自己 has_return 不可能
    }

}


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

        vector<int> *rParam_ids = src2->values_Id;
        vector<int> *fParam_ids = src1->values_Id;

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
                assign(src1, nullptr,dst);
                break;
                //FIXME:总是容易陷入误区 得到v0的值已经是运行时  编译的极限块也做不到预知~
            case GetInt:
                cout << "\nli $v0, 5\n";
                cout<<"syscall\n";
                cout << "sw " << "$v0" << ", " << dst->startAddress << "($zero)"<< endl;
                break;
                //TODO：检查格式统一 全都是IEntry格式   可以进行一个canGetElement的优化
            case GetArrayElement:{
                int index = 0;
                int isNormalArray = src2->type;
                if (isNormalArray){//表示array并不是通过函数传递地址而来  即offsetEntry没用 或者认为就是0 即index就是最终索引
                    if (src1->canGetValue){//array就是数组首地址 index索引知道是第几个元素
                        index += src1->imm;
                        if (IEntries.at(src2->values_Id->at(index))->canGetValue){
                            dst->canGetValue = true;
                            dst->imm = IEntries.at(src2->values_Id->at(index))->imm;
                        }else{
                            //编译时 index可以得到准确值 但是那个元素需要getint运行时获得  getint执行时会le sw 与之对应
                            dst->canGetValue = false;
                            cout << "lw " << "$t0" << ", " << IEntries.at(src2->values_Id->at(index))->startAddress << "($zero)" << endl;
                            cout << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                        }
                    }else{//array就是数组首地址 index索引还不知道是第几个元素 即索引也是取决于getint  需要lw
                        cout << "li " << "$t0" << ", " << src2->startAddress << endl;
                        cout << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        cout << "addu " << "$t2" << ", "  << "$t0" << ", " << "$t1"<< endl; //value's address in $t2
                        cout << "lw " << "$t3" << ", 0($t2)" << endl;
                        cout << "sw " << "$t3" << dst->startAddress << "($zero)" << endl;
                    }
                }else{//不是normal  出现在自定义函数内部的引用数组  此时src2 会是startAddress offset_Entry
                    IEntry* offset = src2->offset_IEntry;
                    if (offset->canGetValue){ //引用数组的索引 已知
                        index += offset->imm;
                        if (src1->canGetValue){
                            index += src1->imm;
                            if (IEntries.at(src2->values_Id->at(index))->canGetValue){
                                dst->canGetValue = true;
                                dst->imm = IEntries.at(src2->values_Id->at(index))->imm;
                            }else{
                                //编译时 index可以得到准确值 但是那个元素需要getint运行时获得  getint执行时会le sw 与之对应
                                dst->canGetValue = false;
                                cout << "lw " << "$t0" << ", " << IEntries.at(src2->values_Id->at(index))->startAddress << "($zero)" << endl;
                                cout << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                            }
                        }else{ //额外索引值是getint 函数调用的引用时
                            cout << "li " << "$t0" << ", " << src2->startAddress << endl;
                            cout << "li " << "$t1" << ", " << index * 4 << endl;//此时index只有offset->imm  再加上src1的值
                            cout << "lw " << "$t2" << ", " << src1->startAddress << "($zero)" << endl;

                            cout << "addu " << "$t3" << ", "  << "$t0" << ", " << "$t1"<< endl;
                            cout << "addu " << "$t3" << ", "  << "$t3" << ", " << "$t2"<< endl; //value's address in $t3
                            cout << "lw " << "$t4" << ", 0($t3)" << endl;
                            cout << "sw " << "$t4" << dst->startAddress << "($zero)" << endl;
                        }
                    }else{ //有时引用数组的索引都是getint  arr[t] ||||||   sll rd rt sham: rt » sham => rd, shift left logical 向左移位
                        cout << "lw " << "$t0" << ", " << src2->offset_IEntry->startAddress<<"$(zero)" << endl;//t in $t0
                        cout << "sll " << "$t0"<< ", "<< "$t0"<< " 2";//$t0
                        if (src1->canGetValue){
                            cout << "li " << "$t1" << ", " << src1->imm  << endl;// in $t1
                        }else{
                            cout << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;// in $t1
                        }
                        cout << "addu " << "$t2" << ", "  << "$t0" << ", " << "$t1"<< endl; // in $t2
                        cout << "li " << "$t3" << ", " << src2->startAddress << endl;
                        cout << "addu " << "$t3" << ", "  << "$t3" << ", " << "$t2"<< endl; //value's address in $t3
                        cout << "lw " << "$t3" << ",  0($t3)" << endl;
                        cout << "sw " << "$t3" << dst->startAddress << "($zero)" << endl;
                    }
                }
                break;
            }
                /**
                     * 将实参的  值 地址 按格式给形参   IN src1->valuesId  存储了对应的形参时生成的IEntry
                     */
            //TODO:函数的格式理解  sp  压栈~虚拟？  IEntry:has_return?
            case FuncCall:
                if (rParam_ids->size() != fParam_ids->size()){
                    cout << "error!!!!\n";
                }
                cout << "#调用函数" << src1->original_funcName<< ": ";
                for (int i = 0; i < rParam_ids->size();i++){
                    assign(IEntries.at(rParam_ids->at(i)), nullptr,IEntries.at(fParam_ids->at(i)));
                }
                cout << endl;
                break;

                /**
                 * 函数名标签  就是函数头的名字  形参的IEntry需要在中间代码就生成
                 */
            case FuncDef:
                cout << "#" << src1->original_funcName<< "部分: ";
                for (auto id: *src1->values_Id) {
                    if(IEntries.at(id)->type == 0){
                        cout <<"value:@" << id <<" ";
                    }else{
                        cout <<"address:@" << id <<" ";
                    }
                }
                cout << endl;
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


