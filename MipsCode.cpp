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
bool assignSp = false;





void MipsCode::assign(IEntry *src1,IEntry *src2,IEntry *dst) { //传进来需要已经values_Id处理好
    /*
     * 值传递  地址赋
     * TODO:需要对全局变量的分类讨论   局部变量也是    写入值编译时不确定时
     * TODO：多次写入的更新原则！！！
     * */
    output << "#assign" << endl;
    //TODO:  参数废弃
    if (src1->type == 6){
        output << "lw $t1," <<src1->imm*4<<"($sp)"<<endl;
        if (dst->type == 6){
            output << "sw " << "$t1, " << dst->imm*4 << "($sp)" << endl;
        }else{
            if (assignSp){
                output << "sw " << "$t1, " << dst->startAddress << "($sp)" << endl;
            }else{
                output << "sw " << "$t1, " << dst->startAddress << "($zero)" << endl;
            }
        }
    }else if (src1->type == 0){
        if (src1->isGlobal){
            output << "la $t1," << src1->original_Name << endl;
        }else if (src1->canGetValue){
            output << "li " << "$t1, " << src1->imm << endl;
        }else{
            if (assignSp){
                output << "lw " << "$t1, " << src1->startAddress << "($sp)" << endl;
            }else{
                output << "lw " << "$t1, " << src1->startAddress << "($zero)" << endl;
            }
        }
        if (dst->type == 6) {//dst放在sp中  可能地址 也可能值  我们不区分  就是参数
            output << "sw " << "$t1, " << dst->imm*4 << "($sp)" << endl;
        }else if (dst->type == 2){//dst存储的是地址
            if (assignSp){
                output << "lw " << "$t2, " << dst->startAddress << "($sp)" << endl;
                //output << "addu $t2,$t2,$sp\n";
                output << "sw " << "$t1, " <<  "0($t2)" << endl;
            }else{
                output << "lw " << "$t2, " << dst->startAddress << "($zero)" << endl;
                output << "sw " << "$t1, " << "0($t2)" << endl; //  需要从变量的IEntry的values_Id中取得的  t0  地址  为写语句而生
            }
        }else{
            if (assignSp){
                output << "sw " << "$t1, " << dst->startAddress << "($sp)" << endl;
            }else{
                output << "sw " << "$t1, " << dst->startAddress << "($zero)" << endl; //dst是值   需要从变量的IEntry的values_Id中取得的
            }


        }
            dst->canGetValue =  false;//后台更新
    }else{
        output << "#地址拷贝\n";
        if (assignSp){
            output<< "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
        }
        else{
            output<< "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
        }
        if (dst->type == 6){
            output<< "sw $t0, "<< dst->imm*4<< "($sp)"<<endl;
        }else{
            if (assignSp){
                output<< "sw $t0, "<< dst->startAddress<< "($sp)"<<endl;
            }else{
                output<< "sw $t0, "<< dst->startAddress<< "($zero)"<<endl;
            }

        }

    }

}

