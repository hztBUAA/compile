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



    /**
     * 输出其他函数的代码ICode
     */

}


