//
// Created by hzt on 2023/11/11.
//

#include "IntermediateCode.h"

int IEntry::generateId() {
    static int  id_generate = 0;
    return id_generate++;
}

string IntermediateCode::iCode2str(ICode *iCode) {
    IEntry *var1 = iCode->src1;
    IEntry *var2 = iCode->src2;
    IEntry *var3 = iCode->dst;
    switch (iCode->type) {
        case Def_Has_Value:
            return "Def_Has_Value :" + var1->name + "type:" + to_string(var1->type) ;
            break;
        case Def_No_Value:
            return "Def_Has_Value :" + var1->name + "type:" + to_string(var1->type) ;
            break;
        case Add:
            return "Add :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case Sub:
            return "Sub :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case Mult:
            return "Mult :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case Div:
            return "Div :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case Mod:
            return "Mod :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case GetInt:
            return "GetInt :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case GetArrayElement:
            return "GetArrayElement :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case FuncCall:
            return "FuncCall :" + var1->name + "type:" + to_string(var1->type)  ;
            break;
        case FuncDef:
            return " FuncDef:" + var1->name + "type:" + to_string(var1->type)  ;
            break;
    }
}

void IntermediateCode::addDef(bool isGlobal, IntermediateCodeType type, IEntry *src1, IEntry *src2, IEntry *dst) {
    iCode = new ICode();
    dst = new IEntry;//最后翻译时 会根据约定的IEntryType去使用IEntry
    iCode->type = type;
    iCode->src1 = src1;
    iCode->src2 = src2;
    iCode->dst = dst;
    if (isGlobal) {
        globalDef.push_back(iCode);
    } else {
        if (isInOtherFunc) {
            if (otherFuncICodes.find(funcLabel) == otherFuncICodes.end()) {
                otherFuncICodes.insert(pair<string, vector<ICode *>>(funcLabel, vector<ICode *>()));
            }
            otherFuncICodes.at(funcLabel).push_back(iCode);
        } else {
            mainICodes.push_back(iCode);
        }
    }
}
void IntermediateCode::addICode(IntermediateCodeType type, IEntry *src1, IEntry *src2, IEntry *dst) {

    iCode = new ICode();
    dst = new IEntry;//最后翻译时 会根据约定的IEntryType去使用IEntry
    iCode->type = type;
    iCode->src1 = src1;
    iCode->src2 = src2;
    iCode->dst = dst;
    if (isInOtherFunc) {
        if (otherFuncICodes.find(funcLabel) == otherFuncICodes.end()) {
            otherFuncICodes.insert(pair<string, vector<ICode *>>(funcLabel, vector<ICode *>()));
        }
        otherFuncICodes.at(funcLabel).push_back(iCode);
    } else {
        mainICodes.push_back(iCode);
    }
}

void IntermediateCode::addICode(IntermediateCodeType type, int src1, IEntry *src2, IEntry *dst) {
    iCode = new ICode();
    dst = new IEntry;
    iCode->type = type;
    auto* s1 = new IEntry;
    s1->imm = src1;
    iCode->src1 = s1;
    iCode->src2 = src2;
    iCode->dst = dst;
    if (isInOtherFunc){
        if (otherFuncICodes.find(funcLabel) == otherFuncICodes.end()){
            otherFuncICodes.insert(pair<string,vector<ICode*>>(funcLabel,vector<ICode*>()));
        }
        otherFuncICodes.at(funcLabel).push_back(iCode);
    }else{
        mainICodes.push_back(iCode);
    }
}


void IntermediateCode::debug_print() {
    cout<<"#全局变量"<<endl;
    for (auto def:globalDef) {
        //输出iCode的类型、和非空的IEntry
        cout<< iCode2str(def) << endl;
    }
    cout<<"#主函数"<<endl;
    //print mainICodes
    for (auto item:mainICodes) {
        //输出iCode的类型、和非空的IEntry
        cout << iCode2str(item) << endl;
    }
    cout<<"#其他自定义函数"<<endl;
    //按顺序输出otherFuncICodes的每一个FuncLabel的ICodes
    for (const auto& func:otherFuncICodes) {
        cout<<"#自定义函数名："+func.first<<endl;
        for (auto item:func.second) {
            cout << iCode2str(item) << endl;

        }
    }
}