void MipsCode::testRe(){
    // 创建一个ofstream对象

// 保存outputFile的原始缓冲区指针
//    std::streambuf* outputFileBuffer = std::output.rdbuf();

// 将outputFile的流重定向到outputFile
//    std::output.rdbuf(output.rdbuf());
    output << ".text\n";
    // 重定向结束后，可以将outputFile的流恢复到原始状态
//    std::output.rdbuf(outputFileBuffer);
}
void MipsCode::translate()  {
// 创建一个ofstream对象
//    std::ofstream output("mips.txt");

// 保存outputFile的原始缓冲区指针
//    std::streambuf* outputFileBuffer = std::output.rdbuf();

// 将outputFile的流重定向到outputFile
//    std::output.rdbuf(output.rdbuf());


    /**
     * 输出全局的变量定义data段  以及全局变量的初始化
     */
    output << ".data 0x10010000\n";
//    output << ".data 0x1000\n";

    output << "temppppppppppppp:  .space  160000\n\n";  // 临时内存区，起始地址为0x10010000 (16) or 268500992 (10)
    /**字符串区
# string tokens:
str_1:  .asciiz   "hello!"
str_3:  .asciiz   "haha"
str_5:  .asciiz   "ha"

     */
     output << "#strings in printf\n";
    for (auto id: strings) {
        output << "str_" << id << ": .asciiz " << "\"" << IEntries.at(id)->str << "\"" << endl;
    }
    for (auto def:globalDef) {
        //形如 def src1
        IntermediateCodeType type = def->type;
        //TODO:名字需要注意  是否在语法分析时准备好名字
        switch (type) {
            case VAR_Def_Has_Value:
                output << def->src1->original_Name << ":  .word  " ;
                for (auto id_init_value:*(def->src1->values_Id)) {
                    output << IEntries.at(id_init_value )->imm << " ";
                }
                output << endl;
                break;
            case VAR_Def_No_Value:
                output << def->src1->original_Name << ":  .word  " ;
                for (int i = 0;i<def->src1->total_length;i++) { //单个普通全局变量
                    output << "0 ";//此时输出值也会是0 在语法分析部分进行了判断补充
                }
                output << endl;
                break;
            case ARRAY_VAR_Def_Has_Value:
                output << def->src1->original_Name << ":  .word  " ;
                for (auto id_init_value:*(def->src1->values_Id)) {
                    output << IEntries.at(id_init_value)->imm << " ";
                }
                output << endl;
                break;
            case ARRAY_Def_No_Value:
                output << def->src1->original_Name << ":  .word  " ;
                for (int i = 0;i<def->src1->total_length;i++) {
                    output << "0 ";
                }
                output << endl;
                break;
            case Const_Def_Has_Value:
                output << def->src1->original_Name << ":  .word  " ;
                for (auto id_value:*(def->src1->values_Id)) {
                    output << IEntries.at(id_value)->imm << " ";
                }
                output << endl;
                break;
            case ARRAY_CONST_Def_Has_Value:
                output << def->src1->original_Name << ":  .word  " ;
                for (auto id_value:*(def->src1->values_Id)) {
                    output << IEntries.at(id_value)->imm << " ";
                }
                output << endl;
                break;
            default:
                break;
        }
    }
    /**
     * 输出主函数main的代码ICode
     */
    output << ".text 0x00400000 \n";
     output << "#主函数main的代码ICode\n";
     output << "main:\n";
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
            case I_Not:{
                output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                output << "seq $t0, $t0,0"<<endl;//0
                output << "sw $t0, "<<dst->startAddress<<"($zero)"<<endl;
                break;
            }
            case Beq:{
                if (src1->canGetValue){
                    output << "li $t0," << src1->imm <<endl;
                }else{
                    output << "lw $t0," << src1->startAddress<<"($zero)" <<endl;
                }
                output << "lw $t1," << src2->startAddress<<"($zero)" <<endl;
                output << "beq $t0, $t1,"<<dst->name<<endl;
                break;
            }
            case Beqz:{
                output << "lw $t0," << src1->startAddress<<"($zero)" <<endl;
                output << "beqz $t0, "<<src2->name<<endl;
                break;
            }
            case Jump_Label:{
                output<< "j "<<src1->name<<endl;
                break;
            }
            case Insert_Label:{
                output << src1->name<< ":" <<endl;
                break;
            }
            case I_And:{
                output << "##################I_And###########\n";
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm!=0  & src2->imm!=0 ) << endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    if (src1->imm == 0){
                        output << "li $t0,0\n";
                        output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                    }else {
                        output << "lw $t0, " << src2->startAddress << "($zero)" << endl;
                        output << "sne $t0,$t0,0\n";
                        output << "sw $t0, " << dst->startAddress << "($zero)" << endl;
                    }
                }else if (src2->canGetValue){
                    if (src2->imm == 0){
                        output << "li $t0,0\n";
                        output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                    }else {
                        output << "lw $t0, " << src1->startAddress << "($zero)" << endl;
                        output << "sne $t0,$t0,0\n";
                        output << "sw $t0, " << dst->startAddress << "($zero)" << endl;
                    }
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "sne $t0,$t0,0\n";
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sne $t1,$t1,0\n";
                    output << "and $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }
            case I_Or:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm!=0| src2->imm!=0) << endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    if (src1->imm == 1){
                        output << "li $t0,1\n";
                        output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                    }else{
                        output << "lw $t0, " << src2->startAddress << "($zero)" << endl;
                        output << "sne $t0,$t0,0\n";
                        output << "sw $t0, " << dst->startAddress << "($zero)" << endl;
                    }
                }else if (src2->canGetValue){
                    if (src2->imm == 1){
                        output << "li $t0,1\n";
                        output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                    }else{
                        output << "lw $t0, " << src1->startAddress << "($zero)" << endl;
                        output << "sne $t0,$t0,0\n";
                        output << "sw $t0, " << dst->startAddress << "($zero)" << endl;
                    }
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "sne $t0,$t0,0\n";
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sne $t1,$t1,0\n";
                    output << "or $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }

            case I_Eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm == src2->imm)<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                    output << "lw $t0, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "seq $t0 , $t0,"<<src1->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else if (src2->canGetValue){
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "seq $t0 , $t0,"<<src2->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "seq $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }
            case I_Grt_eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  >= src2->imm )<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                    output << "lw $t0, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sle $t0 , $t0,"<<src1->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else if (src2->canGetValue){
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "sge $t0 , $t0,"<<src2->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sge $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }
            case I_Less_eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  <= src2->imm )<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                    output << "lw $t0, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sge $t0 , $t0,"<<src1->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else if (src2->canGetValue){
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "sle $t0 , $t0,"<<src2->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sle $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }

            case I_Grt:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  > src2->imm )<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                    output << "lw $t0, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "li $t1, "<<src1->imm<< endl;
                    output << "slt $t0 , $t0, $t1"<<endl;//故意反一下符号 结果正确   slti不适用
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else if (src2->canGetValue){
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "sgt $t0 , $t0,"<<src2->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sgt $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }
            case I_Less:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  < src2->imm )<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    output << "lw $t0, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sgt $t0 , $t0,"<<src1->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else if (src2->canGetValue){
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "li $t1, "<<src2->imm<< endl;
                    output << "slt $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "slt $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }
            case I_not_eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  != src2->imm )<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else if (src1->canGetValue){
                    output << "lw $t0, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sne $t0 , $t0,"<<src1->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else if (src2->canGetValue){
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "sne $t0 , $t0,"<<src2->imm<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }else{
                    output << "lw $t0, "<<src1->startAddress<<"($zero)"<<endl;
                    output << "lw $t1, "<<src2->startAddress<<"($zero)"<<endl;
                    output << "sne $t0 , $t0, $t1"<<endl;
                    output <<"sw $t0, "<<dst->startAddress <<"($zero)"<<endl;
                }
                break;
            }

            case Printf:
                cnt_param = 0;
                output << "#printf语句:" << src2->values_Id->size() << "个exp参数\n";
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
                            output << "li $a0, " << p_val->imm << endl;
                        }else{
                            output << "lw $a0, " << p_val->startAddress << "($zero)" << endl;
                        }
                        output << "li $v0, 1" << endl;
                        output << "syscall" << endl;
                    }else{
                        //la  li 4 syscall
                        output << "la $a0, " << "str_" << id << endl;
                        output << "li $v0, 4" << endl;
                        output << "syscall" << endl;
                    }

                }

                break;
            case Add:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm + src2->imm<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else {
                    if (src1->canGetValue){
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "addu " << "$t2" << ", " << "$t1" << ", " << src1->imm<< endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else if (src2->canGetValue){
                        output << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "addu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else{
                        output << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }
                }
            }
                break;
            case Sub:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm - src2->imm<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else if (src2->canGetValue){
                        output << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "subu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else{
                        output << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }
                }
            }
                break;
            case Mult:{
                output << "#执行乘法\n";
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm * src2->imm<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else if (src2->canGetValue){
                        output << "li " << "$t0" << ", " << src2->imm << endl;
                        output << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else{
                        output << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }
                }
            }
                break;
            //除法：HI存放余数，LO存放除法结果
            case Div:{
                output << "#执行div：\n";
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm / src2->imm<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mflo " << "$t2" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else if (src2->canGetValue){
                        output << "li " << "$t1" << ", " << src2->imm << endl;
                        output << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mflo " << "$t2" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else{
                        output << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mflo " << "$t2" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }
                }
            }
                break;
            //TODO:优化
            case Mod:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm % src2->imm<< endl;
                    output << "sw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mfhi " << "$t2" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else if (src2->canGetValue){
                        output << "li " << "$t1" << ", " << src2->imm << endl;
                        output << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mfhi " << "$t2" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
                    }else{
                        output << "lw " << "$t0" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "lw " << "$t1" << ", " << src2->startAddress << "($zero)" << endl;
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mfhi " << "$t2" << endl;
                        output << "sw " << "$t2" << ", " << dst->startAddress << "($zero)" << endl;
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
                output << "#getint:\n";
                output << "\nli $v0, 5\n";
                output << "syscall\n";
                if(dst->type == 6){
                    output << "sw $v0,"<<dst->imm*4<<"($sp)"<<endl;
                }else if (dst->type == 2){
                    output << "lw " << "$t0" << ", " << dst->startAddress << "($zero)" << endl;
                    output << "sw " << "$v0" << ", " << "0($t0)" << endl;
                }else{
                    output << "sw " << "$v0" << ", " << dst->startAddress << "($zero)" << endl;//基本不会再被用到了？
                }
                dst->canGetValue = false;
                break;
                //TODO：检查格式统一 全都是IEntry格式   可以进行一个canGetElement的优化
            case GetAddress:{
                output<< "#GetTheAddress  sw in dst 's address\n";
                if (src2->type == 1){
                    output<< "lw $t0,"<<IEntries.at(src2->values_Id->at(0))->startAddress<<"($zero)"<<endl;
                }else if(src2->type == 7){
                    output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                }else{
                    if (src2->isGlobal){
                        output<< "la $t0,"<<src2->original_Name<<endl;
                    }else{
                        output<< "li $t0,"<<src2->startAddress<<endl;
                    }
                }
                if (src1){
                    if (src1->canGetValue){
                        output<< "addu $t0,$t0,"<<src1->imm*4<<endl;
                    }else{
                        output<< "lw, $t1,"<<src1->startAddress<<"($zero)"<<endl;
                        output << "sll $t1,$t1,2"<<endl;
                        output<< "addu $t0,$t0,$t1"<<endl;
                    }
                }
                output<< "sw $t0,"<<dst->startAddress<<"($zero)"<<endl;
                break;
            }
            case GetArrayElement:{//FIXME:数组元素的get需要找到元素地址！！！  即本身  而不是值的副本   又或者说成是让定义的数组记住它！！
                output << "#GetArray Element\n";
                int index = 0;
                int isNormalArray = src2->type;
                if (isNormalArray == 0){//表示array并不是通过函数传递地址而来  即offsetEntry没用 或者认为就是0 即index就是最终索引  s7存放数组元素地址
                    if (src1->canGetValue){
                        index += src1->imm;
                        if (src2->isGlobal){
                            output << "la " << "$t0" << ", " << src2->original_Name << endl;
                            output << "li " << "$t1" << ", " << index * 4 << endl;
                            output << "addu " << "$t0" << ", " << "$t0" << ", " << "$t1" << endl; //value's address in $t2
                        }else{
                            if(src2->type ==7){
                                output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                            }else{
                                output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                            }
                        }
                        //dst——type  1    0
                        if (dst->type ==0){
                            output << "lw " << "$t0" << ", 0($t0)" << endl;
                            output << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
                        }else{
                            //采用地址传递  内容是地址
                            output << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
//                                dst->type = 0;
                        }
                    }else{
                        if (src2->isGlobal){
                            output << "la " << "$t0" << ", " << src2->original_Name << endl;
                        }else{
                            if (src2->type ==7){
                                output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                            }else{
                                output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                            }

                        }
                        output << "lw " << "$t1" << ", " << src1->startAddress << "($zero)" << endl;
                        output << "sll " << "$t1" << ", " << "$t1" << ", 2" << endl;
                        output << "addu " << "$t0" << ", " << "$t0" << ", " << "$t1" << endl; //value's address in $t2
                        if (dst->type ==0){
                            output << "lw " << "$t0" << ", 0($t0)" << endl;
                            output << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;//此时dst_ptr的IEntry false  需要lw address 来使用
                        }else{
                            //t0地址
                            output << "sw " << "$t0, " << dst->startAddress << "($zero)" << endl;
//                                dst->type = 0;
                        }
                    }
                }else{//不是normal  出现在自定义函数内部的引用数组  此时src2 会是startAddress offset_Entry
                    if (src2->type == 7){
                        output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                    }else{
                        output<< "lw $t0,"<<src2->startAddress<<"($zero)"<<endl;
                    }
                    if (src1){
                        if (src1->canGetValue){
                            output<< "addu $t0,$t0,"<<src1->imm*4<<endl;
                        }else{
                            output<< "lw $t1,"<<src1->startAddress<<"($zero)"<<endl;
                            output << "sll $t1,$t1,2"<<endl;
                            output<< "addu $t0,$t0,$t1"<<endl;
                        }
                    }
                    if(dst->type == 0){
                        output<< "lw $t0,0($t0)"<<endl;
                        output<< "sw $t0,"<<dst->startAddress<<"($zero)"<<endl;
                    }else{
                        output<< "sw $t0,"<<dst->startAddress<<"($zero)"<<endl;
                    }
                }
                break;
            }
            //main函数中的return
            case Return:{
                output << "li $v0, 10" << endl;
                output << "syscall " << endl;
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
                    output << "error!!!!  rParam_ids->size() = " << rParam_ids->size() << "fParam_ids->size() = " << fParam_ids->size() << "\n";
                }
                output << "#调用函数" << src1->original_Name << ": \n";
                //ra 在sp中压栈
                output << "addiu $sp, $sp, -30000\n";
                output << "sw $ra, 0($sp)\n";
                IEntry*f,*r;
                for (int i = 0; i < rParam_ids->size(); i++) {
                    r = IEntries.at(rParam_ids->at(i));
                    if(r->type == 0){
                        if (r->canGetValue){
                            output << "li " << "$t1, " << r->imm << endl;
                        }else{
                            output << "lw " << "$t1, " << r->startAddress << "($zero)" << endl;
                        }
                        output << "sw " << "$t1, " <<  IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress<<"($sp)" << endl;
                    }else {// only == 1
                        output << "lw $t0, " << src1->startAddress << "($zero)" << endl;
                        output << "sw " << "$t0, " <<  IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress<< "($sp)" << endl;
                    }
                }
                output << endl;
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
                //call function
                output << "jal " << "Funccccc_" << src1->original_Name << endl;
                //ra 出栈
                output << "#返回函数"<<endl;
                output << "lw $ra, 0($sp)\n";
                output << "addiu $sp, $sp, 30000\n";
                //函数返回值在v0中  要sw   其实这里的sw v0 to somewhere 没有用
//                    output << "sw " << "$v0" << ", " << src1->return_IEntry->startAddress << "($zero)"<< endl;//src2 = IEntries.at(func->id)
                if (dst != nullptr){
                    output << "sw " << "$v0" << ", " << dst->startAddress << "($zero)" << endl;//src2 = IEntries.at(func->id)
                }
                break;

                /**
                 * 函数名标签  就是函数头的名字  形参的IEntry需要在中间代码就生成
                 *  for (auto entry :entries) {
                func->values_Id->push_back(entry->id);//传递的最终都是IEntry  值或地址！
            }
                 */
            case FuncDef:
                output << "Funccccc_" << src1->original_Name << ":\n";//函数名标签
                output << "#" << src1->original_Name << "部分: ";
                for (auto id: *src1->values_Id) {
                    if(IEntries.at(id)->type == 0){
                        output << "value:@" << id << " ";
                    }else{
                        output << "address:@" << id << " ";
                    }
                }
                output << endl;
                break;
                /**
                 * 非全局变量的初始化定义
                 */
            case VAR_Def_Has_Value:
                output << "#local_var_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  " ;
                if(IEntries.at(src1->values_Id->at(0))->canGetValue){
                    output << IEntries.at(src1->values_Id->at(0))->imm << " ";
                }else{
                    output << "@(" << src1->values_Id->at(0) << ")" << " ";
                }
                output << endl;
                break;
            case VAR_Def_No_Value:
                output << "#local_var_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "no_value_def\n  " ;
                break;
            case ARRAY_VAR_Def_Has_Value:
                output << "#local_array_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  " ;
                for (auto id_init_value:*(src1->values_Id)) {
                    if (IEntries.at(id_init_value)->canGetValue){
                        output << IEntries.at(id_init_value)->imm << " ";
                    }else{
                        output << "@(" << id_init_value << ")" << " ";
                    }
                }
                output << endl;
                break;
            case ARRAY_Def_No_Value:
                output << "#local_array_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def\n  " ;
                break;
            case Const_Def_Has_Value:
                output << "#const_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  " ;
                for (auto init_id:*(src1->values_Id)) {
                    output << IEntries.at(init_id)->imm << " ";
                }
                cnt = 0;
                output << endl;
                //const的使用读取 也许不用再使用lw
                for (auto id_init_value:*(src1->values_Id)) {
                    if (IEntries.at(id_init_value)->canGetValue){ //认为数组内存是连续存储？
                        output << "li " << "$t0" << ",  " << IEntries.at(id_init_value)->imm << endl;
                    }else{
                        output << "lw " << "$t0" << ",  " << IEntries.at(id_init_value)->startAddress << "($zero)" << endl;
                    }
                    output << "sw " << "$t0, " << src1->startAddress + cnt * 4 << "($zero)" << endl;
                    cnt++;
                }
                break;
            case ARRAY_CONST_Def_Has_Value:
                output << "#array_const@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "def   " ;
                for (auto init_id:*(src1->values_Id)) {
                    output << IEntries.at(init_id)->imm << " ";
                }
                output << endl;
                break;
            default:
                break;

        }
    }


    /**
     * 输出其他函数的代码ICode
     */
     assignSp  =true;
    output << "#自定义函数的代码ICode\n";
    for (const auto& func_codes:otherFuncICodes) {

//        output << "_" << func_codes.first << ":\n";  第一行总是函数的定义头
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
                case I_Not:{
                    output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                    output << "seq $t0, $t0,0"<<endl;//0
                    output << "sw $t0, "<<dst->startAddress<<"($sp)"<<endl;
                    break;
                }
                case Beqz:{
                    output << "lw $t0," << src1->startAddress<<"($sp)" <<endl;
                    output << "beqz $t0, "<<src2->name<<endl;
                    break;
                }
                case Beq:{
                    if (src1->canGetValue){
                        output << "li $t0," << src1->imm <<endl;
                    }else{
                        output << "lw $t0," << src1->startAddress<<"($sp)" <<endl;
                    }
                    output << "lw $t1," << src2->startAddress<<"($sp)" <<endl;
                    output << "beq $t0, $t1,"<<dst->name<<endl;
                    break;
                }
                case Jump_Label:{
                    output<< "j "<<src1->name<<endl;
                    break;
                }
                case Insert_Label:{
                    output << src1->name<< ":" <<endl;
                    break;
                }
                case I_And:{
                    output << "##################I_And###########\n";
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm!=0  & src2->imm!=0 ) << endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        if (src1->imm ==0){
                            output << "li $t0,0\n";
                            output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                        }else{
                            output << "lw $t0, " << src2->startAddress << "($sp)" << endl;
                            output << "sne $t0,$t0,0\n";
                            output << "sw $t0, " << dst->startAddress << "($sp)" << endl;
                        }
                    }else if (src2->canGetValue){
                        if (src2->imm ==0){
                            output << "li $t0,0\n";
                            output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                        }else{
                            output << "lw $t0, " << src1->startAddress << "($sp)" << endl;
                            output << "sne $t0,$t0,0\n";
                            output << "sw $t0, " << dst->startAddress << "($sp)" << endl;
                        }
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "sne $t0,$t0,0\n";
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sne $t1,$t1,0\n";
                        output << "and $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }
                case I_Or:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm !=0 | src2->imm !=0) << endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        if (src1->imm == 1){
                            output << "li $t0,1\n";
                            output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                        }else{
                            output << "lw $t0, " << src2->startAddress << "($sp)" << endl;
                            output << "sne $t0,$t0,0\n";
                            output << "sw $t0, " << dst->startAddress << "($sp)" << endl;
                        }
                    }else if (src2->canGetValue){
                        if (src2->imm == 1){
                            output << "li $t0,1\n";
                            output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                        }else{
                            output << "lw $t0, " << src1->startAddress << "($sp)" << endl;
                            output << "sne $t0,$t0,0\n";
                            output << "sw $t0, " << dst->startAddress << "($sp)" << endl;
                        }
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "sne $t0,$t0,0\n";
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sne $t1,$t1,0\n";
                        output << "or $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }

                case I_Eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm == src2->imm)<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                        output << "lw $t0, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "seq $t0 , $t0,"<<src1->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else if (src2->canGetValue){
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "seq $t0 , $t0,"<<src2->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "seq $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }
                case I_Grt_eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  >= src2->imm )<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                        output << "lw $t0, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sle $t0 , $t0,"<<src1->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else if (src2->canGetValue){
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "sge $t0 , $t0,"<<src2->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sge $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }
                case I_Less_eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  <= src2->imm )<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                        output << "lw $t0, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sge $t0 , $t0,"<<src1->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else if (src2->canGetValue){
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "sle $t0 , $t0,"<<src2->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sle $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }

                case I_Grt:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  > src2->imm )<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        //src2需要lw
