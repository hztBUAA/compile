//
// Created by hzt on 2023/11/11.
//


#include <map>
#include <string>
#include "IntermediateCode.h"
#include "MipsCode.h"
#include <fstream>

using namespace std;
extern vector<ICode *> mainICodes ;
extern map<string, vector<ICode *>> otherFuncICodes;
extern vector<ICode *>globalDef ;
std::ofstream outputFile("mips.txt");





void MipsCode::assign(IEntry *src1,IEntry *src2,IEntry *dst) { //传进来需要已经values_Id处理好
    /*
     * 值传递  地址赋
     * TODO:需要对全局变量的分类讨论   局部变量也是    写入值编译时不确定时
     * TODO：多次写入的更新原则！！！
     * */
    outputFile << "#assign" << endl;
    if (src1->type == 0){
        if (src1->canGetValue){
            outputFile << "li " << "$t1, " << src1->imm<<endl;
        }else{
            outputFile << "lw " << "$t1, " << src1->startAddress<<"($zero)"<<endl;
        }
        if (dst->type == 2){
            outputFile << "lw " << "$t2, " <<dst->startAddress<<"($zero)"<<endl;
            outputFile << "sw " << "$t1, " <<"0($t2)"<<endl; //dst是值   需要从变量的IEntry的values_Id中取得的  t0  地址  为写语句而生
        }else{
            outputFile << "sw " << "$t1, " << dst->startAddress<<"($zero)"<<endl; //dst是值   需要从变量的IEntry的values_Id中取得的
        }
            dst->canGetValue =  false;//后台更新
    }else{
        //赋值的地址参数拷贝！！！
        outputFile << "#地址拷贝\n";
        dst->canGetValue = false;
        dst->isGlobal = src1->isGlobal;
        dst->values_Id = src1->values_Id;
        dst->values = src1->values;//none sense
        dst->type = 1;
        dst->total_length = src1->total_length;
        //dst->dim1_length = src1->dim1_length;//以FParam来标准 不需要传递
        dst->offset_IEntry = src1->offset_IEntry;
        dst->startAddress = src1->startAddress;//关于数组地址的属性都要拷贝
        //FIXME:其他属性就不用拷贝的？   理清楚？ isGlobal保持自己 has_return 不可能
    }

}

