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
extern int log2OfPowerOfTwo(int n);
queue<Reg> regPoolsUsed;
map<Reg,int>RegInfo = {
//        {Reg::$zero, -1},
//        {Reg::$at,   -1},
//        {Reg::$v0,   -1},
        {Reg::$v1,   -1},
//        {Reg::$a0,   -1},
        {Reg::$a1,   -1},
        {Reg::$a2,   -1},
        {Reg::$a3,   -1},
//        {Reg::$t0,   -1},
//        {Reg::$t1,   -1},
//        {Reg::$t2,   -1},
        {Reg::$t3,   -1},
        {Reg::$t4,   -1},
        {Reg::$t5,   -1},
        {Reg::$t6,   -1},
        {Reg::$t7,   -1},
        {Reg::$t8,   -1},
        {Reg::$t9,   -1},
        {Reg::$s0,   -1},
        {Reg::$s1,   -1},
        {Reg::$s2,   -1},
        {Reg::$s3,   -1},
        {Reg::$s4,   -1},
        {Reg::$s5,   -1},
        {Reg::$s6,   -1},
        {Reg::$s7,   -1},
//        {Reg::$k0,   -1},
//        {Reg::$k1,   -1},
//        {Reg::$gp,   -1},
//        {Reg::$sp,   -1},
//        {Reg::$fp,   -1},
//        {Reg::$ra,   -1}
};

//TODO:更改lw sw load store func