//                    output << "li $t0, "<< src1->imm<<endl;
                        output << "lw $t0, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "li $t1, "<<src1->imm<< endl;
                        output << "slt $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else if (src2->canGetValue){
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "sgt $t0 , $t0,"<<src2->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sgt $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }
                case I_Less:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  < src2->imm )<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        output << "lw $t0, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sgt $t0 , $t0,"<<src1->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else if (src2->canGetValue){
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "li $t1, "<<src2->imm<< endl;
                        output << "slt $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "slt $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }
                case I_not_eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  != src2->imm )<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else if (src1->canGetValue){
                        output << "lw $t0, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sne $t0 , $t0,"<<src1->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else if (src2->canGetValue){
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "sne $t0 , $t0,"<<src2->imm<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }else{
                        output << "lw $t0, "<<src1->startAddress<<"($sp)"<<endl;
                        output << "lw $t1, "<<src2->startAddress<<"($sp)"<<endl;
                        output << "sne $t0 , $t0, $t1"<<endl;
                        output <<"sw $t0, "<<dst->startAddress <<"($sp)"<<endl;
                    }
                    break;
                }
                case Printf:
                    output << "#printf语句:" << src2->values_Id->size() << "个exp参数\n";
                    cnt_param = 0;
                    for (auto id: *src1->strings_iEntry_id) {
                        /**
     la $a0, str_1
    li $v0, 4
    syscall
    lw $a0, 268501012($sp)
    li $v0, 1
    syscall
                         */
                        if (IEntries.at(id)->str == "%d") {//lw  li  1 syscall
                            IEntry *p = IEntries.at(src2->values_Id->at(cnt_param++));
//                        IEntry * p_val = IEntries.at(p->values_Id->at(0));//VALUE!!!
                            IEntry *p_val = p;
                            if (p_val->canGetValue) {
                                output << "li $a0, " << p_val->imm << endl;
                            } else {
                                output << "lw $a0, " << p_val->startAddress << "($sp)" << endl;
                            }
                            output << "li $v0, 1" << endl;
                            output << "syscall" << endl;
                        } else {
                            //la  li 4 syscall
                            output << "la $a0, " << "str_" << id << endl;
                            output << "li $v0, 4" << endl;
                            output << "syscall" << endl;
                        }

                    }

                    break;
                case Add:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm + src2->imm<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else {
                        if (src1->canGetValue){
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "addu " << "$t2" << ", " << "$t1" << ", " << src1->imm<< endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else if (src2->canGetValue){
                            output << "lw " << "$t1" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "addu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else{
                            output << "lw " << "$t0" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }
                    }
                }
                    break;
                case Sub:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm - src2->imm<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else if (src2->canGetValue){
                            output << "lw " << "$t1" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "subu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else{
                            output << "lw " << "$t0" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }
                    }
                }
                    break;
                case Mult:{
                    output << "#执行乘法\n";
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm * src2->imm<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else if (src2->canGetValue){
                            output << "li " << "$t0" << ", " << src2->imm << endl;
                            output << "lw " << "$t1" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else{
                            output << "lw " << "$t0" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }
                    }
                }
                    break;
                    //除法：HI存放余数，LO存放除法结果
                case Div:{
                    output << "#执行div：\n";
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm / src2->imm<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mflo " << "$t2" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else if (src2->canGetValue){
                            output << "li " << "$t1" << ", " << src2->imm << endl;
                            output << "lw " << "$t0" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mflo " << "$t2" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else{
                            output << "lw " << "$t0" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mflo " << "$t2" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }
                    }
                }
                    break;
                    //TODO:优化
                case Mod:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm % src2->imm<< endl;
                        output << "sw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mfhi " << "$t2" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else if (src2->canGetValue){
                            output << "li " << "$t1" << ", " << src2->imm << endl;
                            output << "lw " << "$t0" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mfhi " << "$t2" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }else{
                            output << "lw " << "$t0" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "lw " << "$t1" << ", " << src2->startAddress << "($sp)" << endl;
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mfhi " << "$t2" << endl;
                            output << "sw " << "$t2" << ", " << dst->startAddress << "($sp)" << endl;
                        }
                    }
                }
                    break;
                case Assign:
                    assign(src1, nullptr, dst);
                    break;
                    //FIXME:总是容易陷入误区 得到v0的值已经是运行时  编译的极限块也做不到预知~
                case GetInt:
                    output << "#getint:\n";
                    output << "li $v0, 5\n";
                    output << "syscall\n";
                    if(dst->type == 6){
                        output << "sw $v0,"<<dst->imm*4<<"($sp)"<<endl;
                    }else if (dst->type == 2){
                        output << "lw " << "$t0" << ", " << dst->startAddress << "($sp)" << endl;
                        output << "sw " << "$v0" << ", " << "0($t0)" << endl;
                    }else{
                        output << "sw " << "$v0" << ", " << dst->startAddress << "($sp)" << endl;//基本不会再被用到了？
                    }
                    dst->canGetValue = false;
                    break;
                case GetAddress:{
                    output<< "#GetTheAddress  sw in dst 's address\n";
                    if (src2->type == 1){
                        output<< "lw $t0,"<<IEntries.at(src2->values_Id->at(0))->startAddress<<"($sp)"<<endl;
                    }else if(src2->type == 7){
                        output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                    }else{
                        if (src2->isGlobal){
                            output<< "la $t0,"<<src2->original_Name<<endl;
                        }else{
                            output<< "li $t0,"<<src2->startAddress<<endl;
                            output << "addu $t0,$t0,$sp"<<endl;
                        }
                    }
                    if (src1){
                        if (src1->canGetValue){
                            output<< "addu $t0,$t0,"<<src1->imm*4<<endl;
                        }else{
                            output<< "lw, $t1,"<<src1->startAddress<<"($sp)"<<endl;
                            output << "sll $t1,$t1,2"<<endl;
                            output<< "addu $t0,$t0,$t1"<<endl;
                        }
                    }
                    output<< "sw $t0,"<<dst->startAddress<<"($sp)"<<endl;
                    break;
                }
                    //TODO：检查格式统一 全都是IEntry格式   可以进行一个canGetElement的优化
                case GetArrayElement:{//FIXME:数组元素的get需要找到元素地址！！！  即本身  而不是值的副本   又或者说成是让定义的数组记住它！！
                    output << "#GetArray Element\n";
                    int index = 0;
                    int isNormalArray = src2->type;
                    if (isNormalArray == 0){//表示array并不是通过函数传递地址而来  即offsetEntry没用 或者认为就是0 即index就是最终索引  s7存放数组元素地址
                        if (src1->canGetValue){
                            index += src1->imm;
                            if (src2->isGlobal){
                                output << "la " << "$t0" << ", " << src2->original_Name << endl;
                                output << "li " << "$t1" << ", " << index * 4 << endl;
                                output << "addu " << "$t0" << ", " << "$t0" << ", " << "$t1" << endl; //value's address in $t2
                            }else{
                                if(src2->type ==7){
                                    output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                                }else{
                                    output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                                    output << "addu $t0, $t0,$sp\n";
                                }
                            }
                            //dst——type  1    0
                            if (dst->type ==0){
                                output << "lw " << "$t0" << ", 0($t0)" << endl;
                                output << "sw " << "$t0, " << dst->startAddress << "($sp)" << endl;
                            }else{
                                //采用地址传递  内容是地址
                                output << "sw " << "$t0, " << dst->startAddress << "($sp)" << endl;
//                                dst->type = 0;
                            }
                        }else{
                            if (src2->isGlobal){
                                output << "la " << "$t0" << ", " << src2->original_Name << endl;
                            }else{
                                if (src2->type ==7){
                                    output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                                }else{
                                    output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                                    output << "addu $t0, $t0,$sp\n";
                                }

                            }
                            output << "lw " << "$t1" << ", " << src1->startAddress << "($sp)" << endl;
                            output << "sll " << "$t1" << ", " << "$t1" << ", 2" << endl;
                            output << "addu " << "$t0" << ", " << "$t0" << ", " << "$t1" << endl; //value's address in $t2
                            if (dst->type ==0){
                                output << "lw " << "$t0" << ", 0($t0)" << endl;
                                output << "sw " << "$t0, " << dst->startAddress << "($sp)" << endl;//此时dst_ptr的IEntry false  需要lw address 来使用
                            }else{
                                //t0地址
                                output << "sw " << "$t0, " << dst->startAddress << "($sp)" << endl;
//                                dst->type = 0;
                            }
                        }
                    }else{//不是normal  出现在自定义函数内部的引用数组  此时src2 会是startAddress offset_Entry
                        if (src2->type == 7){
                            output << "lw " << "$t0" << ", " << src2->imm*4 <<"($sp)" << endl;
                        }else{
                            output<< "lw $t0,"<<src2->startAddress<<"($sp)"<<endl;
//                            output << "addu $t0, $t0,$sp\n";
                        }
                        if (src1){
                            if (src1->canGetValue){
                                output<< "addu $t0,$t0,"<<src1->imm*4<<endl;
                            }else{
                                output<< "lw $t1,"<<src1->startAddress<<"($sp)"<<endl;
                                output << "sll $t1,$t1,2"<<endl;
                                output<< "addu $t0,$t0,$t1"<<endl;
                            }
                        }
                        if(dst->type == 0){
                            output<< "lw $t0,0($t0)"<<endl;
                            output<< "sw $t0,"<<dst->startAddress<<"($sp)"<<endl;
                        }else{
                            output<< "sw $t0,"<<dst->startAddress<<"($sp)"<<endl;
                        }
                    }
                    break;
                }
                case Return: {
                    if (src1 != nullptr) {
                        if (src1->canGetValue) {
                            output << "li " << "$v0" << ", " << src1->imm << endl;
                        } else {
                            output << "lw " << "$v0" << ", " << src1->startAddress << "($sp)" << endl;
                        }
                    }
                    output << "jr " << "$ra" << endl;
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
                        output << "error!!!!  rParam_ids->size() = " << rParam_ids->size() << "fParam_ids->size() = " << fParam_ids->size() << "\n";
                    }
                    output << "#调用函数" << src1->original_Name << ": \n";
                    //ra 在sp中压栈
                    output << "addiu $sp, $sp, -30000\n";
                    output << "sw $ra, 0($sp)\n";
                    IEntry*f,*r;
                    for (int i = 0; i < rParam_ids->size(); i++) {
                        r = IEntries.at(rParam_ids->at(i));
                        if(r->type == 0){
                            if (r->canGetValue){
                                output << "li " << "$t1, " << r->imm << endl;
                            }else{
                                output << "lw " << "$t1, " << r->startAddress+30000 << "($sp)" << endl;
                            }
                            output << "sw " << "$t1, " <<IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress<<"($sp)" << endl;//多存一个return
                        }else {// only == 1
                            output << "lw $t0, " << src1->startAddress+30000 << "($sp)" << endl;
                            output << "sw " << "$t0, " << IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress << "($sp)" << endl;
                        }
                    }

                    output << endl;
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
                    //call function
                    output << "jal " << "Funccccc_" << src1->original_Name << endl;
                    //ra 出栈
                    output << "#返回函数"<<endl;
                    output << "lw $ra, 0($sp)\n";
                    output << "addiu $sp, $sp, 30000\n";
                    //函数返回值在v0中  要sw   其实这里的sw v0 to somewhere 没有用
