//
// Created by hzt on 2023/11/11.
//

#include "IntermediateCode.h"
extern int funcInitAddress;
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
    this->name = "Labellllll_"+s+"_"+ to_string(this->Id);//label输出iEntry->name
//    this->values_Id = new vector<int>;
//    this->values = new vector<int>;
//    this->imm = 0;
//    tempMemoryAddressTop += 4;
    IEntries.push_back(this);
//         iEntry->address
}

IEntry::IEntry(){
    this->Id = generateId();
    if (isInOtherFunc){
        this->startAddress = tempMemoryAddressTop - funcInitAddress;
    }else{
        this->startAddress = tempMemoryAddressTop;
    }
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
    if (isInOtherFunc){
        this->startAddress = tempMemoryAddressTop - funcInitAddress;
    }else{
        this->startAddress = tempMemoryAddressTop;
    }
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
    if (isInOtherFunc){
        if (otherFuncICodes.find(funcLabel) == otherFuncICodes.end()){
            otherFuncICodes.insert(pair<string,vector<ICode*>>(funcLabel,vector<ICode*>()));
        }
        otherFuncICodes.at(funcLabel).push_back(iCode);
    }else{
        mainICodes.push_back(iCode);
    }
}

/**
 * 在你之前的版本中，当你使用mainICodes.erase(mainICodes.begin()+i+1)时，你删除了一个元素，并且通过i--试图回退索引。然而，由于你的循环会在下一次迭代中自增i，这可能导致跳过一个元素。这是因为erase操作会使容器内的元素重新排列，而你的代码中并没有很好地处理这种情况。

使用迭代器的好处在于，当你在循环中删除元素时，迭代器会被适当地更新，避免了直接使用索引带来的混乱。在上述修改中，我使用了it = mainICodes.erase(std::next(it));，这样可以确保迭代器指向正确的位置。

总的来说，使用迭代器更加安全，因为它们可以适应容器结构的变化，而不需要手动管理索引。这有助于防止因删除元素而引起的错误。
 */
void IntermediateCode::optimize1() {
    // 扫描所有的中间代码，对相邻的assign语句进行合并
    unsigned int before_main_cnt = mainICodes.size();
    unsigned int after_main_cnt = 0;
    while(before_main_cnt != after_main_cnt){
        before_main_cnt = mainICodes.size();
        for (auto it = mainICodes.begin(); it != mainICodes.end(); ++it) {
            if (std::next(it) != mainICodes.end()) {
                auto cur = *it;
                auto next = *std::next(it);
                if (cur->type == Assign && next->type == Assign && cur->dst->Id == next->src1->Id ) {
                    // 合并optimize
                    cur->dst = next->dst;
                    it = mainICodes.erase(std::next(it));
                    --it;  // 回退迭代器，保证不会跳过元素
                }
            }
        }
        after_main_cnt = mainICodes.size();
    }




    for (auto& func : otherFuncICodes) {
        unsigned int before_func_cnt = func.second.size();
        unsigned int after_func_cnt = 0;
        while(before_func_cnt != after_func_cnt ){
            before_func_cnt = func.second.size();
            for (auto it = func.second.begin(); it != func.second.end(); ++it) {
                if (std::next(it) != func.second.end()) {
                    auto cur = *it;
                    auto next = *std::next(it);
                    if (cur->type == Assign && next->type == Assign && cur->dst->Id == next->src1->Id) {
                        // 合并
                        cur->dst = next->dst;
                        it = func.second.erase(std::next(it));
                        --it;  // 回退迭代器，保证不会跳过元素
                    }
                }
            }
            after_func_cnt = func.second.size();
        }
    }
}
int log2OfPowerOfTwo(int n) {
    // 如果不是2的幂次，返回-1或采取其他适当的处理
    if (n <= 0 || (n & (n - 1)) != 0) {
        //error
        return -1;
    }

    int count = 0;
    while (n > 1) {
        n >>= 1;
        count++;
    }

    return count;
}
//mod 2^k  div 2^k转化成移位运算
void IntermediateCode::optimize2() {
    for (auto iCode:mainICodes) {
        int d = 0 ;
        if (iCode->src2) {
            d = iCode->src2->imm;
        }
        if (iCode->type == Div &&iCode->src2->canGetValue && d>0 && (d&(d-1))==0) {
            iCode->type = Right_Shift;
            iCode->src2->imm = log2OfPowerOfTwo(d);
        }else if(iCode->type == Mult &&iCode->src2->canGetValue && d>0 && (d&(d-1))==0){
            iCode->type = Left_Shift;
            iCode->src2->imm = log2OfPowerOfTwo(d);
        }else if(iCode->type == Mod &&iCode->src2->canGetValue && d>0 && (d&(d-1))==0){
            //not sure
        }
    }
    //对于自定义函数同样进行优化
    for (auto& func : otherFuncICodes) {
        for (auto iCode:func.second) {
            int d = 0 ;
            if (iCode->src2) {
                d = iCode->src2->imm;
            }
            if (iCode->type == Div &&iCode->src2->canGetValue && d>0 && (d&(d-1))==0) {
                iCode->type = Right_Shift;
                iCode->src2->imm = log2OfPowerOfTwo(d);
            }else if(iCode->type == Mult &&iCode->src2->canGetValue && d>0 && (d&(d-1))==0){
                iCode->type = Left_Shift;
                iCode->src2->imm = log2OfPowerOfTwo(d);
            }else if(iCode->type == Mod &&iCode->src2->canGetValue && d>0 && (d&(d-1))==0){
                //not sure
            }
        }
    }
}