void MipsCode::testRe(){
    // 创建一个ofstream对象

// 保存outputFile的原始缓冲区指针
//    std::streambuf* outputFileBuffer = std::outputFile.rdbuf();

// 将outputFile的流重定向到outputFile
//    std::outputFile.rdbuf(outputFile.rdbuf());
    outputFile << "text\n";
    // 重定向结束后，可以将outputFile的流恢复到原始状态
//    std::outputFile.rdbuf(outputFileBuffer);
}
void MipsCode::translate() const {
// 创建一个ofstream对象
    std::ofstream outputFile("mips.txt");

// 保存outputFile的原始缓冲区指针
//    std::streambuf* outputFileBuffer = std::outputFile.rdbuf();

// 将outputFile的流重定向到outputFile
//    std::outputFile.rdbuf(outputFile.rdbuf());


    /**
     * 输出全局的变量定义data段  以及全局变量的初始化
     */
    outputFile << ".data 0x10010000\n";
//    outputFile << ".data 0x1000\n";

    outputFile << "temp:  .space  160000\n\n";  // 临时内存区，起始地址为0x10010000 (16) or 268500992 (10)
    /**字符串区
# string tokens:
str_1:  .asciiz   "hello!"
str_3:  .asciiz   "haha"
str_5:  .asciiz   "ha"

     */
     outputFile << "#strings in printf\n";
    for (auto id: strings) {
        outputFile << "str_" << id << ": .asciiz " << "\"" << IEntries.at(id)->str << "\"" << endl;
    }
    for (auto def:globalDef) {
        //形如 def src1
        IntermediateCodeType type = def->type;
        //TODO:名字需要注意  是否在语法分析时准备好名字
        switch (type) {
            case VAR_Def_Has_Value:
                outputFile << def->src1->original_Name << ":  .word  " ;
                for (auto id_init_value:*(def->src1->values_Id)) {
                    outputFile << IEntries.at(id_init_value )->imm << " ";
                }
                outputFile << endl;
                break;
            case VAR_Def_No_Value:
                outputFile << def->src1->original_Name << ":  .word  " ;
                for (int i = 0;i<def->src1->total_length;i++) { //单个普通全局变量
                    outputFile << "0 ";//此时输出值也会是0 在语法分析部分进行了判断补充
                }
                outputFile << endl;
                break;
            case ARRAY_VAR_Def_Has_Value:
                outputFile << def->src1->original_Name << ":  .word  " ;
                for (auto id_init_value:*(def->src1->values_Id)) {
                    outputFile << IEntries.at(id_init_value)->imm << " ";
                }
                outputFile << endl;
                break;
            case ARRAY_Def_No_Value:
                outputFile << def->src1->original_Name << ":  .word  " ;
                for (int i = 0;i<def->src1->total_length;i++) {
                    outputFile << "0 ";
                }
                outputFile << endl;
                break;
            case Const_Def_Has_Value:
                outputFile << def->src1->original_Name << ":  .word  " ;
                for (auto id_value:*(def->src1->values_Id)) {
                    outputFile << IEntries.at(id_value)->imm << " ";
                }
                outputFile << endl;
                break;
            case ARRAY_CONST_Def_Has_Value:
                outputFile<<  def->src1->original_Name <<":  .word  " ;
                for (auto id_value:*(def->src1->values_Id)) {
                    outputFile <<IEntries.at(id_value)->imm << " ";
                }
                outputFile << endl;
                break;
            default:
                break;
        }
    }
    /**
     * 输出主函数main的代码ICode
     */
    outputFile<<".text 0x00400000 \n";
     outputFile<<"#主函数main的代码ICode\n";
     outputFile<<"main:\n";
    for (auto ICode: mainICodes) {
        IntermediateCodeType type = ICode->type;
        IEntry *src1 = ICode->src1;
        IEntry *src2 = ICode->src2;
        IEntry *dst= ICode->dst;
        IEntry **dst_ptr = &dst;
        int cnt_param;//for printf_exp
        int cnt = 0;//for def sw address
        vector<int> *rParam_ids ;
        vector<int> *fParam_ids ;

        switch (type) {
            case Printf:
                cnt_param = 0;
                outputFile << "#printf语句:" << src2->values_Id->size()<<"个exp参数\n";
                for (auto id: *src1->strings_iEntry_id) {
                    /**
 la $a0, str_1
li $v0, 4
syscall
lw $a0, 268501012($zero)
li $v0, 1
syscall
                     */
                    if (IEntries.at(id)->str == "%d"){//lw  li  1 syscall
                        IEntry * p = IEntries.at( src2->values_Id->at(cnt_param++));
//                        IEntry * p_val = IEntries.at(p->values_Id->at(0));//VALUE!!!
IEntry * p_val = p;
                        if (p_val->canGetValue){
                            outputFile << "li $a0, "<< p_val->imm<< endl;
                        }else{
                            outputFile << "lw $a0, "<<p_val->startAddress<<"($zero)"<<endl;
                        }
                        outputFile <<"li $v0, 1"<<endl;
                        outputFile <<"syscall"<<endl;
                    }else{
                        //la  li 4 syscall
                        outputFile << "la $a0, "<< "str_"<<id<< endl;
                        outputFile <<"li $v0, 4"<<endl;
                        outputFile <<"syscall"<<endl;
                    }

                }

                break;
            case Add:{
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm + src2->imm;
                }else {
                    if (src1->canGetValue){
                        outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        outputFile << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        outputFile << "li " << "$t0" << ", " << src2->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        outputFile << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        outputFile << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        outputFile << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
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
                        outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        outputFile << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        outputFile << "li " << "$t0" << ", " << src2->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        outputFile << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        outputFile << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        outputFile << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                    }
                }
            }
                break;
            case Mult:{
                outputFile <<"#执行乘法\n";
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm * src2->imm;
                }else {
                    if (src1->canGetValue){
                        outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        outputFile << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        outputFile << "li " << "$t0" << ", " << src2->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        outputFile << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        outputFile << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        outputFile << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
                    }
                }
            }
                break;
            //除法：HI存放余数，LO存放除法结果
            case Div:{
                outputFile << "#执行div：\n";
                if (src1->canGetValue && src2->canGetValue){
                    dst->canGetValue = true;
                    dst->imm = src1->imm / src2->imm;
                }else {
                    if (src1->canGetValue){
                        outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        outputFile << "div " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "mflo " << "$t2" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        outputFile << "li " << "$t1" << ", " << src2->imm << endl;
                        outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        outputFile << "div " <<  "$t0" << ", " << "$t1" << endl;
                        outputFile << "mflo " << "$t2" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        outputFile << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        outputFile << "div "<< "$t0" << ", " << "$t1" << endl;
                        outputFile << "mflo " << "$t2" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
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
                        outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        outputFile << "div " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "mfhi " << "$t2" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else if (src2->canGetValue){
                        outputFile << "li " << "$t1" << ", " << src2->imm << endl;
                        outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        outputFile << "div "  << "$t0" << ", " << "$t1" << endl;
                        outputFile << "mfhi " << "$t2" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress  << "($zero)"<< endl;
                    }else{
                        outputFile << "lw " << "$t0" << ", " << src1->startAddress<< "($zero)" << endl;
                        outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)"<< endl;
                        outputFile << "div " << "$t0" << ", " << "$t1" << endl;
                        outputFile << "mfhi " << "$t2" << endl;
                        outputFile << "sw " << "$t2" << ", " << dst->startAddress<< "($zero)" << endl;
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
                outputFile<< "#getint:\n";
                outputFile << "\nli $v0, 5\n";
                outputFile << "syscall\n";
                if (dst->type == 2){
                    outputFile << "lw " << "$t0" << ", "<< dst->startAddress<<"($zero)" << endl;
                    outputFile << "sw " << "$v0" << ", "<< "0($t0)" << endl;
                }else{
                    outputFile << "sw " << "$v0" << ", " << dst->startAddress << "($zero)" << endl;//基本不会再被用到了？
                }
                dst->canGetValue = false;
                break;
                //TODO：检查格式统一 全都是IEntry格式   可以进行一个canGetElement的优化
            case GetArrayElement:{//FIXME:数组元素的get需要找到元素地址！！！  即本身  而不是值的副本   又或者说成是让定义的数组记住它！！
                outputFile << "#GetArray Element\n";
                int index = 0;
                int isNormalArray = src2->type;
                if (isNormalArray == 0){//表示array并不是通过函数传递地址而来  即offsetEntry没用 或者认为就是0 即index就是最终索引  s7存放数组元素地址
                    if (src1->canGetValue){
                        index += src1->imm;
                        if (src2->isGlobal){
                            outputFile << "la " << "$t0" << ", " << src2->original_Name << endl;
                            outputFile << "li " << "$t1" << ", " << index*4 << endl;
                            outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl; //value's address in $t2
                        }else{
                            outputFile << "li " << "$t0" << ", " << src2->startAddress + index*4 << endl;
                        }
                        //dst——type  2    0
                        if (dst->type ==0){
                            outputFile << "lw " << "$t0" << ", 0($t0)" << endl;
                            outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                        }else{
                            //t0存地址
                            outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                            dst->type = 0;
                        }
                    }else{
                        if (src2->isGlobal){
                            outputFile << "la " << "$t0" << ", " << src2->original_Name << endl;
                        }else{
                            outputFile << "li " << "$t0" << ", " << src2->startAddress + index*4 << endl;
                        }
                        outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        outputFile << "sll " << "$t1" << ", " << "$t1"<< ", 2" << endl;
                        outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl; //value's address in $t2
                        if (dst->type ==0){
                            outputFile << "lw " << "$t0" << ", 0($t0)" << endl;
                            outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;//此时dst_ptr的IEntry false  需要lw address 来使用
                        }else{
                            //t0地址
                            outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                        }
                    }
                }else{//不是normal  出现在自定义函数内部的引用数组  此时src2 会是startAddress offset_Entry
                    IEntry* offset = src2->offset_IEntry;
                    if (offset->canGetValue){ //引用数组的索引 已知
                        index += offset->imm;
                        if (src1->canGetValue){
                            index += src1->imm;
                            if (src2->isGlobal){
                                outputFile << "la " << "$t0" << ", " << src2->original_Name<<endl;
                                outputFile << "li " << "$t1" << ", " << index*4 << endl;
                                outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl;
                                if (dst->type ==0){
                                    outputFile << "lw " << "$t0" << ", 0($t0)" << endl;
                                    outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;//此时dst_ptr的IEntry false  需要lw address 来使用
                                }else{
                                    //t0地址
                                    outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                }
                            }else{
                                if (dst->type ==0){
                                    outputFile << "lw " << "$t0" << ", " << src2->startAddress + index *4<< "($zero)"<<endl;
                                    outputFile << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                                }else{
                                    //t0地址
                                    outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                }
                            }
                        }else{ //额外索引值是getint 函数调用的引用时
                            if (src2->isGlobal){
                                outputFile << "la " << "$t0" << ", " << src2->original_Name << endl;
                                outputFile << "li " << "$t1" << ", " << index * 4 << endl;//此时index只有offset->imm  再加上src1的值
                                outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl;
                                outputFile << "lw " << "$t2" << ", " << src1->startAddress << "($zero)" << endl;
                                outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t2"<< endl; //value's address in $t3
                            }else{
                                outputFile << "li " << "$t0" << ", " << src2->startAddress + index*4 << endl;
                                outputFile << "lw " << "$t2" << ", " << src1->startAddress << "($zero)" << endl;
                                outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t2"<< endl; //value's address in $t3
                            }
                            if (dst->type ==0){
                                outputFile << "lw " << "$t0" << ", " << src2->startAddress + index *4<< "($zero)"<<endl;
                                outputFile << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                            }else{
                                //t0地址
                                outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                            }
                        }
                    }else{ //有时引用数组的索引都是getint  arr[t] ||||||   sll rd rt sham: rt » sham => rd, shift left logical 向左移位
                        outputFile << "lw " << "$t0" << ", " << offset->startAddress<<"($zero)" << endl;//t in $t0
                        outputFile << "sll " << "$t0"<< ", "<< "$t0"<< " 2\n";//$t0
                        if (src1->canGetValue){
                            outputFile << "li " << "$t1" << ", " << src1->imm*4  << endl;// in $t1
                        }else{
                            outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;// in $t1
                            outputFile << "sll " << "$t1"<< ", "<< "$t1"<< " 2\n";//$t0
                        }
                        outputFile << "addu " << "$t2" << ", "  << "$t0" << ", " << "$t1"<< endl; // in $t2
                        if (src2->isGlobal){
                            outputFile << "la " << "$t3" << ", " << src2->original_Name << endl;
                        }else{
                            outputFile << "li " << "$t3" << ", " << src2->startAddress<< endl;
                        }
                        outputFile << "addu " << "$t0" << ", "  << "$t3" << ", " << "$t2"<< endl; //value's address in $t3
                        if (dst->type ==0){
                            outputFile << "lw " << "$t0" << ", " <<  "0($t0)"<<endl;
                            outputFile << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                        }else{
                            //t0地址
                            outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                        }
                    }
                }
                break;
            }
            //main函数中的return
            case Return:{
                outputFile <<"li $v0, 10"<< endl;
                outputFile << "syscall " << endl;
                break;
            }
                /**
                     * 将实参的  值 地址 按格式给形参   IN src1->valuesId  存储了对应的形参时生成的IEntry
                     */
            //TODO:函数的格式理解  sp  压栈~虚拟？  IEntry:has_return?
            case FuncCall:
                rParam_ids = src2->values_Id;//point to value
                fParam_ids = src1->values_Id;//point to def
                if (rParam_ids->size() != fParam_ids->size()) {
                    outputFile << "error!!!!  rParam_ids->size() = "<<rParam_ids->size()<<"fParam_ids->size() = "<<fParam_ids->size()<<"\n";
                }
                outputFile << "#调用函数" << src1->original_Name << ":\n ";
                for (int i = 0; i < rParam_ids->size();i++){
                    assign(IEntries.at(rParam_ids->at(i)), nullptr,IEntries.at(IEntries.at(fParam_ids->at(i))->values_Id->at(0)));
                }
                outputFile << endl;
                /**
                 * # Pushing Function Real Params:
addiu $sp, $sp, -30000
sw $ra, 0($sp)
# Call function!
jal Label_1
lw $ra, 0($sp)
# Pop params
addiu $sp, $sp, 30000
                 */
                //ra 在sp中压栈
                outputFile << "addiu $sp, $sp, -4\n";
                outputFile <<"sw $ra, 0($sp)\n";
                //call function
                outputFile << "jal " << "_" << src1->original_Name << endl;
                //ra 出栈
                outputFile <<"lw $ra, 0($sp)\n";
                outputFile << "addiu $sp, $sp, 4\n";

                //函数返回值在v0中  要sw
//                outputFile << "sw " << "$v0" << ", " << src1->return_IEntry->startAddress << "($zero)" << endl;//src2 = IEntries.at(func->id)
                if (dst != nullptr){
                    outputFile << "sw " << "$v0" << ", " << dst->startAddress << "($zero)"<< endl;//src2 = IEntries.at(func->id)
                }
                break;

                /**
                 * 函数名标签  就是函数头的名字  形参的IEntry需要在中间代码就生成
                 *  for (auto entry :entries) {
                func->values_Id->push_back(entry->id);//传递的最终都是IEntry  值或地址！
            }
                 */
            case FuncDef:
                outputFile <<"_"<<src1->original_Name <<":\n";//函数名标签
                outputFile << "#" << src1->original_Name << "部分: ";
                for (auto id: *src1->values_Id) {
                    if(IEntries.at(id)->type == 0){
                        outputFile <<"value:@" << id <<" ";
                    }else{
                        outputFile <<"address:@" << id <<" ";
                    }
                }
                outputFile << endl;
                break;
                /**
                 * 非全局变量的初始化定义
                 */
            case VAR_Def_Has_Value:
                outputFile<< "#local_var_@"+ to_string(ICode->src1->Id) <<"_"+src1->original_Name<<"_def:  " ;
                if(IEntries.at(src1->values_Id->at(0))->canGetValue){
                    //                    src1->imm = IEntries.at(src1->values_Id->at(0))->imm;
                    //                    src1->canGetValue = true;
                    outputFile<<IEntries.at(src1->values_Id->at(0))->imm << " ";
                }else{
                    //                    outputFile << "";
                    //                    outputFile << "lw " << "$t0" << ", " << (IEntries.at(src1->values_Id->at(0))->startAddress) << "($zero)" << endl;
                    //                    outputFile << "sw " << "$t0" << ", " << (src1->startAddress) << "($zero)" << endl;
                    outputFile<<"@("<<src1->values_Id->at(0)<<")"<< " ";
                }
                outputFile<<endl;
                if(IEntries.at(src1->values_Id->at(0))->canGetValue){
                    outputFile << "li " << "$t0" << ",  "<<IEntries.at(src1->values_Id->at(0))->imm << endl;
                }else{
                    outputFile << "lw " << "$t0" << ",  "<<IEntries.at(src1->values_Id->at(0))->startAddress<< "($zero)" << endl;
                }
                outputFile << "sw " << "$t0, " <<  IEntries.at(src1->values_Id->at(0))->startAddress << "($zero)" << endl;
                break;
            case VAR_Def_No_Value:
                outputFile<< "#local_var_@"+ to_string(ICode->src1->Id) <<"_"+src1->original_Name<<"no_value_def\n  " ;
                break;
            case ARRAY_VAR_Def_Has_Value:
                outputFile<< "#local_array_@"+ to_string(ICode->src1->Id) <<"_"+src1->original_Name<<"_def:  " ;
                for (auto id_init_value:*(src1->values_Id)) {
                    if (IEntries.at(id_init_value)->canGetValue){
                        ;//FIXME:编译时可以得出的值  以后要用就再按需取
                        outputFile<<IEntries.at(id_init_value)->imm<<" ";
                    }else{
                        outputFile<<"@("<<id_init_value<<")" << " ";
                        ;//FIXME:编译时不可以得出的值  说明运行后通过getint的变量间接得到值  以后要用就再按需lw sw取  初始值IEntry是有若干个IEntry组成的 可以进行canGetValue的判断
                    }
                }
                outputFile<<endl;
                cnt = 0;
                for (auto id_init_value:*(src1->values_Id)) {
                    if (IEntries.at(id_init_value)->canGetValue){ //认为数组内存是连续存储？
                        outputFile << "li " << "$t0" << ",  "<<IEntries.at(id_init_value)->imm << endl;
                    }else{
                        outputFile << "lw " << "$t0" << ",  "<<IEntries.at(id_init_value)->startAddress<< "($zero)" << endl;
                    }
                    outputFile << "sw " << "$t0, " <<  src1->startAddress+cnt*4 << "($zero)" << endl;
                    cnt++;
                }
                break;
            case ARRAY_Def_No_Value:
                outputFile<< "#local_array_@"+ to_string(ICode->src1->Id) <<"_"+src1->original_Name<<"_def\n  " ;
                break;
            case Const_Def_Has_Value:
                outputFile<< "#const_@"+ to_string(ICode->src1->Id) <<"_"+src1->original_Name<<"_def:  " ;
                for (auto init_id:*(src1->values_Id)) {
                    outputFile<<IEntries.at(init_id)->imm<<" ";
                }
                cnt = 0;
                outputFile<<endl;
                for (auto id_init_value:*(src1->values_Id)) {
                    if (IEntries.at(id_init_value)->canGetValue){ //认为数组内存是连续存储？
                        outputFile << "li " << "$t0" << ",  "<<IEntries.at(id_init_value)->imm << endl;
                    }else{
                        outputFile << "lw " << "$t0" << ",  "<<IEntries.at(id_init_value)->startAddress<< "($zero)" << endl;
                    }
                    outputFile << "sw " << "$t0, " <<  src1->startAddress+cnt*4 << "($zero)" << endl;
                    cnt++;
                }
                break;
            case ARRAY_CONST_Def_Has_Value:
                outputFile<< "#array_const@"+ to_string(ICode->src1->Id)<<"_"+src1->original_Name <<"def   " ;
                for (auto init_id:*(src1->values_Id)) {
                    outputFile<<IEntries.at(init_id)->imm<<" ";
                }
                outputFile<<endl;
                break;
            default:
                break;

        }
    }


    /**
     * 输出其他函数的代码ICode
     */
    outputFile<<"#自定义函数的代码ICode\n";
    for (const auto& func_codes:otherFuncICodes) {

//        outputFile << "_" << func_codes.first << ":\n";  第一行总是函数的定义头
        for (auto ICode: func_codes.second) {
            IntermediateCodeType type = ICode->type;
            IEntry *src1 = ICode->src1;
            IEntry *src2 = ICode->src2;
            IEntry *dst = ICode->dst;
            IEntry **dst_ptr = &dst;
            int cnt_param;//for printf_exp
            int cnt = 0;//for def sw address
            vector<int> *rParam_ids;
            vector<int> *fParam_ids;

            switch (type) {
                case Printf:
                    outputFile << "#printf语句:" << src2->values_Id->size()<<"个exp参数\n";
                    cnt_param = 0;
                    for (auto id: *src1->strings_iEntry_id) {
                        /**
     la $a0, str_1
    li $v0, 4
    syscall
    lw $a0, 268501012($zero)
    li $v0, 1
    syscall
                         */
                        if (IEntries.at(id)->str == "%d") {//lw  li  1 syscall
                            IEntry *p = IEntries.at(src2->values_Id->at(cnt_param++));
//                        IEntry * p_val = IEntries.at(p->values_Id->at(0));//VALUE!!!
                            IEntry *p_val = p;
                            if (p_val->canGetValue) {
                                outputFile << "li $a0, " << p_val->imm << endl;
                            } else {
                                outputFile << "lw $a0, " << p_val->startAddress << "($zero)" << endl;
                            }
                            outputFile << "li $v0, 1" << endl;
                            outputFile << "syscall" << endl;
                        } else {
                            //la  li 4 syscall
                            outputFile << "la $a0, " << "str_" << id << endl;
                            outputFile << "li $v0, 4" << endl;
                            outputFile << "syscall" << endl;
                        }

                    }

                    break;
                case Add: {
                    if (src1->canGetValue && src2->canGetValue) {
                        dst->canGetValue = true;
                        dst->imm = src1->imm + src2->imm;
                    } else {
                        if (src1->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else if (src2->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src2->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        }
                    }
                }
                    break;
                case Sub: {
                    if (src1->canGetValue && src2->canGetValue) {
                        dst->canGetValue = true;
                        dst->imm = src1->imm - src2->imm;
                    } else {
                        if (src1->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else if (src2->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src2->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "sub " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        }
                    }
                }
                    break;
                case Mult: {
                    if (src1->canGetValue && src2->canGetValue) {
                        dst->canGetValue = true;
                        dst->imm = src1->imm * src2->imm;
                    } else {
                        if (src1->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else if (src2->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src2->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        }
                    }
                }
                    break;
                    //除法：HI存放余数，LO存放除法结果
                case Div: {
                    outputFile << "#执行div：\n";
                    if (src1->canGetValue && src2->canGetValue) {
                        dst->canGetValue = true;
                        dst->imm = src1->imm / src2->imm;
                    } else {
                        if (src1->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "div " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "mflo " << "$t2" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else if (src2->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src2->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "div " << "$t1" << ", " << "$t0" << endl;
                            outputFile << "mflo " << "$t2" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "div "  << "$t0" << ", " << "$t1" << endl;
                            outputFile << "mflo " << "$t2" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        }
                    }
                }
                    break;
                    //TODO:优化
                case Mod: {
                    if (src1->canGetValue && src2->canGetValue) {
                        dst->canGetValue = true;
                        dst->imm = src1->imm % src2->imm;
                    } else {
                        if (src1->canGetValue) {
                            outputFile << "li " << "$t0" << ", " << src1->imm << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "div " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "mfhi " << "$t2" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else if (src2->canGetValue) {
                            outputFile << "li " << "$t1" << ", " << src2->imm << endl;
                            outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "div "  << "$t0" << ", " << "$t1" << endl;
                            outputFile << "mfhi " << "$t2" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                            outputFile << "div " << "$t0" << ", " << "$t1" << endl;
                            outputFile << "mfhi " << "$t2" << endl;
                            outputFile << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                        }
                    }
                }
                    break;
                    //FIXME:一定是地址？
                case Assign:
                    assign(src1, nullptr, dst);
                    break;
                    //FIXME:总是容易陷入误区 得到v0的值已经是运行时  编译的极限块也做不到预知~
                case GetInt:
                    outputFile<< "#getint:\n";
                    outputFile << "li $v0, 5\n";
                    outputFile << "syscall\n";
                    if (dst->type == 2){
                        outputFile << "lw " << "$t0" << ", "<< dst->startAddress<<"($zero)" << endl;
                        outputFile << "sw " << "$v0" << ", "<< "0($t0)" << endl;
                    }else{
                        outputFile << "sw " << "$v0" << ", " << dst->startAddress << "($zero)" << endl;//基本不会再被用到了？
                    }
                    dst->canGetValue = false;
                    break;
                    //TODO：检查格式统一 全都是IEntry格式   可以进行一个canGetElement的优化
                case GetArrayElement:{//FIXME:数组元素的get需要找到元素地址！！！  即本身  而不是值的副本   又或者说成是让定义的数组记住它！！
                    outputFile << "#GetArray Element\n";
                    int index = 0;
                    int isNormalArray = src2->type;
                    if (isNormalArray == 0){//表示array并不是通过函数传递地址而来  即offsetEntry没用 或者认为就是0 即index就是最终索引  s7存放数组元素地址
                        if (src1->canGetValue){
                            index += src1->imm;
                            if (src2->isGlobal){
                                outputFile << "la " << "$t0" << ", " << src2->original_Name << endl;
                                outputFile << "li " << "$t1" << ", " << index*4 << endl;
                                outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl; //value's address in $t2
                            }else{
                                outputFile << "li " << "$t0" << ", " << src2->startAddress + index*4 << endl;
                            }
                            //dst——type  1    0
                            if (dst->type ==0){
                                outputFile << "lw " << "$t0" << ", 0($t0)" << endl;
                                outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                            }else{
                                //采用地址传递  内容是地址
                                outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                dst->type = 0;
                            }
                        }else{
                            if (src2->isGlobal){
                                outputFile << "la " << "$t0" << ", " << src2->original_Name << endl;
                            }else{
                                outputFile << "li " << "$t0" << ", " << src2->startAddress + index*4 << endl;
                            }
                            outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                            outputFile << "sll " << "$t1" << ", " << "$t1"<< ", 2" << endl;
                            outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl; //value's address in $t2
                            if (dst->type ==0){
                                outputFile << "lw " << "$t0" << ", 0($t0)" << endl;
                                outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;//此时dst_ptr的IEntry false  需要lw address 来使用
                            }else{
                                //t0地址
                                outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                dst->type = 0;
                            }
                        }
                    }else{//不是normal  出现在自定义函数内部的引用数组  此时src2 会是startAddress offset_Entry
                        IEntry* offset = src2->offset_IEntry;
                        if (offset->canGetValue){ //引用数组的索引 已知
                            index += offset->imm;
                            if (src1->canGetValue){
                                index += src1->imm;
                                if (src2->isGlobal){
                                    outputFile << "la " << "$t0" << ", " << src2->original_Name<<endl;
                                    outputFile << "li " << "$t1" << ", " << index*4 << endl;
                                    outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl;
                                    if (dst->type ==0){
                                        outputFile << "lw " << "$t0" << ", 0($t0)" << endl;
                                        outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;//此时dst_ptr的IEntry false  需要lw address 来使用
                                    }else{
                                        //t0地址
                                        outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                    }
                                }else{
                                    if (dst->type ==0){
                                        outputFile << "lw " << "$t0" << ", " << src2->startAddress + index *4<< "($zero)"<<endl;
                                        outputFile << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                                    }else{
                                        //t0地址
                                        outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                        dst->type = 0;
                                    }
                                }
                            }else{ //额外索引值是getint 函数调用的引用时
                                if (src2->isGlobal){
                                    outputFile << "la " << "$t0" << ", " << src2->original_Name << endl;
                                    outputFile << "li " << "$t1" << ", " << index * 4 << endl;//此时index只有offset->imm  再加上src1的值
                                    outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t1"<< endl;
                                    outputFile << "lw " << "$t2" << ", " << src1->startAddress << "($zero)" << endl;
                                    outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t2"<< endl; //value's address in $t3
                                }else{
                                    outputFile << "li " << "$t0" << ", " << src2->startAddress + index*4 << endl;
                                    outputFile << "lw " << "$t2" << ", " << src1->startAddress << "($zero)" << endl;
                                    outputFile << "addu " << "$t0" << ", "  << "$t0" << ", " << "$t2"<< endl; //value's address in $t3
                                }
                                if (dst->type ==0){
                                    outputFile << "lw " << "$t0" << ", " << src2->startAddress + index *4<< "($zero)"<<endl;
                                    outputFile << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                                }else{
                                    //t0地址
                                    outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                    dst->type = 0;
                                }
                            }
                        }else{ //有时引用数组的索引都是getint  arr[t] ||||||   sll rd rt sham: rt » sham => rd, shift left logical 向左移位
                            outputFile << "lw " << "$t0" << ", " << offset->startAddress<<"($zero)" << endl;//t in $t0
                            outputFile << "sll " << "$t0"<< ", "<< "$t0"<< " 2\n";//$t0
                            if (src1->canGetValue){
                                outputFile << "li " << "$t1" << ", " << src1->imm*4  << endl;// in $t1
                            }else{
                                outputFile << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;// in $t1
                                outputFile << "sll " << "$t1"<< ", "<< "$t1"<< " 2\n";//$t0
                            }
                            outputFile << "addu " << "$t2" << ", "  << "$t0" << ", " << "$t1"<< endl; // in $t2
                            if (src2->isGlobal){
                                outputFile << "la " << "$t3" << ", " << src2->original_Name << endl;
                            }else{
                                outputFile << "li " << "$t3" << ", " << src2->startAddress<< endl;
                            }
                            outputFile << "addu " << "$t0" << ", "  << "$t3" << ", " << "$t2"<< endl; //value's address in $t3
                            if (dst->type ==0){
                                outputFile << "lw " << "$t0" << ", " <<  "0($t0)"<<endl;
                                outputFile << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                            }else{
                                //t0地址
                                outputFile << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                                dst->type = 0;
                            }
                        }
                    }
                    break;
                }
                case Return: {
                    if (src1 != nullptr) {
                        if (src1->canGetValue) {
                            outputFile << "li " << "$v0" << ", " << src1->imm << endl;
                        } else {
                            outputFile << "lw " << "$v0" << ", " << src1->startAddress << "($zero)" << endl;
                        }
                    }
                    outputFile << "jr " << "$ra" << endl;
                    break;
                }
                    /**
                         * 将实参的  值 地址 按格式给形参   IN src1->valuesId  存储了对应的形参时生成的IEntry
                         */
                    //TODO:函数的格式理解  sp  压栈~虚拟？  IEntry:has_return?
                case FuncCall:
                    rParam_ids = src2->values_Id;
                    fParam_ids = src1->values_Id;
                    if (rParam_ids->size() != fParam_ids->size()) {
                        outputFile << "error!!!!  rParam_ids->size() = "<<rParam_ids->size()<<"fParam_ids->size() = "<<fParam_ids->size()<<"\n";
                    }
                    outputFile << "#调用函数" << src1->original_Name << ": \n";
                    for (int i = 0; i < rParam_ids->size(); i++) {
                        assign(IEntries.at(rParam_ids->at(i)), nullptr,
                               IEntries.at(IEntries.at(fParam_ids->at(i))->values_Id->at(0)));
                    }
                    outputFile << endl;
                    /**
                     * # Pushing Function Real Params:
    addiu $sp, $sp, -30000
    sw $ra, 0($sp)
    # Call function!
    jal Label_1
    lw $ra, 0($sp)
    # Pop params
    addiu $sp, $sp, 30000
                     */
                    //ra 在sp中压栈
                    outputFile << "addiu $sp, $sp, -4\n";
                    outputFile << "sw $ra, 0($sp)\n";
                    //call function
                    outputFile << "jal " << "_" << src1->original_Name << endl;
                    //ra 出栈

                    outputFile << "lw $ra, 0($sp)\n";
                    outputFile << "addiu $sp, $sp, 4\n";
                    //函数返回值在v0中  要sw   其实这里的sw v0 to somewhere 没有用
//                    outputFile << "sw " << "$v0" << ", " << src1->return_IEntry->startAddress << "($zero)"<< endl;//src2 = IEntries.at(func->id)
                    if (dst != nullptr){
                        outputFile << "sw " << "$v0" << ", " << dst->startAddress << "($zero)"<< endl;//src2 = IEntries.at(func->id)
                    }
                    break;

                    /**
                     * 函数名标签  就是函数头的名字  形参的IEntry需要在中间代码就生成
                     *  for (auto entry :entries) {
                    func->values_Id->push_back(entry->id);//传递的最终都是IEntry  值或地址！
                }
                     */
                case FuncDef:
                    outputFile << "_" << src1->original_Name << ":\n";//函数名标签
                    outputFile << "#" << src1->original_Name << "部分: ";
                    for (auto id: *src1->values_Id) {
                        if (IEntries.at(id)->type == 0) {
                            outputFile << "value:@" << id << " ";
                        } else {
                            outputFile << "address:@" << id << " ";
                        }
                    }
                    outputFile << endl;
                    break;
                    /**
                     * 非全局变量的初始化定义
                     */
                case VAR_Def_Has_Value:
                    outputFile << "#local_var_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  ";
                    if (IEntries.at(src1->values_Id->at(0))->canGetValue) {
                        //                    src1->imm = IEntries.at(src1->values_Id->at(0))->imm;
                        //                    src1->canGetValue = true;
                        outputFile << IEntries.at(src1->values_Id->at(0))->imm << " ";
                    } else {
                        //                    outputFile << "";
                        //                    outputFile << "lw " << "$t0" << ", " << (IEntries.at(src1->values_Id->at(0))->startAddress) << "($zero)" << endl;
                        //                    outputFile << "sw " << "$t0" << ", " << (src1->startAddress) << "($zero)" << endl;
                        outputFile << "@(" << src1->values_Id->at(0) << ")" << " ";
                    }
                    outputFile << endl;
                    if (IEntries.at(src1->values_Id->at(0))->canGetValue) {
                        outputFile << "li " << "$t0" << ",  " << IEntries.at(src1->values_Id->at(0))->imm << endl;
                    } else {
                        outputFile << "lw " << "$t0" << ",  " << IEntries.at(src1->values_Id->at(0))->startAddress
                             << "($zero)" << endl;
                    }
                    outputFile << "sw " << "$t0, " << IEntries.at(src1->values_Id->at(0))->startAddress << "($zero)" << endl;
                    break;
                case VAR_Def_No_Value:
                    outputFile << "#local_var_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name
                         << "no_value_def\n  ";
                    break;
                case ARRAY_VAR_Def_Has_Value:
                    outputFile << "#local_array_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  ";
                    for (auto id_init_value: *(src1->values_Id)) {
                        if (IEntries.at(id_init_value)->canGetValue) { ;//FIXME:编译时可以得出的值  以后要用就再按需取
                            outputFile << IEntries.at(id_init_value)->imm << " ";
                        } else {
                            outputFile << "@(" << id_init_value << ")"
                                 << " ";;//FIXME:编译时不可以得出的值  说明运行后通过getint的变量间接得到值  以后要用就再按需lw sw取  初始值IEntry是有若干个IEntry组成的 可以进行canGetValue的判断
                        }
                    }
                    outputFile << endl;
                    cnt = 0;
                    for (auto id_init_value: *(src1->values_Id)) {
                        if (IEntries.at(id_init_value)->canGetValue) { //认为数组内存是连续存储？
                            outputFile << "li " << "$t0" << ",  " << IEntries.at(id_init_value)->imm << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ",  " << IEntries.at(id_init_value)->startAddress << "($zero)"
                                 << endl;
                        }
                        outputFile << "sw " << "$t0, " <<  src1->startAddress+cnt*4 << "($zero)" << endl;
                        cnt++;
                    }
                    break;
                case ARRAY_Def_No_Value:
                    outputFile << "#local_array_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def\n  ";
                    break;
                case Const_Def_Has_Value:
                    outputFile << "#const_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  ";
                    for (auto init_value: *(src1->values)) {
                        outputFile << init_value << " ";
                    }
                    outputFile << endl;
                    cnt = 0;
                    for (auto id_init_value: *(src1->values_Id)) {
                        if (IEntries.at(id_init_value)->canGetValue) { //认为数组内存是连续存储？
                            outputFile << "li " << "$t0" << ",  " << IEntries.at(id_init_value)->imm << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ",  " << IEntries.at(id_init_value)->startAddress << "($zero)"
                                 << endl;
                        }
                        outputFile << "sw " << "$t0, " <<  src1->startAddress+cnt*4 << "($zero)" << endl;
                        cnt++;
                    }
                    break;
                case ARRAY_CONST_Def_Has_Value:
                    outputFile << "#array_const@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "def   ";
                    for (auto init_id:*(src1->values_Id)) {
                        outputFile<<IEntries.at(init_id)->imm<<" ";
                    }
                    outputFile << endl;
                    cnt = 0;
                    for (auto id_init_value: *(src1->values_Id)) {
                        if (IEntries.at(id_init_value)->canGetValue) { //认为数组内存是连续存储？
                            outputFile << "li " << "$t0" << ",  " << IEntries.at(id_init_value)->imm << endl;
                        } else {
                            outputFile << "lw " << "$t0" << ",  " << IEntries.at(id_init_value)->startAddress << "($zero)"
                                 << endl;
                        }
                        outputFile << "sw " << "$t0, " <<  src1->startAddress+cnt*4 << "($zero)" << endl;
                        cnt++;
                    }
                    break;
                default:
                    break;

            }
        }
    }
// 重定向结束后，可以将outputFile的流恢复到原始状态
//    std::outputFile.rdbuf(outputFileBuffer);

}


