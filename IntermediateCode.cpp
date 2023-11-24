//
// Created by hzt on 2023/11/11.
//

#include "IntermediateCode.h"
vector<int> strings;
vector<ICode *> mainICodes ;
map<string, vector<ICode *>> otherFuncICodes;
vector<ICode *>globalDef ;
vector<IEntry *> IEntries;
int tempMemoryAddressTop = 268500992;

int IEntry::generateId() {
    static int  id_generate = 0;
    return id_generate++;
}
IEntry::IEntry(const string& s){
    this->Id = generateId();
    this->type =5;
//    this->startAddress = tempMemoryAddressTop;
    this->name = "Label_"+s+"_"+ to_string(this->Id);//label输出iEntry->name
//    this->values_Id = new vector<int>;
//    this->values = new vector<int>;
//    this->imm = 0;
//    tempMemoryAddressTop += 4;
    IEntries.push_back(this);
//         iEntry->address
}

IEntry::IEntry(){
    this->Id = generateId();
    this->startAddress = tempMemoryAddressTop;
    this->name = "@"+ to_string(this->Id);
    this->values_Id = new vector<int>;
    this->values = new vector<int>;
    this->imm = 0;
    tempMemoryAddressTop += 4;
    IEntries.push_back(this);
//         iEntry->address
}
//FIXME数组的定义时的IEntry的生成  type = 0  只有地址时才是1
 IEntry::IEntry(int length){
    this->Id = generateId();
    this->startAddress = tempMemoryAddressTop;
    this->values_Id = new vector<int>;
    this->values = new vector<int>;
    this->name = "@"+ to_string(this->Id);
    this->imm = 0;
    tempMemoryAddressTop += length*4;
     IEntries.push_back(this);
//         iEntry->address
}

string IntermediateCode::iCode2str(ICode *iCode) {
    IEntry *var1 = iCode->src1;
    IEntry *var2 = iCode->src2;
    IEntry *var3 = iCode->dst;
    switch (iCode->type) {
        case VAR_Def_Has_Value:
            return "define var_has_value_or: id, " + to_string(var1->Id)+ " value:"+ to_string(IEntries.at(var1->values_Id->at(0))->imm);//非const都先直接用IEntry存起来 后端生成时再解包
        case VAR_Def_No_Value:
            return "define var_no_value : id, " + to_string(var1->Id);
        case ARRAY_VAR_Def_Has_Value:
            return "define var_array_has_value : id, " + to_string(var1->Id);
        case ARRAY_Def_No_Value:
            return "define var_array_has_value : id, " + to_string(var1->Id);
        case Const_Def_Has_Value:
            return "define const_has_value : id, " + to_string(var1->Id) + " value: " + to_string(var1->values->at(0));
        case ARRAY_CONST_Def_Has_Value:
            return "define const_array_has_value : id," + to_string(var1->Id);
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
//    dst = new IEntry;//最后翻译时 会根据约定的IEntryType去使用IEntry
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
    iCode->type = type;
    iCode->src1 = src1;
    iCode->src2 = src2;
    iCode->dst = dst;
    if (dst){
        dst->canGetValue = false;
    }
    if (isInOtherFunc) {
        if (otherFuncICodes.find(funcLabel) == otherFuncICodes.end()) {
            otherFuncICodes.insert(pair<string, vector<ICode *>>(funcLabel, vector<ICode *>()));
        }
        otherFuncICodes.at(funcLabel).push_back(iCode);
    } else {
        mainICodes.push_back(iCode);
    }
}

//TODO:dst需要运行时才知道value了   dst确保进入时已经new过
void IntermediateCode::addICode(IntermediateCodeType type, int src1, IEntry *src2, IEntry *dst) {
    iCode = new ICode();
    iCode->type = type;
    auto* s1 = new IEntry;
    s1->canGetValue = true;
    s1->imm = src1;
    iCode->src1 = s1;
    iCode->src2 = src2;
    iCode->dst = dst;
    if (dst){
        dst->canGetValue = false;
    }
    if (isInOtherFunc){
        if (otherFuncICodes.find(funcLabel) == otherFuncICodes.end()){
            otherFuncICodes.insert(pair<string,vector<ICode*>>(funcLabel,vector<ICode*>()));
        }
        otherFuncICodes.at(funcLabel).push_back(iCode);
    }else{
        mainICodes.push_back(iCode);
    }
}


//void IntermediateCode::debug_print() {
//    output<<"#全局变量"<<endl;
//    for (auto def:globalDef) {
//        //输出iCode的类型、和非空的IEntry
//        output<< iCode2str(def) << endl;
//    }
//    output<<"#主函数"<<endl;
//    //print mainICodes
//    for (auto item:mainICodes) {
//        //输出iCode的类型、和非空的IEntry
//        output << iCode2str(item) << endl;
//    }
//    output<<"#其他自定义函数"<<endl;
//    //按顺序输出otherFuncICodes的每一个FuncLabel的ICodes
//    for (const auto& func:otherFuncICodes) {
//        output<<"#自定义函数名："+func.first<<endl;
//        for (auto item:func.second) {
//            output << iCode2str(item) << endl;
//
//        }
//    }
//}