void IntermediateCode::optimize3() {
    //对于四元式中的临时变量，如果其只在一个地方被使用，那么就可以将其删除
    unsigned int before_main_cnt = mainICodes.size();
    unsigned int after_main_cnt = 0;
    while(before_main_cnt != after_main_cnt){
        before_main_cnt = mainICodes.size();
        for (auto it = mainICodes.begin(); it != mainICodes.end(); ++it) {
            if (std::next(it) != mainICodes.end()) {
                auto cur = *it;
                auto next = *std::next(it);
                if ((cur->type == Add || cur->type == Sub || cur->type == Mult || cur->type == Div || cur->type == Mod || cur->type == GetArrayElement || cur->type == GetAddress || cur->type == I_Not || cur->type == I_Eq ||cur->type
                                                                                                                                                                                                                               == I_not_eq || cur->type == I_Grt || cur->type == I_Grt_eq || cur->type == I_Or || cur->type == I_And || cur->type == I_Less || cur->type == I_Less_eq || cur->type == Right_Shift || cur->type == Left_Shift)
                    && next->type == Assign && next->dst->type != 2 && cur->dst->Id == next->src1->Id ) {
                    // 合并
                    cur->dst = next->dst;
                    it = mainICodes.erase(std::next(it));
                    --it;  // 回退迭代器，保证不会跳过元素
                }
                //void 函数的返回值iEntry不需要assign
                if (cur->type == FuncCall && !cur->src1->has_return && next->type == Assign ) {
                    // del assign
                    it = mainICodes.erase(std::next(it));
                    --it;  // 回退迭代器，保证不会跳过元素
                }else if(cur->type == FuncCall && cur->src1->has_return && next->type == Assign && cur->dst->Id == next->src1->Id && next->dst->type != 2){
                    // merge assign
                    cur->dst = next->dst;
                    it = mainICodes.erase(std::next(it));
                    --it;  // 回退迭代器，保证不会跳过元素
                }


            }
        }
        after_main_cnt = mainICodes.size();
    }

    for (auto& func : otherFuncICodes) {
        unsigned int before_func_cnt = func.second.size();
        unsigned int after_func_cnt = 0;
        while(before_func_cnt != after_func_cnt ) {
            before_func_cnt = func.second.size();
            for (auto it = func.second.begin(); it != func.second.end(); ++it) {
                if (std::next(it) != func.second.end()) {
                    auto cur = *it;
                    auto next = *std::next(it);
                    if ((cur->type == Add || cur->type == Sub || cur->type == Mult || cur->type == Div || cur->type == Mod || cur->type == GetArrayElement || cur->type == GetAddress || cur->type == I_Not || cur->type == I_Eq ||cur->type
                                                                                                                                                                                                                                   == I_not_eq || cur->type == I_Grt || cur->type == I_Grt_eq || cur->type == I_Or || cur->type == I_And || cur->type == I_Less || cur->type == I_Less_eq || cur->type == Right_Shift || cur->type == Left_Shift)
                        && next->type == Assign && next->dst->type != 2&& cur->dst->Id == next->src1->Id ) {
                        // 合并
                        cur->dst = next->dst;
                        it = func.second.erase(std::next(it));
                        --it;  // 回退迭代器，保证不会跳过元素
                    }
                    //void 函数的返回值iEntry不需要assign
                    if (cur->type == FuncCall && !cur->src1->has_return && next->type == Assign) {
                        // del assign
                        it = func.second.erase(std::next(it));
                        --it;  // 回退迭代器，保证不会跳过元素
                    }else if(cur->type == FuncCall && cur->src1->has_return && next->type == Assign && cur->dst->Id == next->src1->Id && next->dst->type != 2){
                        // merge assign
                        cur->dst = next->dst;
                        it = func.second.erase(std::next(it));
                        --it;  // 回退迭代器，保证不会跳过元素
                    }
                }
            }
            after_func_cnt = func.second.size();
        }

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

