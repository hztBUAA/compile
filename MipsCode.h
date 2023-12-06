//
// Created by hzt on 2023/11/11.
//

#ifndef LEX_MIPSCODE_H
#define LEX_MIPSCODE_H

#include <queue>
#include "IntermediateCode.h"
using namespace std;
enum class Reg {
    $zero,

    $at,

    $v0, $v1,
    $a0, $a1, $a2, $a3,

    $t0, $t1, $t2, $t3, $t4, $t5, $t6, $t7, $t8, $t9,

    $s0, $s1, $s2, $s3, $s4, $s5, $s6, $s7,

    $k0, $k1,

    $gp,

    $sp,

    $fp,

    $ra
};

const std::map<Reg, std::string> reg2s = {
        {Reg::$zero, "$zero"},
        {Reg::$at,   "$at"},
        {Reg::$v0,   "$v0"},
        {Reg::$v1,   "$v1"},
        {Reg::$a0,   "$a0"},
        {Reg::$a1,   "$a1"},
        {Reg::$a2,   "$a2"},
        {Reg::$a3,   "$a3"},
        {Reg::$t0,   "$t0"},
        {Reg::$t1,   "$t1"},
        {Reg::$t2,   "$t2"},
        {Reg::$t3,   "$t3"},
        {Reg::$t4,   "$t4"},
        {Reg::$t5,   "$t5"},
        {Reg::$t6,   "$t6"},
        {Reg::$t7,   "$t7"},
        {Reg::$t8,   "$t8"},
        {Reg::$t9,   "$t9"},
        {Reg::$s0,   "$s0"},
        {Reg::$s1,   "$s1"},
        {Reg::$s2,   "$s2"},
        {Reg::$s3,   "$s3"},
        {Reg::$s4,   "$s4"},
        {Reg::$s5,   "$s5"},
        {Reg::$s6,   "$s6"},
        {Reg::$s7,   "$s7"},
        {Reg::$k0,   "$k0"},
        {Reg::$k1,   "$k1"},
        {Reg::$gp,   "$gp"},
        {Reg::$sp,   "$sp"},
        {Reg::$fp,   "$fp"},
        {Reg::$ra,   "$ra"}
};





class MipsCode {
private:
    ofstream &output;
    IntermediateCode &intermediateCode;

public:
    //TODO: 用于存储全局变量和数组的内存地址  MIPS只用使用IEntry中的地址
    explicit MipsCode(IntermediateCode &intermediateCode1,ofstream&output1):intermediateCode(intermediateCode1),output(output1){};
    /**
     * 根据type进行中间代码到MIPS的翻译
     */
    void assign(IEntry *src1,IEntry *src2,IEntry *dst) ;
    void translate();

    /**
     * 一些被traslate调用的工具代码 如：lw、sw、add、sub、mul、div、beq、bne、bge、bgt、ble、blt、j、jal、jr、li、la、move、mflo、mfhi、syscall
     */
    void loadIEntry(IEntry* iEntry,Reg toReg);

    void loadAddress(int iEntry_address,Reg toReg);

    void storeIEntry(IEntry* iEntry,Reg fromReg);
    Reg canFindInReg(IEntry* iEntry);
    void testRe();
    void clearRegPool();
    Reg hasOneRegToStore();
};


#endif //LEX_MIPSCODE_H