void MipsCode::assign(IEntry *src1,IEntry *src2,IEntry *dst) { //传进来需要已经values_Id处理好
    /*
     * 值传递  地址赋
     * TODO:需要对全局变量的分类讨论   局部变量也是    写入值编译时不确定时
     * TODO：多次写入的更新原则！！！
     * */
    output << "#assign" << endl;
    //TODO:  参数废弃
     if (src1->type == 0){
        if (src1->isGlobal){
            output << "la $t1," << src1->original_Name << endl;
        }else if (src1->canGetValue){
            output << "li " << "$t1, " << src1->imm << endl;
        }else{
            loadIEntry(src1,Reg::$t1);
        }
        if (dst->type == 2){//dst的值 是地址 即使是函数中也已经由于GETARRAY 加上了sp  正在考虑是不是可以和type == 1合并 不能！！
            if (assignSp){
                loadIEntry(dst,Reg::$t2);
                output << "sw " << "$t1, " <<  "0($t2)" << endl;
            }else{
                loadIEntry(dst,Reg::$t2);
                output << "sw " << "$t1, " << "0($t2)" << endl; //  需要从变量的IEntry的values_Id中取得的  t0  地址  为写语句而生
            }
        }else{
            storeIEntry(dst,Reg::$t1);
        }
            dst->canGetValue =  false;//后台更新
    }else{
        output << "#地址拷贝\n";
         loadIEntry(src1,Reg::$t0);
         storeIEntry(dst,Reg::$t0);
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

    output << ".data 0x10010000\n";
    output << "temppppppppppppp:  .space  160000\n\n";  // 临时内存区，起始地址为0x10010000 (16) or 268500992 (10)
     output << "#strings in printf\n";
    for (auto id: strings) {
        output << "str_" << id << ": .asciiz " << "\"" << IEntries.at(id)->str << "\"" << endl;
    }
    for (auto def:globalDef) {
        IntermediateCodeType type = def->type;
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
                for (int i = 0;i<def->src1->total_length;i++) {
                    output << "0 ";
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
        int cnt_param;//for printf_exp
        int cnt = 0;//for def sw address
        vector<int> *rParam_ids ;
        vector<int> *fParam_ids ;

        switch (type) {
            case Right_Shift:{
                loadIEntry(src1,Reg::$t0);
                output << "slt $t1,$t0,$zero"<<endl;
                output << "beqz $t1, "<<"Test_neg_"<<src1->Id<<endl;
                output << "addu $t0,$t0,"<< (1<<src2->imm)-1<<endl;
                output << "Test_neg_"<<src1->Id<<":"<<endl;
                output << "sra $t0, $t0, "<<src2->imm << endl;
                storeIEntry(dst,Reg::$t0);
                break;
            }
            case Left_Shift:{
                loadIEntry(src1,Reg::$t0);
                output << "sll $t0, $t0,  "<<src2->imm<< endl;
                storeIEntry(dst,Reg::$t0);
                break;
            }
            //#################################################################################//
            case I_Not:{
                loadIEntry(src1,Reg::$t0);
                output << "seq $t0, $t0,0"<<endl;//0
                storeIEntry(dst,Reg::$t0);
                break;
            }
            case Beq:{
                loadIEntry(src1,Reg::$t0);
                loadIEntry(src2,Reg::$t1);
                output << "beq $t0, $t1,"<<dst->name<<endl;
                break;
            }
            case Beqz:{
                loadIEntry(src1,Reg::$t0);
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
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    if (src1->imm == 0){
                        storeIEntry(dst,Reg::$zero);
                    }else {
                        loadIEntry(src2,Reg::$t0);
                        output << "sne $t0,$t0,0\n";
                        storeIEntry(dst,Reg::$t0);
                    }
                }else if (src2->canGetValue){
                    if (src2->imm == 0){
                        storeIEntry(dst,Reg::$zero);
                    }else {
                        loadIEntry(src1,Reg::$t0);
                        output << "sne $t0,$t0,0\n";
                        storeIEntry(dst,Reg::$t0);
                    }
                }else{
                    loadIEntry(src1,Reg::$t0);
                    output << "sne $t0,$t0,0\n";
                    loadIEntry(src2,Reg::$t1);
                    output << "sne $t1,$t1,0\n";
                    output << "and $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }
                break;
            }
            case I_Or:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm!=0| src2->imm!=0) << endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    if (src1->imm == 1){
                        output << "li $t0,1\n";
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        loadIEntry(src2,Reg::$t0);
                        output << "sne $t0,$t0,0\n";
                        storeIEntry(dst,Reg::$t0);
                    }
                }else if (src2->canGetValue){
                    if (src2->imm == 1){
                        output << "li $t0,1\n";
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        output << "lw $t0, " << src1->startAddress << "($zero)" << endl;
                        output << "sne $t0,$t0,0\n";
                        storeIEntry(dst,Reg::$t0);
                    }
                }else{
                    /**
                     * %%%%
                     */
                    loadIEntry(src1,Reg::$t0);
                    output << "sne $t0,$t0,0\n";
                    loadIEntry(src2,Reg::$t1);
                    output << "sne $t1,$t1,0\n";
                    output << "or $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }
                break;
            }

            case I_Eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm == src2->imm)<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    loadIEntry(src2,Reg::$t0);
                    output << "seq $t0 , $t0,"<<src1->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src2->canGetValue){
                    loadIEntry(src1,Reg::$t0);
                    output << "seq $t0 , $t0,"<<src2->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else{
                    loadIEntry(src1,Reg::$t0);
                    loadIEntry(src2,Reg::$t1);
                    output << "seq $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }
                break;
            }
            case I_Grt_eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  >= src2->imm )<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    loadIEntry(src2,Reg::$t0);
                    output << "sle $t0 , $t0,"<<src1->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src2->canGetValue){
                    loadIEntry(src1,Reg::$t0);
                    output << "sge $t0 , $t0,"<<src2->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else{
                    loadIEntry(src1,Reg::$t0);
                    loadIEntry(src2,Reg::$t1);
                    output << "sge $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }
                break;
            }
            case I_Less_eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  <= src2->imm )<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    loadIEntry(src2,Reg::$t0);
                    output << "sge $t0 , $t0,"<<src1->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src2->canGetValue){
                    loadIEntry(src1,Reg::$t0);
                    output << "sle $t0 , $t0,"<<src2->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else{
                    loadIEntry(src1,Reg::$t0);
                    loadIEntry(src2,Reg::$t1);
                    output << "sle $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }
                break;
            }

            case I_Grt:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  > src2->imm )<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    loadIEntry(src2,Reg::$t0);
                    output << "li $t1, "<<src1->imm<< endl;
                    output << "slt $t0 , $t0, $t1"<<endl;//故意反一下符号 结果正确   slti不适用
                    storeIEntry(dst,Reg::$t0);
                }else if (src2->canGetValue){
                    loadIEntry(src1,Reg::$t0);
                    output << "sgt $t0 , $t0,"<<src2->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else{
                    loadIEntry(src1,Reg::$t0);
                    loadIEntry(src2,Reg::$t1);
                    output << "sgt $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }
                break;
            }
            case I_Less:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  < src2->imm )<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    loadIEntry(src2,Reg::$t0);
                    output << "sgt $t0 , $t0,"<<src1->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src2->canGetValue){
                    loadIEntry(src1,Reg::$t0);
                    output << "li $t1, "<<src2->imm<< endl;
                    output << "slt $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else{
                    loadIEntry(src1,Reg::$t0);
                    loadIEntry(src2,Reg::$t1);
                    output << "slt $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
                }
                break;
            }
            case I_not_eq:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << (src1->imm  != src2->imm )<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src1->canGetValue){
                    loadIEntry(src2,Reg::$t0);
                    output << "sne $t0 , $t0,"<<src1->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else if (src2->canGetValue){
                    loadIEntry(src1,Reg::$t0);
                    output << "sne $t0 , $t0,"<<src2->imm<<endl;
                    storeIEntry(dst,Reg::$t0);
                }else{
                    loadIEntry(src1,Reg::$t0);
                    loadIEntry(src2,Reg::$t1);
                    output << "sne $t0 , $t0, $t1"<<endl;
                    storeIEntry(dst,Reg::$t0);
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
                        IEntry * p_val = p;
                        if (p_val->canGetValue){
                            output << "li $a0, " << p_val->imm << endl;
                        }else{
                            loadIEntry(p_val,Reg::$a0);
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
                    storeIEntry(dst,Reg::$t0);
                }else {
                    if (src1->canGetValue){
                        loadIEntry(src2,Reg::$t1);
                        output << "addu " << "$t2" << ", " << "$t1" << ", " << src1->imm<< endl;
                        storeIEntry(dst,Reg::$t2);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t1);
                        output << "addu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                        storeIEntry(dst,Reg::$t2);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }
                }
            }
                break;
            case Sub:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm - src2->imm<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        loadIEntry(src2,Reg::$t1);
                        output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t1);
                        output << "subu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                        storeIEntry(dst,Reg::$t2);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }
                }
            }
                break;
            case Mult:{
                output << "#执行乘法\n";
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm * src2->imm<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        loadIEntry(src2,Reg::$t1);
                        output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }else if (src2->canGetValue){
                        output << "li " << "$t0" << ", " << src2->imm << endl;
                        loadIEntry(src1,Reg::$t1);
                        output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }
                }
            }
                break;
            //除法：HI存放余数，LO存放除法结果
            case Div:{
                output << "#执行div：\n";
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm / src2->imm<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        loadIEntry(src2,Reg::$t1);
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mflo " << "$t2" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }else if (src2->canGetValue){
                        output << "li " << "$t1" << ", " << src2->imm << endl;
                        loadIEntry(src1,Reg::$t0);
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mflo " << "$t2" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mflo " << "$t2" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }
                }
            }
                break;
            //TODO:优化
            case Mod:{
                if (src1->canGetValue && src2->canGetValue){
                    output << "li " << "$t0" << ", " << src1->imm % src2->imm<< endl;
                    storeIEntry(dst,Reg::$t0);
                }else {
                    if (src1->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm << endl;
                        loadIEntry(src2,Reg::$t1);
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mfhi " << "$t2" << endl;
                        storeIEntry(dst,Reg::$t2);
                    }else if (src2->canGetValue){
                        int d = src2->imm;
                        if ((d&(d-1)) == 0 &&d >0) {
                            loadIEntry(src1,Reg::$t0);
                            output << "sgt $t1, $t0, $zero"<< endl;
                            output << "beqz $t1, "<<"Mod_"<<src1->Id<<endl;

                            output << "andi $t0, $t0, "<< ((1<< log2OfPowerOfTwo(d))-1)<< endl;
                            output << "j "<<"Mod_end_"<<src1->Id<<endl;

                            output << "Mod_"<<src1->Id<<":"<<endl;
                            output << "subu $t0, $zero, $t0"<< endl;
                            output << "andi $t0, $t0, "<< ((1<< log2OfPowerOfTwo(d))-1)<< endl;
                            output << "subu $t0, $zero, $t0"<< endl;
                            output << "Mod_end_"<<src1->Id<<":"<<endl;
                            storeIEntry(dst,Reg::$t0);
                        }
                        else{
                            output << "li " << "$t1" << ", " << src2->imm << endl;
                            loadIEntry(src1,Reg::$t0);
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mfhi " << "$t2" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "div " << "$t0" << ", " << "$t1" << endl;
                        output << "mfhi " << "$t2" << endl;
                        storeIEntry(dst,Reg::$t2);
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
                if (dst->type == 2){
                    loadIEntry(dst,Reg::$t0);
                    output << "sw " << "$v0" << ", " << "0($t0)" << endl;
                }else{
                    storeIEntry(dst,Reg::$t0);
                }
                dst->canGetValue = false;
                break;
                //TODO：检查格式统一 全都是IEntry格式   可以进行一个canGetElement的优化
            case GetAddress:{
                output<< "#GetTheAddress  sw in dst 's address\n";
                if (src2->type == 1){
                    loadIEntry(IEntries.at(src2->values_Id->at(0)),Reg::$t0);
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
                        loadIEntry(src1,Reg::$t1);
                        output << "sll $t1,$t1,2"<<endl;
                        output<< "addu $t0,$t0,$t1"<<endl;
                    }
                }
                storeIEntry(dst,Reg::$t0);
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
                                output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                        }
                        //dst——type  1    0
                        if (dst->type ==0){
                            output << "lw " << "$t0" << ", 0($t0)" << endl;
                            storeIEntry(dst,Reg::$t0);
                        }else{
                            //采用地址传递  内容是地址
                            storeIEntry(dst,Reg::$t0);
                        }
                    }else{
                        if (src2->isGlobal){
                            output << "la " << "$t0" << ", " << src2->original_Name << endl;
                        }else{
                                output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                        }
                        loadIEntry(src1,Reg::$t1);
                        output << "sll " << "$t1" << ", " << "$t1" << ", 2" << endl;
                        output << "addu " << "$t0" << ", " << "$t0" << ", " << "$t1" << endl; //value's address in $t2
                        if (dst->type ==0){
                            output << "lw " << "$t0" << ", 0($t0)" << endl;
                            storeIEntry(dst,Reg::$t0);
                        }else{
                            //t0地址
                            storeIEntry(dst,Reg::$t0);
                        }
                    }
                }else{//不是normal  出现在自定义函数内部的引用数组  此时src2 会是startAddress offset_Entry
                    loadIEntry(src2,Reg::$t0);
                    if (src1){
                        if (src1->canGetValue){
                            output<< "addu $t0,$t0,"<<src1->imm*4<<endl;
                        }else{
                            loadIEntry(src1,Reg::$t1);
                            output << "sll $t1,$t1,2"<<endl;
                            output<< "addu $t0,$t0,$t1"<<endl;
                        }
                    }
                    if(dst->type == 0){
                        output<< "lw $t0,0($t0)"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        storeIEntry(dst,Reg::$t0);
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
                output << "addiu $sp, $sp, -100000\n";
                output << "sw $ra, 0($sp)\n";
                IEntry*f,*r;
                for (int i = 0; i < rParam_ids->size(); i++) {
                    r = IEntries.at(rParam_ids->at(i));
                    if(r->type == 0){
                        if (r->canGetValue){
                            output << "li " << "$t1, " << r->imm << endl;
                        }else{
//                            output << "lw " << "$t1, " << r->startAddress<<"($zero)" << endl;
                            loadIEntry(r,Reg::$t1);
                        }
//                        storeIEntry(IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0)),Reg::$t1);
                        output << "sw " << "$t1, " <<  IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress<<"($sp)" << endl;
                    }else {// only == 1
//                        output << "lw $t0, " << src1->startAddress << "($zero)" << endl;
                        loadIEntry(src1,Reg::$t0);
                        output << "sw " << "$t0, " <<  IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress<< "($sp)" << endl;
                    }
                }
                output << endl;
                /**
                 * # Pushing Function Real Params:
addiu $sp, $sp, -100000
sw $ra, 0($sp)
# Call function!
jal Label_1
lw $ra, 0($sp)
# Pop params
addiu $sp, $sp, 100000
                 */
                //call function
                output << "jal " << "Funccccc_" << src1->original_Name << endl;
                //ra 出栈
                output << "#返回函数"<<endl;
                output << "lw $ra, 0($sp)\n";
                output << "addiu $sp, $sp, 100000\n";
                if (dst != nullptr){
                    storeIEntry(dst,Reg::$v0);
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
                output << endl;
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

    output << "#自定义函数的代码ICode\n";
    for (const auto& func_codes:otherFuncICodes) {
    output<< "#clearRegPool()\n";
/**
 * 寄存器池需要清空
 */
        clearRegPool();
        assignSp  =true;
//        output << "_" << func_codes.first << ":\n";  第一行总是函数的定义头
        for (auto ICode: func_codes.second) {
            IntermediateCodeType type = ICode->type;
            IEntry *src1 = ICode->src1;
            IEntry *src2 = ICode->src2;
            IEntry *dst = ICode->dst;
            int cnt_param;//for printf_exp
            int cnt = 0;//for def sw address
            vector<int> *rParam_ids;
            vector<int> *fParam_ids;

            switch (type) {
                case Right_Shift:{
                    loadIEntry(src1,Reg::$t0);
                    output << "slt $t1,$t0,$zero"<<endl;
                    output << "beqz $t1, "<<"Test_neg_"<<src1->Id<<endl;
                    output << "addu $t0,$t0,"<< (1<<src2->imm)-1<<endl;
                    output << "Test_neg_"<<src1->Id<<":"<<endl;
                    output << "sra $t0, $t0," <<src2->imm<< endl;
                    storeIEntry(dst,Reg::$t0);
                    break;
                }
                case Left_Shift:{
                    loadIEntry(src1,Reg::$t0);
                    output << "sll $t0, $t0," << src2->imm <<endl;
                    storeIEntry(dst,Reg::$t0);
                    break;
                }
                //#############################################
                case I_Not:{
                    loadIEntry(src1,Reg::$t0);
                    output << "seq $t0, $t0,0"<<endl;//0
                    storeIEntry(dst,Reg::$t0);
                    break;
                }
                case Beqz:{
                    loadIEntry(src1,Reg::$t0);
                    output << "beqz $t0, "<<src2->name<<endl;
                    break;
                }
                case Beq:{
                    loadIEntry(src1,Reg::$t0);
                    loadIEntry(src2,Reg::$t1);
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
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        if (src1->imm ==0){
                            storeIEntry(dst,Reg::$zero);
                        }else{
                            loadIEntry(src2,Reg::$t0);
                            output << "sne $t0,$t0,0\n";
                            storeIEntry(dst,Reg::$t0);
                        }
                    }else if (src2->canGetValue){
                        if (src2->imm ==0){
                            output << "li $t0,0\n";
                            storeIEntry(dst,Reg::$t0);
                        }else{
                            loadIEntry(src1,Reg::$t0);
                            output << "sne $t0,$t0,0\n";
                            storeIEntry(dst,Reg::$t0);
                        }
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        output << "sne $t0,$t0,0\n";
                        loadIEntry(src2,Reg::$t1);
                        output << "sne $t1,$t1,0\n";
                        output << "and $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }
                    break;
                }
                case I_Or:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm !=0 | src2->imm !=0) << endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        if (src1->imm == 1){
                            output << "li $t0,1\n";
                            storeIEntry(dst,Reg::$t0);
                        }else{
                            loadIEntry(src2,Reg::$t0);
                            output << "sne $t0,$t0,0\n";
                            storeIEntry(dst,Reg::$t0);
                        }
                    }else if (src2->canGetValue){
                        if (src2->imm == 1){
                            output << "li $t0,1\n";
                            storeIEntry(dst,Reg::$t0);
                        }else{
                            loadIEntry(src1,Reg::$t0);
                            output << "sne $t0,$t0,0\n";
                            storeIEntry(dst,Reg::$t0);
                        }
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        output << "sne $t0,$t0,0\n";
                        loadIEntry(src2,Reg::$t1);
                        output << "sne $t1,$t1,0\n";
                        output << "or $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }
                    break;
                }

                case I_Eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm == src2->imm)<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        loadIEntry(src2,Reg::$t0);
                        output << "seq $t0 , $t0,"<<src1->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t0);
                        output << "seq $t0 , $t0,"<<src2->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "seq $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }
                    break;
                }
                case I_Grt_eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  >= src2->imm )<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        loadIEntry(src2,Reg::$t0);
                        output << "sle $t0 , $t0,"<<src1->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t0);
                        output << "sge $t0 , $t0,"<<src2->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "sge $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }
                    break;
                }
                case I_Less_eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  <= src2->imm )<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        loadIEntry(src2,Reg::$t0);
                        output << "sge $t0 , $t0,"<<src1->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t0);
                        output << "sle $t0 , $t0,"<<src2->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "sle $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }
                    break;
                }

                case I_Grt:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  > src2->imm )<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        loadIEntry(src2,Reg::$t0);
                        output << "li $t1, "<<src1->imm<< endl;
                        output << "slt $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t0);
                        output << "sgt $t0 , $t0,"<<src2->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "sgt $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }
                    break;
                }
                case I_Less:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  < src2->imm )<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        loadIEntry(src2,Reg::$t0);
                        output << "sgt $t0 , $t0,"<<src1->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t0);
                        output << "li $t1, "<<src2->imm<< endl;
                        output << "slt $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "slt $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }
                    break;
                }
                case I_not_eq:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << (src1->imm  != src2->imm )<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src1->canGetValue){
                        loadIEntry(src2,Reg::$t0);
                        output << "sne $t0 , $t0,"<<src1->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else if (src2->canGetValue){
                        loadIEntry(src1,Reg::$t0);
                        output << "sne $t0 , $t0,"<<src2->imm<<endl;
                        storeIEntry(dst,Reg::$t0);
                    }else{
                        loadIEntry(src1,Reg::$t0);
                        loadIEntry(src2,Reg::$t1);
                        output << "sne $t0 , $t0, $t1"<<endl;
                        storeIEntry(dst,Reg::$t0);
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
                            IEntry *p_val = p;
                            if (p_val->canGetValue) {
                                output << "li $a0, " << p_val->imm << endl;
                            } else {
                                loadIEntry(p_val,Reg::$a0);
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
                        storeIEntry(dst,Reg::$t0);
                    }else {
                        if (src1->canGetValue){
                            loadIEntry(src2,Reg::$t1);
                            output << "addu " << "$t2" << ", " << "$t1" << ", " << src1->imm<< endl;
                            storeIEntry(dst,Reg::$t2);
                        }else if (src2->canGetValue){
                            loadIEntry(src1,Reg::$t1);
                            output << "addu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                            storeIEntry(dst,Reg::$t2);
                        }else{
                            loadIEntry(src1,Reg::$t0);
                            loadIEntry(src2,Reg::$t1);
                            output << "addu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }
                    }
                }
                    break;
                case Sub:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm - src2->imm<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            loadIEntry(src2,Reg::$t1);
                            output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }else if (src2->canGetValue){
                            loadIEntry(src1,Reg::$t1);
                            output << "subu " << "$t2" << ", " << "$t1" << ", " << src2->imm<< endl;
                            storeIEntry(dst,Reg::$t2);
                        }else{
                            loadIEntry(src1,Reg::$t0);
                            loadIEntry(src2,Reg::$t1);
                            output << "subu " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }
                    }
                }
                    break;
                case Mult:{
                    output << "#执行乘法\n";
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm * src2->imm<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            loadIEntry(src2,Reg::$t1);
                            output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }else if (src2->canGetValue){
                            output << "li " << "$t0" << ", " << src2->imm << endl;
                            loadIEntry(src1,Reg::$t1);
                            output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }else{
                            loadIEntry(src1,Reg::$t0);
                            loadIEntry(src2,Reg::$t1);
                            output << "mul " << "$t2" << ", " << "$t0" << ", " << "$t1" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }
                    }
                }
                    break;
                    //除法：HI存放余数，LO存放除法结果
                case Div:{
                    output << "#执行div：\n";
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm / src2->imm<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            loadIEntry(src2,Reg::$t1);
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mflo " << "$t2" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }else if (src2->canGetValue){
                            output << "li " << "$t1" << ", " << src2->imm << endl;
                            loadIEntry(src1,Reg::$t0);
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mflo " << "$t2" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }else{
                            loadIEntry(src1,Reg::$t0);
                            loadIEntry(src2,Reg::$t1);
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mflo " << "$t2" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }
                    }
                }
                    break;
                    //TODO:优化
                case Mod:{
                    if (src1->canGetValue && src2->canGetValue){
                        output << "li " << "$t0" << ", " << src1->imm % src2->imm<< endl;
                        storeIEntry(dst,Reg::$t0);
                    }else {
                        if (src1->canGetValue){
                            output << "li " << "$t0" << ", " << src1->imm << endl;
                            loadIEntry(src2,Reg::$t1);
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mfhi " << "$t2" << endl;
                            storeIEntry(dst,Reg::$t2);
                        }else if (src2->canGetValue){
                            int d = src2->imm;
                            if ((d&(d-1)) == 0 && d>0) {
                                loadIEntry(src1,Reg::$t0);
                                output << "sgt $t1, $t0, $zero"<< endl;
                                output << "beqz $t1, "<<"Mod_"<<src1->Id<<endl;

                                output << "andi $t0, $t0, "<< ((1<< log2OfPowerOfTwo(d))-1)<< endl;
                                output << "j "<<"Mod_end_"<<src1->Id<<endl;

                                output << "Mod_"<<src1->Id<<":"<<endl;
                                output << "subu $t0, $zero, $t0"<< endl;
                                output << "andi $t0, $t0, "<< ((1<< log2OfPowerOfTwo(d))-1)<< endl;
                                output << "subu $t0, $zero, $t0"<< endl;

                                output << "Mod_end_"<<src1->Id<<":"<<endl;
                                storeIEntry(dst,Reg::$t2);
                            }
                            else{
                                output << "li " << "$t1" << ", " << src2->imm << endl;
                                loadIEntry(src1,Reg::$t0);
                                output << "div " << "$t0" << ", " << "$t1" << endl;
                                output << "mfhi " << "$t2" << endl;
                                storeIEntry(dst,Reg::$t2);
                            }
                        }else{
                            loadIEntry(src1,Reg::$t0);
                            loadIEntry(src2,Reg::$t1);
                            output << "div " << "$t0" << ", " << "$t1" << endl;
                            output << "mfhi " << "$t2" << endl;
                            storeIEntry(dst,Reg::$t2);
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
                    if (dst->type == 2){
                        loadIEntry(dst,Reg::$t0);
                        output << "sw " << "$v0" << ", " << "0($t0)" << endl;
                    }else{
                        storeIEntry(dst,Reg::$v0);
                    }
                    dst->canGetValue = false;
                    break;
                case GetAddress:{
                    output<< "#GetTheAddress  sw in dst 's address\n";
                    if (src2->type == 1){
                        loadIEntry(IEntries.at(src2->values_Id->at(0)),Reg::$t0);
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
                            loadIEntry(src1,Reg::$t1);
                            output << "sll $t1,$t1,2"<<endl;
                            output<< "addu $t0,$t0,$t1"<<endl;
                        }
                    }
                    storeIEntry(dst,Reg::$t0);
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
                                output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                                output << "addu $t0, $t0,$sp\n";
                            }
                            //dst——type  1    0
                            if (dst->type ==0){
                                output << "lw " << "$t0" << ", 0($t0)" << endl;
                                storeIEntry(dst,Reg::$t0);
                            }else{
                                //采用地址传递  内容是地址
                                storeIEntry(dst,Reg::$t0);
                            }
                        }else{
                            if (src2->isGlobal){
                                output << "la " << "$t0" << ", " << src2->original_Name << endl;
                            }else{
                                output << "li " << "$t0" << ", " << src2->startAddress + index * 4 << endl;
                                output << "addu $t0, $t0,$sp\n";
                            }
                            loadIEntry(src1,Reg::$t1);
                            output << "sll " << "$t1" << ", " << "$t1" << ", 2" << endl;
                            output << "addu " << "$t0" << ", " << "$t0" << ", " << "$t1" << endl; //value's address in $t2
                            if (dst->type ==0){
                                output << "lw " << "$t0" << ", 0($t0)" << endl;
                                storeIEntry(dst,Reg::$t0);
                            }else{
                                //t0地址
                                storeIEntry(dst,Reg::$t0);
                            }
                        }
                    }else{//不是normal  出现在自定义函数内部的引用数组  此时src2 会是startAddress offset_Entry
                        loadIEntry(src2,Reg::$t0);
                        if (src1){
                            if (src1->canGetValue){
                                output<< "addu $t0,$t0,"<<src1->imm*4<<endl;
                            }else{
                                loadIEntry(src1,Reg::$t1);
                                output << "sll $t1,$t1,2"<<endl;
                                output<< "addu $t0,$t0,$t1"<<endl;
                            }
                        }
                        if(dst->type == 0){
                            output<< "lw $t0,0($t0)"<<endl;
                            storeIEntry(dst,Reg::$t0);
                        }else{
                            storeIEntry(dst,Reg::$t0);
                        }
                    }
                    break;
                }
                case Return: {
                    if (src1 != nullptr) {
                        if (src1->canGetValue) {
                            output << "li " << "$v0" << ", " << src1->imm << endl;
                        } else {
                            loadIEntry(src1,Reg::$v0);
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
                    output << "addiu $sp, $sp, -100000\n";
                    output << "sw $ra, 0($sp)\n";
                    IEntry*f,*r;
                    for (int i = 0; i < rParam_ids->size(); i++) {
                        r = IEntries.at(rParam_ids->at(i));
                        if(r->type == 0){
                            if (r->canGetValue){
                                output << "li " << "$t1, " << r->imm << endl;
                            }else{
                                loadIEntry(r,Reg::$t1);
//                                output << "lw " << "$t1, " << r->startAddress+100000 << "($sp)" << endl;
                            }
//                            storeIEntry(IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0)),Reg::$t1);
                            storeIEntry(IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0)),Reg::$t1);
//                            output << "sw " << "$t1, " <<IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress<<"($sp)" << endl;//多存一个return
                        }else {// only == 1
                            output << "lw $t0, " << src1->startAddress+100000 << "($sp)" << endl;
//                            storeIEntry(IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0)),Reg::$t0);
                            output << "sw " << "$t0, " << IEntries.at( IEntries.at(fParam_ids->at(i))->values_Id->at(0))->startAddress << "($sp)" << endl;
                        }
                    }

                    output << endl;
                    /**
                     * # Pushing Function Real Params:
    addiu $sp, $sp, -100000
    sw $ra, 0($sp)
    # Call function!
    jal Label_1
    lw $ra, 0($sp)
    # Pop params
    addiu $sp, $sp, 100000
                     */
                    //call function
                    output << "jal " << "Funccccc_" << src1->original_Name << endl;
                    //ra 出栈
                    output << "#返回函数"<<endl;
                    output << "lw $ra, 0($sp)\n";
                    output << "addiu $sp, $sp, 100000\n";

                    if (dst != nullptr){
                        storeIEntry(dst,Reg::$v0);
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
}


//TODO:还需要注意 la 全局变量的地址
void MipsCode::loadIEntry(IEntry *iEntry, Reg toReg) {
    int id;
    id = iEntry->Id;
    Reg fromReg = canFindInReg(iEntry);
    if( fromReg != Reg::$zero){
        output << "move " << reg2s.at(toReg)<< ", " << reg2s.at(fromReg) << endl;
    }else{
        if (iEntry->canGetValue) {
            output << "li " << reg2s.at(toReg) << ", " << iEntry->imm << endl;
        } else {
            if (assignSp){
                output << "lw " << reg2s.at(toReg) << ", " << iEntry->startAddress << "($sp)" << endl;
            }else{
                output << "lw " << reg2s.at(toReg) << ", " << iEntry->startAddress << "($zero)" << endl;
            }
        }
        //TODO：对寄存器池进行优化 不是替换    而是记录 把这个寄存器的值存到RegInfo中   可能move 的利益更大  也不一定
    }
}

void MipsCode::storeIEntry(IEntry *to_iEntry, Reg fromReg) {
    int id;
    id = to_iEntry->Id;
    Reg toReg = hasOneRegToStore();
    if( toReg != Reg::$zero){
        output<<"#find a reg to store:\n";
        output << "move " << reg2s.at(toReg)<< ", " << reg2s.at(fromReg) << endl;
        regPoolsUsed.push(toReg);
        RegInfo[toReg] = id;
    }else{
        int toSwId;
        Reg outReg = regPoolsUsed.front();
        toSwId = RegInfo.at(outReg);
        IEntry* toSwIEntry = IEntries.at(toSwId);
        output << "#release the outReg:"<< reg2s.at(outReg)<<", store new value"<< endl;
        if (assignSp){
            output << "sw " << reg2s.at(outReg) << ", " << toSwIEntry->startAddress << "($sp)" << endl;
        }else{
            output << "sw " << reg2s.at(outReg) << ", " << toSwIEntry->startAddress << "($zero)" << endl;
        }
        regPoolsUsed.pop();

        output << "move " << reg2s.at(outReg)<< ", " << reg2s.at(fromReg) << endl;
        regPoolsUsed.push(outReg);
        RegInfo[outReg] = id;
    }
}

Reg MipsCode::hasOneRegToStore() {
    //搜索RegInfo的寄存器池  还有没有可以存的寄存器
    for (auto regInfo:RegInfo) {
        if (regInfo.second == -1) {
            return regInfo.first;
        }
    }
    return Reg::$zero;
}


Reg MipsCode::canFindInReg(IEntry *iEntry) {
    //搜索RegInfo的寄存器池  second有没有等于iEntry的id
    int id = iEntry->Id;
    for (auto regInfo:RegInfo) {
        if (regInfo.second == id) {
            return regInfo.first;
        }
    }
    return Reg::$zero;
}

void MipsCode::clearRegPool() {
    for (auto &item: RegInfo) {
        if (item.second != -1) {
            Reg outReg = item.first;
            IEntry * toSwIEntry = IEntries.at(item.second);
            /**
             * sw 返回
             */
            if (assignSp){
                output << "sw " << reg2s.at(outReg) << ", " << toSwIEntry->startAddress << "($sp)" << endl;
            }else{
                output << "sw " << reg2s.at(outReg) << ", " << toSwIEntry->startAddress << "($zero)" << endl;
            }
            item.second = -1;
        }

    }
}