//                    output << "sw " << "$v0" << ", " << src1->return_IEntry->startAddress << "($sp)"<< endl;//src2 = IEntries.at(func->id)
                    if (dst != nullptr){
                        output << "sw " << "$v0" << ", " << dst->startAddress << "($sp)" << endl;//src2 = IEntries.at(func->id)
                    }
                    break;

                    /**
                     * 函数名标签  就是函数头的名字  形参的IEntry需要在中间代码就生成
                     *  for (auto entry :entries) {
                    func->values_Id->push_back(entry->id);//传递的最终都是IEntry  值或地址！
                }
                     */
                case FuncDef:
                    output << "Funccccc_" << src1->original_Name << ":\n";//函数名标签
                    output << "#" << src1->original_Name << "部分: ";
                    for (auto id: *src1->values_Id) {
                        if (IEntries.at(id)->type == 0) {
                            output << "value:@" << id << " ";
                        } else {
                            output << "address:@" << id << " ";
                        }
                    }
                    output << endl;
                    break;
                    /**
                     * 非全局变量的初始化定义
                     */
                case VAR_Def_Has_Value:
                    output << "#local_var_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  " ;
                    if(IEntries.at(src1->values_Id->at(0))->canGetValue){
                        output << IEntries.at(src1->values_Id->at(0))->imm << " ";
                    }else{
                        output << "@(" << src1->values_Id->at(0) << ")" << " ";
                    }
                    output << endl;
                    break;
                case VAR_Def_No_Value:
                    output << "#local_var_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "no_value_def\n  " ;
                    break;
                case ARRAY_VAR_Def_Has_Value:
                    output << "#local_array_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  " ;
                    for (auto id_init_value:*(src1->values_Id)) {
                        if (IEntries.at(id_init_value)->canGetValue){
                            output << IEntries.at(id_init_value)->imm << " ";
                        }else{
                            output << "@(" << id_init_value << ")" << " ";
                        }
                    }
                    output << endl;
                    cnt = 0;
                    break;
                case ARRAY_Def_No_Value:
                    output << "#local_array_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def\n  " ;
                    break;
                case Const_Def_Has_Value:
                    output << "#const_@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "_def:  " ;
                    for (auto init_id:*(src1->values_Id)) {
                        output << IEntries.at(init_id)->imm << " ";
                    }
                    cnt = 0;
                    output << endl;
                    //const的使用读取 也许不用再使用lw
                    for (auto id_init_value:*(src1->values_Id)) {
                        if (IEntries.at(id_init_value)->canGetValue){ //认为数组内存是连续存储？
                            output << "li " << "$t0" << ",  " << IEntries.at(id_init_value)->imm << endl;
                        }else{
                            output << "lw " << "$t0" << ",  " << IEntries.at(id_init_value)->startAddress << "($sp)" << endl;
                        }
                        output << "sw " << "$t0, " << src1->startAddress + cnt * 4 << "($sp)" << endl;
                        cnt++;
                    }
                    break;
                case ARRAY_CONST_Def_Has_Value:
                    output << "#array_const@" + to_string(ICode->src1->Id) << "_" + src1->original_Name << "def   " ;
                    for (auto init_id:*(src1->values_Id)) {
                        output << IEntries.at(init_id)->imm << " ";
                    }
                    output << endl;
                    break;
                default:
                    break;

            }
        }
    }
// 重定向结束后，可以将outputFile的流恢复到原始状态
//    std::output.rdbuf(outputFileBuffer);

}


