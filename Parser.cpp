//
// Created by hzt on 2023/9/25.
//



#include "Parser.h"
#include "Lexer.h"
#include <iostream>
using namespace std;

int funcInitAddress = 0;

string funcLabel;
bool isInOtherFunc = false;//区分中间代码是在主函数还是自定义函数   --注意定义还要分一个全局--既不是主函数 也不是
extern vector<ICode *> mainICodes ;
extern vector<ICode *>globalDef ;
extern int  tempMemoryAddressTop;
void Parser::Print_Grammar_Output(const string& s) {
    if (enablePrint){

        //ouputFile << s << endl;
        lexer.outputFile << s << endl;
    }
}


void Parser::CompUnit() {
/**
 * 这是起始部分   识别符号  先读取一个     需要放权！！！！
 * 不应该这样直接全部包揽  允许一定的回溯  在当前那里判断就行
 * 按自己的想法去写
 *
 */
    GET_A_WORD;


    while(WORD_TYPE != FINISH){
        if ( WORD_TYPE == CONSTTK){ //Decl
            Decl();//常量声明部分入口是const
        }
        else if (WORD_TYPE == INTTK ){
            PRINT_WORD;
            //PRINT INT   这里之所以输出当前CompUnit右边产生式没有的终结符  是因为不把它当做下一级的非终结符 将它提前到父亲这一级
            GET_A_WORD;
            if (WORD_TYPE == MAINTK){ //MainFuncDef  int main()
                MainFuncDef();
            }else if(WORD_TYPE == IDENFR){ // int [ident]
                /**
                 *
                 * 无需peek   a letter 读取完ident后    ch已经指向下一个字符了
                 */
                //lexer.ch = PEEK_A_LETTER;
                int pre_line = lexer.line;
                while(isspace(lexer.ch)){
                    if(lexer.ch == '\n'){
                        lexer.line++;
                    }
                    lexer.ch = (char )lexer.sourceFile.get();
                }
                if (lexer.ch == '('){//FuncDef 从名字<ident>开始  还没有输出functype
                    //FUNCTYPE
                    FuncType();
                    FuncDef(FUNC_INT);
                }else if(lexer.ch == ',' || lexer.ch == ';'  || lexer.ch == '=' || lexer.ch == '['){
                    Decl();//变量声明部分  入口是ident
                }else{
                    //Error  缺少;
                    PRINT_WORD;//print ident
                    GET_A_WORD;
                    lexer.line_lastWord = pre_line;
                    errorHandler.Insert_Error(SEMICOLON_MISSING);

                }
            }
        }else if(WORD_TYPE == VOIDTK){
            PRINT_WORD;//PRINT VOID
            GET_A_WORD;
            FuncType();
            if(WORD_TYPE != IDENFR){
                //ERROR
            }
            FuncDef(FUNC_VOID);
        }
        else if (WORD_TYPE == NOTE){
            GET_A_WORD;
        }
    }
    Print_Grammar_Output("<CompUnit>");
}
void Parser::Decl(){
    //不可以自调用  在更高层进行循环   Decl只负责一次  由CompUnit多次调用
    //进入VarDecl  or  ConstDecl
    //VAR从ident开始  还未PRINT IDENT
    //CONST 从CONST开始
    if (WORD_TYPE == CONSTTK){
        ConstDecl();//常量声明部分  const开始  还未输出const
    }else if(WORD_TYPE == IDENFR){
        //lexer.ch = PEEK_A_LETTER;
        /**
         * 问题-1
         */
//        while (isspace(lexer.ch)){
//            lexer.ch = lexer.sourceFile.get();
//        }
//        if(lexer.ch == ',' || lexer.ch == ';'  || lexer.ch == '=' || lexer.ch == '['){
//            VarDecl();//变量声明部分
//        }
        VarDecl();
    }else{
        //Error
    }
}

void Parser::VarDecl() {
    //to VarDef
    //start from    正指向ident
    vector<Entry*> entries;
   VarDef(entries);//有错误就下放到下一层 按照文法编写
   while(WORD_TYPE == COMMA){
       PRINT_WORD;//PRINT ,
       GET_A_WORD;
       VarDef(entries);
   }
    if (WORD_TYPE != SEMICN){
        //Error;  缺少分号
        errorHandler.Insert_Error(SEMICOLON_MISSING);
//        semantic.skip(VARDECL);
//        semantic.recordEntries(entries);
        return;
    }

    //不缺少分号 会运行到这儿
    PRINT_WORD;//PRINT ;
    Print_Grammar_Output("<VarDecl>");
//    semantic.recordEntries(entries);
    GET_A_WORD;//POINT TO NEXT
}

void Parser::VarDef(vector<Entry*> &entries) {
    /**
     * WORD_TYPE正指向 IDENTFR
     */
     bool error = false;
    if (WORD_TYPE != IDENFR){
        //Error
    }
    string ident = WORD_DISPLAY;
    int op = 0;
    //定义：先在这一层找   找到就报错REDEFINE然后跳到；        没找到就填表
    if(tableManager.isRedefine(ident) || inArguments(entries,ident)){
        errorHandler.Insert_Error(REDEFINE);
        error = true;
    }//重定义错误，局部化处理

    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    int values[3];
    IEntry* exp_iEntries[3];
    while(WORD_TYPE == LBRACK){
        op++;
        PRINT_WORD;//PRINT [
        GET_A_WORD;
        exp_iEntries[op] = new IEntry;
        ConstExp(exp_iEntries[op],values[op],isInOtherFunc); //FIXME不要再相信values   就是无效值
        if (WORD_TYPE == RBRACK){
            PRINT_WORD;//PRINT ]
            GET_A_WORD;
        }else{
            //Error  缺少右中括号
            errorHandler.Insert_Error(RBRACK_MISSING);
//            error =true;
        }
    }
    int nums = 0;//数组的长度  会在扫描初始值时设定  同时本身也会设定
    IEntry * iEntry;
    int total_length,dim1_length;
    if (op == 0){
        total_length = dim1_length = 1;
        iEntry = new IEntry;
    }else if(op == 1){
        total_length = dim1_length = exp_iEntries[1]->imm;
        iEntry = new IEntry(total_length);
    }else if(op == 2){
        total_length = exp_iEntries[1]->imm*exp_iEntries[2]->imm;//FIXME；数组初始大小
        dim1_length = exp_iEntries[2]->imm;
        iEntry = new IEntry(total_length);
    }
    iEntry->dim1_length = dim1_length;
    iEntry->total_length = total_length;
    Entry * entry;
    switch (op) {
        case 0:
            entry = semantic.fillInfoEntry(ident,VAR);
            break;
        case 1:
            entry = semantic.fillInfoEntry(ident,ARRAY_1_VAR);
            break;
        case 2:
            entry = semantic.fillInfoEntry(ident,ARRAY_2_VAR);
            break;
        default:
            break;
    }
    bool hasValue = false;
    nums = 0;
    iEntry->original_Name = ident;
    if (WORD_TYPE == ASSIGN){
        hasValue = true;
        PRINT_WORD;
        GET_A_WORD;
        iEntry->values_Id = new vector<int>;//values_Id给变量数组
        InitVal(iEntry,nums);
    }else{
        iEntry->values_Id = new vector<int>;
        IEntry*p;
        for (int i = 0; i <total_length;i++){
            iEntry->values_Id->push_back((p = new IEntry)->Id);
            //exp_iEntry->original_Name = iEntry->original_Name.append("_").append(to_string(nums)).append("_");
            p->original_Name = iEntry->original_Name;
            p->startAddress = iEntry->startAddress + 4*i;
            p->canGetValue = true;
            p->imm = 0;
//            IEntry *v;
//            v = new IEntry;
//            v->imm = 0;
//            v->canGetValue = true;
//            intermediateCode.addICode(Assign,v, nullptr,p);
        }
    }
    if(!error){
        entries.push_back(entry);
        semantic.recordEntries(entry);
        entry->id = iEntry->Id;
        if (ISGLOBAL){
            iEntry->isGlobal = true;//MIPS依据这个生成标签或者地址  lw  FIXME:标签采用   originalName_id：
        }
        if (hasValue){
            if (op == 0){
                intermediateCode.addDef(ISGLOBAL,VAR_Def_Has_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
            }else{
                intermediateCode.addDef(ISGLOBAL,ARRAY_VAR_Def_Has_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
            }
        }else{
            if (op == 0){
                //后台更新全局变量的初始值
                if (ISGLOBAL){
                    IEntries.at(iEntry->values_Id->at(0))->canGetValue = true;
                    IEntries.at(iEntry->values_Id->at(0))->imm = 0;
                }
                intermediateCode.addDef(ISGLOBAL,VAR_Def_No_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
            }else{
                if (ISGLOBAL){
                    for (auto id: *iEntry->values_Id) {
                        IEntries.at(id)->canGetValue = true;
                        IEntries.at(id)->imm = 0;
                    }
                }
                intermediateCode.addDef(ISGLOBAL,ARRAY_Def_No_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
            }

        }
    }
    Print_Grammar_Output("<VarDef>");
}

//当前指向const
void Parser::ConstDecl() {
    //
    PRINT_WORD;//PRINT CONST
    GET_A_WORD;
    //当前指向BType（int）
    if (WORD_TYPE != INTTK){
        //Error
    }
    PRINT_WORD;//PRINT INT
    GET_A_WORD;//当前指向ident
    if (WORD_TYPE != IDENFR){
        //Error
    }
    vector<Entry*> entries;
    ConstDef(entries);// point to the ident and into ConstDef()
    while(WORD_TYPE == COMMA){
        PRINT_WORD;//PRINT COMMA
        GET_A_WORD;
        ConstDef(entries);
    }
    if (WORD_TYPE != SEMICN){
        //Error
        errorHandler.Insert_Error(SEMICOLON_MISSING);
//        semantic.skip(CONSTDECL);
//        semantic.recordEntries(entries);
        return;
    }

    PRINT_WORD;//PRINT ;
    Print_Grammar_Output("<ConstDecl>");
//    tableManager.insertIntoTable(entries);
    GET_A_WORD;//point to next word
}

void Parser::ConstDef(vector<Entry*>& entries) {
    if (WORD_TYPE != IDENFR){
        //Error
    }
    bool error = false;
    string ident = WORD_DISPLAY;
    //定义：先在这一层找   找到就报错REDEFINE  没找到就填表
    if(tableManager.isRedefine(ident) || inArguments(entries,ident)){
        errorHandler.Insert_Error(REDEFINE);
        error = true;
    }

    PRINT_WORD;
    GET_A_WORD;
    int op = 0;

    IEntry * exp_iEntries[3];
    int values[3];
    while(WORD_TYPE == LBRACK){
        op++;
        PRINT_WORD;
        GET_A_WORD;
        exp_iEntries[op] = new IEntry;
        ConstExp(exp_iEntries[op],values[op],isInOtherFunc);
        if (WORD_TYPE == RBRACK){
            PRINT_WORD;
            GET_A_WORD;
        }else{
            //Error
            errorHandler.Insert_Error(RBRACK_MISSING);
//            error = true;
        }
    }
    int nums = 0;//数组的长度  会在扫描初始值时设定  同时本身也会设定
    IEntry * iEntry;
    int total_length,dim1_length;
    if (op == 0){
        total_length = dim1_length = 1;
        iEntry = new IEntry;
    }else if(op == 1){
        total_length = dim1_length = exp_iEntries[1]->imm;
        iEntry = new IEntry(total_length);
    }else if(op == 2){
        total_length = exp_iEntries[1]->imm*exp_iEntries[2]->imm;//FIXME；数组初始大小
        dim1_length = exp_iEntries[2]->imm;
        iEntry = new IEntry(total_length);
    }
    iEntry->dim1_length = dim1_length;
    iEntry->total_length = total_length;
    Entry * entry;
    switch (op) {
        case 0:
            entry = semantic.fillInfoEntry(ident,CONST);
            break;
        case 1:
            entry = semantic.fillInfoEntry(ident,ARRAY_1_CONST);
            break;
        case 2:
            entry = semantic.fillInfoEntry(ident,ARRAY_2_CONST);
            break;
        default:
            break;
    }
    nums = 0;
    iEntry->original_Name = ident;
//常量定义  必须赋值
    if (WORD_TYPE != ASSIGN){
        //Error
    }else{
        PRINT_WORD;
        GET_A_WORD;
        //FIXME:这里的iEntry可以作为存放初值  --数组  --普通变量
        iEntry->values_Id = new vector<int>;
//        iEntry->values = new vector<int>;
        ConstInitVal(iEntry,nums);
    }
    //留下对数组长度的补充计算  以及变量值的存储
    //已经指向下一个word  ,  or ;
    //FIXME:目前认为values只保存在IEntry中   TODO：需要注意同步IEntry和Entry的关系
    if(!error){
        entries.push_back(entry);
        semantic.recordEntries(entry);
        entry->id = iEntry->Id;
        if(ISGLOBAL){
            iEntry->isGlobal = true;
        }
        if (op == 0){
            intermediateCode.addDef(ISGLOBAL,Const_Def_Has_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
        }else{
            intermediateCode.addDef(ISGLOBAL,ARRAY_CONST_Def_Has_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
        }
    }
    Print_Grammar_Output("<ConstDef>");
}

//FIXME:至多只有两层   append
//FIXME:value表示目前的个数   dim1_length表示第一层维数的大小   ConstInitVa中的iEntry指向的是数组变量的IEntry
void Parser::ConstInitVal(IEntry *iEntry,int&nums) {
    if (WORD_TYPE == LBRACE){
        PRINT_WORD;//PRINT {
        GET_A_WORD;
        ConstInitVal(iEntry, nums);
        while(WORD_TYPE == COMMA){
            PRINT_WORD;//PRINT ,
            GET_A_WORD;
            ConstInitVal(iEntry, nums);
        }
        if(WORD_TYPE != RBRACE){
            //Error
        }
        PRINT_WORD;//PRINT }
        GET_A_WORD;
    }else{
        int value;
        auto *v= new IEntry;
        auto* _constExp = new IEntry;//true canGet 会在内部进行设置
        ConstExp(_constExp, value, isInOtherFunc);
        if (ISGLOBAL){
            iEntry->values_Id->push_back(_constExp->Id);
        }else{
            v->startAddress = iEntry->startAddress + 4*nums;
            v->original_Name = iEntry->original_Name;
            iEntry->values_Id->push_back(v->Id);//值要不统一存到values中  定义时  因为你不知道是数组还是啥 TODO： imm是确定的中间变量再用？
            intermediateCode.addICode(Assign,_constExp, nullptr,v);
        }
        nums++;
    }
    Print_Grammar_Output("<ConstInitVal>");
}

//FIXME:至多只有两层   append
//FIXME:nums表示目前的个数  一个ConstExp只有一个值   value表示ConstExp当前的算出值
void Parser::ConstExp(IEntry *iEntry,int&value,bool InOtherFunc) {
    AddExp(iEntry, value, InOtherFunc);
//    iEntry->values->push_back(value);//FIXME: ConstExp一定可以算出来
    Print_Grammar_Output("<ConstExp>");
}

//Exp->AddExp
//AddExp->MulExp{[+-]MulExp}
//认为iEntry由下一级反馈  如果是空 说明值已经算出来放进了value   非空说明值为临时变量
void Parser::AddExp(IEntry *iEntry,int&value,bool iInOtherFunc) {
    int value1,value2;
    IEntry*iEntry1 ,*iEntry2 ;
    iEntry1 = new IEntry;
    //TODO:理解iEntry的原理
    MulExp(iEntry1, value1, iInOtherFunc);

    //FIXME:iEntry代表当前语法成分的代表组件  可以存地址 ----只在函数形参实参出现     也可以存值---可以编译时已经算出来的--也可以是运行时
    //:FIXME:iEntry只有在最底层能确定部件时才会指向生成的iEntry (new ) 需要根据情况设定属性 op默认为值0） 如果value值有效需要置canGetValue = true 默认为false)
    if (WORD_TYPE == PLUS || WORD_TYPE == MINU){
        Exp_type =0;
        IEntry *ans;
        while(WORD_TYPE == PLUS || WORD_TYPE == MINU){
            int op = WORD_TYPE == PLUS? 0:1;
            if (!isLValInStmt)
                Print_Grammar_Output("<AddExp>");
            PRINT_WORD;//print + -
            GET_A_WORD;
            iEntry2 = new IEntry;
            MulExp(iEntry2, value2, iInOtherFunc);
            if (op == 0) {
                if (ISGLOBAL || iEntry1->canGetValue&&iEntry2->canGetValue) {
                    iEntry1->imm = iEntry1->imm + iEntry2->imm;
                    iEntry1->canGetValue = true;
                } else {
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Add, iEntry1, iEntry2, ans);
                    iEntry1 = ans;
                }
            }else if(op ==1){
                if (ISGLOBAL|| iEntry1->canGetValue&&iEntry2->canGetValue) {
                    iEntry1->imm = iEntry1->imm - iEntry2->imm;
                    iEntry1->canGetValue = true;
                } else {
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Sub, iEntry1, iEntry2, ans);
                    iEntry1 = ans;
                }
            }

        }

    }else{
        //error
    }


    if (!isLValInStmt){
        if (iEntry1->canGetValue){
            iEntry->canGetValue = true;
            iEntry->imm = iEntry1->imm;
        }else{
            intermediateCode.addICode(Assign,iEntry1,nullptr,iEntry);
            iEntry->canGetValue = false;
        }
        Print_Grammar_Output("<AddExp>");
    }

    //已经指向下一个
}
//MulExp是项  下面有因子
//MulExp →UnaryExp [*/%] {UnaryExp}
void Parser::MulExp(IEntry *iEntry,int&value,bool InOtherFunc) {
    int value1,value2;
     IEntry *iEntry1, *iEntry2;
    IEntry * ans;
    iEntry1 = new IEntry;
    UnaryExp(iEntry1, value1, InOtherFunc);
    if (WORD_TYPE == MULT || WORD_TYPE == DIV || WORD_TYPE == MOD){
        Exp_type = 0;
        while(WORD_TYPE == MULT || WORD_TYPE == DIV || WORD_TYPE == MOD){
            int op;
            switch (WORD_TYPE) {
                case MULT:
                    op = 0;
                    break;
                case DIV:
                    op = 1;
                    break;
                case MOD:
                    op = 2;
                    break;
                default:
                    op = -1;
                    break;
            }

            if (!isLValInStmt)
                Print_Grammar_Output("<MulExp>");
            PRINT_WORD;//print the */%
            GET_A_WORD;
            iEntry2 = new IEntry;
            UnaryExp(iEntry2, value2, InOtherFunc);
            if (op == 0){
                if (ISGLOBAL|| iEntry1->canGetValue&&iEntry2->canGetValue){
                    iEntry1->canGetValue = true;
                    iEntry1->imm = iEntry1->imm * iEntry2->imm;
                }else{
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Mult, iEntry1, iEntry2, ans);
                    iEntry1 = ans;
                }

            }else if(op ==1){
                if (ISGLOBAL|| iEntry1->canGetValue&&iEntry2->canGetValue){
                    iEntry1->canGetValue = true;
                    iEntry1->imm = iEntry1->imm / iEntry2->imm;
                }else{
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Div, iEntry1, iEntry2, ans);
                    iEntry1 = ans;
                }
            }else{
                if (ISGLOBAL|| iEntry1->canGetValue&&iEntry2->canGetValue){
                    iEntry1->canGetValue = true;
                    iEntry1->imm = iEntry1->imm % iEntry2->imm;
                }else{
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Mod, iEntry1, iEntry2, ans);
                    iEntry1 = ans;
                }
            }

        }
    }else{
        //error
    }

    if (!isLValInStmt){
        if (iEntry1->canGetValue){
            iEntry->canGetValue = true;
            iEntry->imm = iEntry1->imm;
        }else{
            intermediateCode.addICode(Assign,iEntry1,nullptr,iEntry);
            iEntry->canGetValue = false;
        }
        Print_Grammar_Output("<MulExp>");
    }

    //已经指向下一个
}
void Parser::UnaryExp(IEntry * iEntry,int & value,bool InOtherFunc) {
    int func_ident_line;
    if (WORD_TYPE == LPARENT || WORD_TYPE == INTCON){
        PrimaryExp(iEntry, value, InOtherFunc);
    }else if(WORD_TYPE == IDENFR){
        string ident = WORD_DISPLAY;
        Entry * temp = tableManager.cur;
        bool not_define_error = true;
        while(temp != nullptr){
            if(temp->entries->find(ident) != temp->entries->end()){
                not_define_error = false;
                break;
            }
            temp = temp->Father_Entry;
        }
        if (not_define_error){
            errorHandler.Insert_Error(NOT_DEFINE);
//        ouputFile << "1   "+to_string(errorHandler.error_line);
        }
        //lexer.ch = PEEK_A_LETTER;   为了去除所有的空白字符  用peek无法做到
        //之所以不用GET_A_WORD   是因为  这样会丢失ident    造成约束不一致
        //GET_A_WORD总是从ch当前属于的字符进行划分
        lexer.line_lastWord = lexer.line;
        func_ident_line = lexer.line;
        while(isspace(lexer.ch)){
            if(lexer.ch == '\n'){
                lexer.line++;
            }
            lexer.ch = (char )lexer.sourceFile.get();
        }
        if(lexer.ch == '('){
            Entry * func;
            bool err = false;
            Exp_type = 0;//函数调用时只可能返回值  且实参里真正有Exp是不可能的  所以没用
            int RParams_count = 0;
            //进入FuncRParams
            PRINT_WORD;//PRINT IDENT
            ident = WORD_DISPLAY;

            //函数未定义
            if( ! semantic.isEverDefined(ident,FUNC_INT, false)){//先认为是int函数   不做过多要求
                errorHandler.Insert_Error(NOT_DEFINE,func_ident_line);
                 err =true;
            }
            //无论是否报错  都继续  报了错就是正常的
            GET_A_WORD;//算上 （
            PRINT_WORD;//PRINT (
            GET_A_WORD;//指向FuncRParams  也可能是无参少了右括号
            //可以没有参数
            if(WORD_TYPE == RPARENT){//  也要去判断参数个数的问题
                /**
                 * 函数没有参数 不需要压栈
                 */
                func = nullptr;
                temp = tableManager.cur;
                while(temp != nullptr){
                    if (temp->entries->find(ident) != temp->entries->end()){
                        func = temp->entries->at(ident);
                        if (func->kind != FUNC_INT && func->kind != FUNC_VOID){
                            func = nullptr;
                        }
                        break;
                    }
                    temp = temp->Father_Entry;
                }
                if (func != nullptr && !(func->fParams->empty()) ){//函数定义时有参数 但是调用没参数  无脑个数问题
//                    errorHandler.error_line = func_ident_line;
                    errorHandler.Insert_Error(FUNC_RPARAMS_COUNT_ERROR,func_ident_line);
                }else{//没有参数的函数调用  src1函数定义头  src2函数参数IEntry  放在valueId
                    auto * params = new IEntry;
                    params->values_Id = new vector<int>;
                    if (func){
                        if (func->kind == FUNC_INT){//空参数
                            iEntry->canGetValue = false;
//                            iEntry->startAddress = IEntries.at(func->id)->return_IEntry->startAddress;//地址赋值 类似拷贝
                            intermediateCode.addICode(FuncCall,IEntries.at(func->id), params, iEntry);//FIXME:又返回值的函数  把值给到这个新建的iEntry  需要自己新建iEntry
                        }else{//此时iEntry没用
                            intermediateCode.addICode(FuncCall,IEntries.at(func->id), params, nullptr);
                        }
                    }
                }
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }else if(WORD_TYPE == LPARENT || WORD_TYPE == INTCON || WORD_TYPE == IDENFR || WORD_TYPE == PLUS || WORD_TYPE == MINU){
                //函数实参的first集
                /**
               * 函数有参数 需要压栈
               */
                func = nullptr;
                temp = tableManager.cur;
                while(temp != nullptr){
                    if (temp->entries->find(ident) != temp->entries->end()){
                        func = temp->entries->at(ident);
                        if (func->kind != FUNC_INT && func->kind != FUNC_VOID){
                            func = nullptr;
                        }
                        break;
                    }
                    temp = temp->Father_Entry;
                }
                func_name = ident;
                errorHandler.error_line = func_ident_line;//记录可能发生错误的行号
                auto * find_func = IEntries.at(func->id);//这个是函数定义头
                auto * params = new IEntry;//函数实参IEntry  下面的id才是真正实在参数
                params->values_Id = new vector<int>;
                auto *func_rParams = params->values_Id;//FIXME:values_address解决了有些值可能不是直接imm显示的

                FuncRParams(func_ident_line,func_rParams);
                if (WORD_TYPE != RPARENT){
                    //Error  缺少右括号）
                    errorHandler.Insert_Error(RPARENT_MISSING);
                }else {
                        //有参数的函数调用  src1函数定义头  src2函数参数IEntry  放在valueId
                        //TODO:明确传的是啥
                        //FParams->push_back(exp_iEntry->Id);//后端去拷贝assign
                    if (func){
                        if (func->kind == FUNC_INT){
                            intermediateCode.addICode(FuncCall,IEntries.at(func->id), params, iEntry);//FIXME:又返回值的函数  把值给到这个新建的iEntry  需要自己新建iEntry
                        }else{
                            intermediateCode.addICode(FuncCall,IEntries.at(func->id), params, nullptr);
                        }
                    }
                    PRINT_WORD;//PRINT )
                    GET_A_WORD;//point to next WORD
                }
            }else{
                    //函数无参 且缺失 ‘）’
                errorHandler.Insert_Error(RPARENT_MISSING);
            }
            /**
             * 调用函数时的func
             */
             if (func){
                 if(func->kind == FUNC_INT)
                     Exp_type = 0;
                 else if(func->kind == FUNC_VOID)//void  或者根本没有定义的函数 且已经报错未定义
                     Exp_type = -5;
             }else{
                 Exp_type = -4;
            }

        }else if(lexer.ch == '['){
            PrimaryExp(iEntry,value,InOtherFunc);
        }else{
            //Error  or  PrimaryExp 's LVal
            PrimaryExp(iEntry,value,InOtherFunc);//下放错误
        }
    }else{
        Exp_type = 0;
        int op;
        UnaryOp(op);
        UnaryExp(iEntry,value,InOtherFunc);
        if (op == 0){
            ;//无事
        }else if (op == 1){
            if(iEntry->canGetValue){
                iEntry->imm = -1*iEntry->imm;
            }else{
                intermediateCode.addICode(Sub,0,iEntry,iEntry);
            }
        }else if(op ==2 ){
            //FIXME:仅出现在条件表达式？TODO
            if (iEntry->canGetValue){
                iEntry->imm = (iEntry->imm == 1) ? 0 : 1;
            }else{
                intermediateCode.addICode(I_Not,iEntry, nullptr,iEntry);
            }
        }
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<UnaryExp>");
}

void Parser::PrimaryExp(IEntry * iEntry,int & value,bool InOtherFunc) {
    if (WORD_TYPE == LPARENT){
        PRINT_WORD;//PRINT (
        GET_A_WORD;
        Exp(iEntry, value, InOtherFunc);
        if (WORD_TYPE != RPARENT){
            //Error
            errorHandler.Insert_Error(RPARENT_MISSING);
        }else{
            PRINT_WORD;//PRINT )
            GET_A_WORD;
        }
    }else if(WORD_TYPE == INTCON){
        Exp_type = 0;
        Number(iEntry, value, InOtherFunc);
    }else{  //  指向ident LVal->GetInt
        auto *lVal = new IEntry;
        LVal(&lVal, value, InOtherFunc);//不在这一层报错？   放到下一层LVal = getint() | Exp  &是为了改变iEntry指向
        if (WORD_TYPE == ASSIGN){//FIXME:从Stmt-> LVal = ...来的  如果LVal为全局  或者非全局的赋值呢  标签？ lw？
            //TODO：使用全局变量 直接读值 编译时存好 0 或者具体值  这点在语法分析时没做好 需要MIPS code 进行进一步
            //TODO:   写入全局变量  如果写入值编译时确定 则直接更新编译时的IEntry|否则
            //赋值语句 需要找对真正的写地址！！！
            PRINT_WORD;//PRINT =
            GET_A_WORD;
            if (WORD_TYPE == GETINTTK){
                PRINT_WORD;//PRITN GETINT
                GET_A_WORD;
                PRINT_WORD;//PRINT (
                GET_A_WORD;
                if(WORD_TYPE == RPARENT){
                    PRINT_WORD;
                    GET_A_WORD;
                }else{
                    errorHandler.Insert_Error(RPARENT_MISSING);
                }
                intermediateCode.addICode(GetInt, nullptr, nullptr,lVal);
                lVal->canGetValue = false;
            } else {
                auto*exp = new IEntry;
                Exp(exp, value, InOtherFunc);//FIXME:直接将LVal的IEntry赋值到Exp中 表示Exp的最终结果就是LVal的内存所在区域的值！  如果不是直接求出值  那么
                intermediateCode.addICode(Assign,exp, nullptr,lVal);//一般的传递
                lVal->canGetValue = false;
            }
            //FIXME:这里是用来表示LVal是真正的左值  也就是语法树中不被算作Exp的  也就是本来LVal = getint（） | Exp这些是在Stmt中的  我的写法会让它在Stmt-》Exp中进行推导完成  无伤大雅  在此告诉自己
            isLValInStmt = true;
        }else{
            if (ISGLOBAL){
                iEntry->imm = IEntries.at(lVal->Id)->imm;
                iEntry->canGetValue = true;
            }else{
                intermediateCode.addICode(Assign,lVal, nullptr,iEntry);//一般的传递
            }
        }
        //不是赋值语句   需要将LVal找到的IEntry 传回上面 我现在不想用二级指针
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<PrimaryExp>");
}

//TODO：LVal说明是引用曾经定义过的变量（源程序）   需要二级指针进行重定向
//TODO：   值  地址    地址是需要拷贝原来数组的一切东西  只是type == 1  && offset is  valid      valueId 指向的IEntry就恒定为这个数组默认的值了   查询 如果canGet 就Get   不能  就lw sw 相对于这个IEntry的startAddress    || 写入值  置canGet为false  sw startAddress
void Parser::LVal(IEntry ** iEntry,int & value,bool inOtherFunc) { // 这里面中的容易错的地方 ident  line已经指向下一个字符前所在的行
    if (WORD_TYPE != IDENFR){
        //Error
    }
    PRINT_WORD;//PRINT IDENT
    string ident = WORD_DISPLAY;
    int error_const_line = lexer.line;
    int error_semicolon_missing_line = lexer.line_lastWord;
    Kind kind = VAR;
    int op  =0;
    Entry * temp = tableManager.cur;
    //无论是否报错  都继续  报了错就是正常的
    GET_A_WORD;
    IEntry * array_exps[3];
    int values[3];
    while(WORD_TYPE == LBRACK){
        op++;
        PRINT_WORD;//PRINT [
        GET_A_WORD;
        array_exps[op] = new IEntry;
        Exp(array_exps[op], values[op], inOtherFunc);
        if (WORD_TYPE != RBRACK){
            //Error  缺少有中括号]
            errorHandler.Insert_Error(RBRACK_MISSING);
        }else{
            PRINT_WORD;//PRINT ]
            GET_A_WORD;//POINT TO NEXT WORD
        }
    }
    /**
     * 数组在定义时长度肯定已经确定 同时值会存储在tableManager的符号表系统中
     * 所有由中间代码小组件的value（编译时确定）都会拷贝到符号表的Entry中---暂不清楚  反正IEntry是主要的  Entry存在id映射到IEntry
     */
    //引用LVal  搜寻之前定义时的类型，从而得出当前引用LVal时的类型   ------实际上LVal必须引用元素  即得到的一定是值---并不是  可以参数LVal是地址 一维或者二维都行
    temp = tableManager.cur;
    Entry *find = nullptr;
    Exp_type = -3;
    while (temp!= nullptr){
        if(temp->entries->find(ident) != temp->entries->end()){
            Kind kind1 = (find = (temp->entries->at(ident)))->kind;
            if(kind1 == ARRAY_1_VAR || kind1 == ARRAY_1_CONST){
                Exp_type = 1-op;
            }else if(kind1 == ARRAY_2_VAR || kind1 == ARRAY_2_CONST){
                Exp_type = 2-op;
            }else if (kind1 == VAR || kind1 == CONST){
                Exp_type = 0-op;
            }else{
                //实际上是函数赋值   是未定义的
            }
            break;
        }
        temp = temp->Father_Entry;
    }
    int dim1_length;
    int index;
    IEntry * index_entry ;
    if (!find){
        goto End;
    }
     dim1_length= IEntries.at(find->id)->dim1_length;
    index = 0;
  index_entry = new IEntry;
    if (Exp_type == -3){
        //没找到 未定义
//        errorHandler.Insert_Error(NOT_DEFINE);
    }else if(Exp_type < 0){
        //超出维数的引用   不出现
    }else if(Exp_type == 0){
                //address for writing
        //
        IEntry *_val;
        if (IEntries.at(find->id)->type == 0){
            _val = IEntries.at(find->id);
        }else{  //这里之前是为了函数的      现在可以其实可以不用了
            _val = IEntries.at(IEntries.at(find->id)->values_Id->at(0));
        }
        if (op == 2){
            if (WORD_TYPE == Type::ASSIGN){
                (*iEntry)->type =2;
            }else{
                (*iEntry)->type =0;
            }
            if (ISGLOBAL){
                (*iEntry)->imm = IEntries.at(_val->values_Id->at(array_exps[1]->imm*dim1_length + array_exps[2]->imm))->imm;
                (*iEntry)->canGetValue = true;
            }else{
                if(array_exps[2]->canGetValue && array_exps[1]->canGetValue){
                    index = array_exps[1]->imm*dim1_length + array_exps[2]->imm;
                    intermediateCode.addICode(GetArrayElement,index,_val,*iEntry);
                }else{
                    if (array_exps[1]->canGetValue){
                        int t = array_exps[1]->imm*dim1_length;
                        intermediateCode.addICode(IntermediateCodeType::Add,t,array_exps[2],index_entry);
                        intermediateCode.addICode(GetArrayElement,index_entry,_val,*iEntry);//此时iEntry在getArrayElement中会储存对应的address 方便之后的lw
                    }
                    else if(array_exps[2]->canGetValue){
                        auto *t = new IEntry;
                        intermediateCode.addICode(IntermediateCodeType::Mult,dim1_length,array_exps[1],t);
                        intermediateCode.addICode(IntermediateCodeType::Add,array_exps[2],t,index_entry);
                        intermediateCode.addICode(GetArrayElement,index_entry,_val,*iEntry);
                    }else{
                        auto *t = new IEntry;
                        intermediateCode.addICode(IntermediateCodeType::Mult,dim1_length,array_exps[1],t);
                        intermediateCode.addICode(IntermediateCodeType::Add,array_exps[2],t,index_entry);
                        intermediateCode.addICode(GetArrayElement,index_entry,_val,*iEntry);
                    }
                }//FIXME:数组定义时的IEntry （src2）   偏移index（不乘4）index_entry-》能get就get 不能就lw address
            }
        }else if(op == 1){//FIXME:可能you缺漏
            if (WORD_TYPE == Type::ASSIGN){
                (*iEntry)->type =2;
            }else{
                (*iEntry)->type =0;
            }
            if (ISGLOBAL){
                (*iEntry)->imm = IEntries.at(_val->values_Id->at(array_exps[1]->imm))->imm;
                (*iEntry)->canGetValue = true;
            }
            else{
                intermediateCode.addICode(GetArrayElement,array_exps[1],_val,*iEntry);
            }
        }else if(op == 0){//TODO:统一都在values_Id
            if (WORD_TYPE == Type::ASSIGN){//决定iEntry的内容是地址还是值  进行assign
                (*iEntry)->type =2;
            }else{
                (*iEntry)->type =0;
            }
            if (ISGLOBAL){
                (*iEntry)->imm = IEntries.at(_val->values_Id->at(0))->imm;
                (*iEntry)->canGetValue = true;
            }else{
                intermediateCode.addICode(GetArrayElement,0,_val,*iEntry);
            }
        }
    }else if(Exp_type == 1){ //find就是对应的曾经定义过的Entry   iEntry标识直接传递地址  非值的地址变量  只出现在函数形参中
        //一维地址
        /**
         * 只可能是传给别人做地址
         * 重新生成一个IEntry   用来表示内存位置
         * TODO:先不考虑烦人的array找不到对应的值的情况---需要进一步生成index的中间代码
         */
        (*iEntry)->type =1;
        if (op == 0){
            //没有其他需要加的offset
            intermediateCode.addICode(GetAddress, nullptr,IEntries.at(find->id),*iEntry);
        }else{
            //有一个数字需要加  exps[1]*dim1
            if (array_exps[1]->canGetValue){
                index = array_exps[1]->imm*dim1_length;
                intermediateCode.addICode(GetAddress, index,IEntries.at(find->id),*iEntry);
            }
            else{
                intermediateCode.addICode(Mult,dim1_length,array_exps[1],index_entry);
                intermediateCode.addICode(GetAddress, index_entry,IEntries.at(find->id),*iEntry);
            }
        };
    }else{ // 二级地址要小心 TODO: 形参的ConstExp是有用的  const不能作为数组参数！！！ 所以只用伪造valuesID
        //2维地址  伪造iEntry数组 认为是type = 1!!!!  取元素才不会取到你、空
        (*iEntry)->type =1;
        intermediateCode.addICode(GetAddress, nullptr,IEntries.at(find->id),*iEntry);
    }

    End:;//find是我为了中间代码新加的




    /**
     *     此时array_exps[]为索引Exp的临时变量IEntry
     *     values[]为值（如果可以算出来）
     *     注意value并没有存到IEntry中  因为IEntry的存在就是为了生成中间代码的四元式时不得不生产出来的临时变量来代表中间结果的变量 也有可能是编译时无法直接求出value的  所以需要中间临时变量进行代替  这一部分是会在生成MIPS时通过IEntry-》内存位置从而消除
     */
     /**
      * 二维地址
      */





/**
 * index_entry   index
 *
 *
 *
 * 要理解IEntry---在中间代码的层次 -不要太底层了 ---要做的是把一些有用的信息放进这里面---IEntry是要编号的   ICode的作用
 */

    //错误的先后顺序  先检查方括号匹配 ---判断具体类型和未定义 再判断是否是左值定义常量 isLeft参数为true表示检测左值的
    if(WORD_TYPE == ASSIGN ){
        Entry *t = tableManager.cur;
        Entry * _const = nullptr;
        while(t != nullptr){
            if(t->entries->find(ident) != t->entries->end()){
                _const = t->entries->at(ident);
                if (_const->kind == CONST || _const->kind == ARRAY_1_CONST || _const->kind == ARRAY_2_CONST){
                    errorHandler.error_line = error_const_line;
                    errorHandler.Insert_Error(CONST_LEFT);
                }
                break;
            }
            t = t->Father_Entry;
        }
    }else if (WORD_TYPE == IDENFR || WORD_TYPE == RBRACE
    || WORD_TYPE == CONSTTK || WORD_TYPE == RETURNTK
    || WORD_TYPE == IFTK || WORD_TYPE == ELSETK
    || WORD_TYPE == CONTINUETK || WORD_TYPE == BREAKTK
    ||WORD_TYPE == PRINTFTK|| WORD_TYPE == FORTK
    || WORD_TYPE == VOIDTK  || WORD_TYPE == LBRACE
    ||WORD_TYPE == INTTK){
        //引用值结尾 却漏了分号  不在这里报错  但是要修改last_line
    lexer.line_lastWord = error_semicolon_missing_line;
    }
    Print_Grammar_Output("<LVal>");
}

void Parser::Number( IEntry *iEntry,int & value,bool InOtherFunc) {
    if (WORD_TYPE != INTCON){
        //Error
    }
    //TODO:  需要进一步明确value的值是否需要放在IEntry中？   这是后面的常数优化
//    if (ISGLOBAL){
        iEntry->canGetValue = true;
        iEntry->imm =  lexer.token.number;
//    }else{
//        auto v = new IEntry;
//        v->canGetValue = true;
//        v->imm = lexer.token.number;
////        intermediateCode.addICode(Assign,v,nullptr,iEntry);
//    }
    PRINT_WORD;
    Print_Grammar_Output("<Number>");
    GET_A_WORD;//NOT PW
}

void Parser::Exp(IEntry *iEntry,int & value,bool InOtherFunc) {//TODO:认为iEntry代表Exp的值 或者存值的地址
//    auto* _addExp = new IEntry;
//FIXME：对比ConstExp->AddExp  这里不用再iEntry = new Entry
    AddExp(iEntry,value,InOtherFunc);
//    iEntry->values->push_back(value);
//    iEntry->values_Id->push_back(_addExp->Id);//FIXME:只记录那个值的存储位置
    if (!isLValInStmt)
        Print_Grammar_Output("<Exp>");
}

void Parser::MainFuncDef() {
    isInOtherFunc = false;
    funcLabel = "main";
    if (WORD_TYPE != MAINTK){
        //ERROR
    }
    PRINT_WORD;//PRINT MAIN
    GET_A_WORD;//POINT TO (
    PRINT_WORD;//PRINT (
    GET_A_WORD;//point to )
    if (WORD_TYPE != RPARENT){//point to {
        errorHandler.Insert_Error(RPARENT_MISSING);
    }else{
        PRINT_WORD;//print )
        GET_A_WORD;//point to {
    }
    string ident = "main";
    INFO_ENTRY = semantic.fillInfoEntry(ident,FUNC_INT);
    semantic.recordEntries(INFO_ENTRY);
    tableManager.downTable(ident);
    Block();
    tableManager.upTable();
    Print_Grammar_Output("<MainFuncDef>");
}

//TODO:函数实参一一赋值给已经占住位置的形参FParams是   当初函数头的形参的values_Id
void Parser::FuncRParams(int func_ident_line,vector<int>  *RParams) {
    bool already_error_func_type = false;
    bool already_error_func_count = false;
    //依次取出Exp的类型 然后判断函数符号表中的vector   类型只需要看 0 1 2 普通变量 一维数组 二维数组
    //可是应该怎么比较好的完成这个Exp类型的判断呢
    Entry * func = nullptr;
    Entry * temp = tableManager.cur;
    while(temp  != nullptr){
        if(temp->entries->find(func_name) != temp->entries->end()){
            func = temp->entries->at(func_name);
            if(func->kind != FUNC_INT && func-> kind != FUNC_VOID)
                    func = nullptr;
            break;
        }
        temp = temp->Father_Entry;
    }
    int cnt = 0;
    int x;
    vector<Entry *> FArguments;
    if (func != nullptr){
        FArguments = *(func->fParams);
    }
    if (FArguments.empty()&&func != nullptr){ //定义函数时空参数  但是调用存在参数  无脑个数问题
//        errorHandler.error_line = func_ident_line;
        errorHandler.Insert_Error(FUNC_RPARAMS_COUNT_ERROR,func_ident_line);
        already_error_func_count = true;
    }
    errorHandler.error_type = NORMAL;//先清除之前的  只在第一个实参前这样
    auto * exp_iEntry = new IEntry;
    int value;
    Exp(exp_iEntry, value, true);//里面进行实参的未定义报错
    if(func != nullptr&& !already_error_func_count&&!already_error_func_type){//如果函数是未定义的函数 也就不需要检查实参的两种类型错误
        x = Kind2Exp_type(FArguments.at(cnt++)->kind);//函数定义时的形参
        if(Exp_type != x&&Exp_type !=  -4){
//            errorHandler.error_line = func_ident_line;
            errorHandler.Insert_Error(FUNC_RPARAMS_TYPE_ERROR,func_ident_line);
            already_error_func_type = true;
        }
    }
    /**
     * 将实参exp_iEntry放入
     */
     //TODO:在中间代码阶段就先不要多多去管闲事去想着分类讨论值  把IEntry看做抽象 能够加快效率  新建的exp_IEntry
    RParams->push_back(exp_iEntry->Id);//后端去拷贝assign
//    intermediateCode.addICode(Assign, exp_iEntry,nullptr,IEntries.at(RParams->at(cnt-1)));

    while(WORD_TYPE == COMMA){
        PRINT_WORD;
        GET_A_WORD;
        exp_iEntry = new IEntry;
        Exp(exp_iEntry,value,isInOtherFunc);
        if(func != nullptr && !already_error_func_count&&!already_error_func_type&& errorHandler.error_type != NOT_DEFINE){
            if(cnt >= FArguments.size()){//实际调用参数多
//                errorHandler.error_line = func_ident_line;
                errorHandler.Insert_Error(FUNC_RPARAMS_COUNT_ERROR,func_ident_line);//不会出现一行两个错误 既有
                already_error_func_count = true;
                break;
            }
            x = Kind2Exp_type(FArguments.at(cnt++)->kind);//函数定义时的形参
            if(Exp_type != x &&Exp_type !=-4&& !already_error_func_type&&!already_error_func_count){//避免报错多个类型不匹配？
//                errorHandler.error_line = func_ident_line;
                errorHandler.Insert_Error(FUNC_RPARAMS_TYPE_ERROR,func_ident_line);
                already_error_func_type = true;
            }
        }
        RParams->push_back(exp_iEntry->Id);
    }
    if(cnt < FArguments.size() &&!already_error_func_type&&!already_error_func_count&& func != nullptr){//实际调用参数少
        errorHandler.Insert_Error(FUNC_RPARAMS_COUNT_ERROR,func_ident_line);//不会出现一行两个错误 既有
    }
    Print_Grammar_Output("<FuncRParams>");
}

void Parser::FuncDef(Kind func_type) {

    //FuncDef 从名字<ident>开始
    bool error = false;
    string ident =WORD_DISPLAY;
    funcLabel = ident;
    auto* func = new IEntry ;//FIXME:表示函数定义时的相关信息头： 形参values_Id 有没有返回值
    funcInitAddress = func->startAddress;
    isInOtherFunc = true;
    //定义：先在这一层找   找到就报错REDEFINE  没找到就填表
    if(tableManager.isRedefine(ident)){
        errorHandler.Insert_Error(REDEFINE);
        error = true;
        ident = "main";
    }
    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    PRINT_WORD;//PRINT (
    GET_A_WORD;
    if(WORD_TYPE == RPARENT || WORD_TYPE == LBRACE){//可能的情况func({  空参数漏了）
        //no FUNCFParams
        INFO_ENTRY = semantic.fillInfoEntry(ident,func_type);//空参数
        func->has_return = func_type == FUNC_INT;
        func->original_Name = ident;
        if (func->has_return ){
            func->return_IEntry = new IEntry;
        }else{
            //占位
            ;
            func->return_IEntry = new IEntry;
        }
        func->values_Id = new vector<int>;
        INFO_ENTRY->fParams = new vector<Entry *>;
        INFO_ENTRY->id = func->Id;//后续被引用时才知道是这个IEntry的函数头
        semantic.recordEntries(INFO_ENTRY);
        if (WORD_TYPE == RPARENT){
            intermediateCode.addICode(IntermediateCodeType::FuncDef,func, nullptr, nullptr);
            PRINT_WORD;
            GET_A_WORD;//point to {
        }else{
            errorHandler.Insert_Error(RPARENT_MISSING);
        }
        tableManager.downTable(ident);


        Block();
        tableManager.upTable();
    }else{
        INFO_ENTRY = semantic.fillInfoEntry(ident,func_type);
        func->has_return = func_type == FUNC_INT;
        func->original_Name = ident;
        func->values_Id = new vector<int>;
        INFO_ENTRY->id = func->Id;
        semantic.recordEntries(INFO_ENTRY);
        tableManager.downTable(ident);//对于有参函数   参数和block都属于这个函数名的对应符号表level
        vector<Entry *> entries;//涉及到堆上内存引用？
        if(WORD_TYPE == INTTK){
            FuncFParams(entries);
            //指向）
            //FIXME:将IEntry的对应形参存储完善!!!!
            for (auto entry :entries) {
                func->values_Id->push_back(entry->id);
            }
            func->original_Name = ident;
            if(WORD_TYPE != RPARENT){
                //ERROR
                errorHandler.Insert_Error(RPARENT_MISSING);
            }else{
                if (func->has_return ){
                    func->return_IEntry = new IEntry;
                }else{
//占位
                    ;
                    func->return_IEntry = new IEntry;
                }
                intermediateCode.addICode(IntermediateCodeType::FuncDef,func, nullptr, nullptr);
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }

            tableManager.cur->fParams = new vector(entries);//还真是   如果直接&entries  会丢失
//            semantic.recordEntries(entries);//参数也重新放进这个函数中 当做局部变量
//            funcInitAddress = func->startAddress;

            Block();
            tableManager.upTable();
        }else{
            //Error  形参只能int打头    如果改成增加double！！！
            //TODO:完善
        }
    }
    tableManager.cur->entries->erase("main");//如果是重定义的函数 需要抹掉它
    isInOtherFunc = false;//确保func的startAddress是大数
    //TODO:FuncCall中间代码 FIXME：完成FuncCALL   src1为函数头


    Print_Grammar_Output("<FuncDef>");
}

void Parser::FuncType() {
    Print_Grammar_Output("<FuncType>");
}

//TODO:函数形参的处理
void Parser::FuncFParams(vector<Entry *> &arguments) {
    FuncFParam(arguments);
    while(WORD_TYPE == COMMA){
        PRINT_WORD;//PRINT ,
        GET_A_WORD;
        FuncFParam(arguments);
    }
    Print_Grammar_Output("<FuncFParams>");
}

void Parser::FuncFParam(vector<Entry *> & arguments) {
    if(WORD_TYPE != INTTK){
        //error
    }
    PRINT_WORD;//PRINT INT
    GET_A_WORD;
    int op =0;
    string ident = WORD_DISPLAY;
    //定义：先在这一层找   找到就报错REDEFINE  没找到就填表
    if(tableManager.isRedefine(ident) ){
        errorHandler.Insert_Error(REDEFINE);//重定义不加入符号表形参???
//        op =-1;  也要加入？   只是报错
    }

    IEntry*exp_iEntrys[3];
    int values[3];
    IEntry *rParam;
    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    if(WORD_TYPE != LBRACK){
        //认为就是普通变量  这里不会有error
        if (op != -1)
            op = 0;
    }else{
        PRINT_WORD;//PRINT [
        if (op !=-1)
            op++;
        GET_A_WORD;
        if(WORD_TYPE != RBRACK){
            //ERROR  缺少右中括号
//            op = -1;
            errorHandler.Insert_Error(RBRACK_MISSING);
            goto END;
        }
        PRINT_WORD;//PRINT ]   一维数组变量
        GET_A_WORD;
        if(WORD_TYPE != LBRACK){
            //认为就是一维数组变量  这里不会有error
        }else{
            //二维数组变量
            if(op!=-1)
                op++;
            PRINT_WORD;//PRINT [
            GET_A_WORD;
            if(WORD_TYPE == RBRACK){
                //ERROR  二维数组 最后一维需要常数  也只有exp_iEntrys[2] 有可能是值
            }
            exp_iEntrys[op] = new IEntry;
            ConstExp(exp_iEntrys[op],values[2],isInOtherFunc);
            //FIXME:无关紧要  实参拷贝到形参时默认是正确的通过实参就可以断定type
            if(WORD_TYPE != RBRACK){
                //ERROR  缺少右中括号
//                op = -1;
                errorHandler.Insert_Error(RBRACK_MISSING);
                goto END;
            }
            PRINT_WORD;//PRINT ]
            GET_A_WORD;//指向下一个
        }
    }
    END:
    if(op == -1){
        //正指向本来应该是]的位置  其实已经是下一个
        ;
    }else{
        Kind kind;
        if (op == 0){
            kind = VAR;
            rParam = new IEntry;
            rParam->type = 0;
            rParam->values_Id = new vector<int>;
            rParam->original_Name = ident;
            IEntry*v;
            rParam->values_Id->push_back((v = new IEntry)->Id);//new IEntry存放值
            v->type = 0;//TODO：6表示需要取的时候要用 sp  且不是lw地址
            v->imm = (int)arguments.size()+1;
            v->startAddress = rParam->startAddress;
            //函数的值需要sp来保存 imm来记录是第几个参数
        }else if(op == 1){
            kind = ARRAY_1_VAR;//关于kind 类型匹配 需要放宽 const常量  int变量？说明：常量数组不允许加到参数中  所以都是VAR类型即可
            rParam = new IEntry();
            rParam->type = 1;
            rParam->values_Id = new vector<int>;
            rParam->original_Name = ident;
            IEntry * v;
            rParam->values_Id->push_back((v = new IEntry)->Id);
//            v->offset_IEntry = new IEntry;//不一定需要？
            v->type = 1;//lw 地址
            v->imm = (int)arguments.size()+1;
            v->startAddress = rParam->startAddress;
//            rParam->offset_IEntry = new IEntry;
            //new IEntry存放地址
        }else{
            kind = ARRAY_2_VAR;
            rParam = new IEntry();
            rParam->dim1_length = exp_iEntrys[2]->imm;
            rParam->values_Id = new vector<int>;
            rParam->original_Name = ident;
            IEntry * v;
            rParam->values_Id->push_back((v = new IEntry)->Id);
//            v->offset_IEntry = new IEntry;
            v->type =1;
            v->imm = (int)arguments.size()+1;
//            rParam->offset_IEntry = new IEntry;
            rParam->type = 1;
            v->startAddress = rParam->startAddress;
        }

       //TODo：函数形参站住位置  指向的是定义本身不是值本身 类似于定义变量！！！
        INFO_ENTRY = semantic.fillInfoEntry(ident,kind);
        INFO_ENTRY->id = rParam->Id;
        arguments.push_back(INFO_ENTRY);
        semantic.recordEntries(INFO_ENTRY);
    }
    Print_Grammar_Output("<FuncFParam>");
}

void Parser::Block() {

    if (WORD_TYPE != LBRACE){
        //ERROR
    }
    PRINT_WORD;//PRINT {
    GET_A_WORD;
    if(WORD_TYPE == RBRACE){
        //空语句块  单独分号是放在stmt解析的  所以一个空语句块是存在的
        if (tableManager.cur->kind == FUNC_INT && tableManager.cur->return_error){
            errorHandler.Insert_Error(INT_FUNC_NO_RETURN);
        }else if (tableManager.cur->kind == FUNC_VOID){
            intermediateCode.addICode(Return, nullptr, nullptr, nullptr);
        }
        PRINT_WORD; // print }
        GET_A_WORD;
    }else{
        while(WORD_TYPE != RBRACE){
            BlockItem();//这一级不处理错误
            //只看最后一句有没有return  在这里进行一个peek
            //如果这里缺少}  会出现问题   但是不可能出现
        }
        if(WORD_TYPE != RBRACE){
            //ERROR   不可能出现的
            errorHandler.Insert_Error(RBRACE_MISSING);
        }else{
            if (tableManager.cur->kind == FUNC_INT && tableManager.cur->return_error){
                errorHandler.Insert_Error(INT_FUNC_NO_RETURN);
            }else if (tableManager.cur->kind == FUNC_VOID){
                intermediateCode.addICode(Return, nullptr, nullptr, nullptr);
            }
            PRINT_WORD; // print }
            GET_A_WORD;
        }
    }
    Print_Grammar_Output("<Block>");
}

void Parser::BlockItem() {
    if (WORD_TYPE == CONSTTK || WORD_TYPE == INTTK){
        /**
         * DECL   常量从const   变量从ident
         */
        if(WORD_TYPE == INTTK){
            PRINT_WORD;//PRINT INT
            GET_A_WORD;//POINT TO IDENT FOR VAR
        }
        Decl();
    }else{
        Stmt();
    }
//不用输出语法成分
}

//TODO:有关下放Exp的处理
void Parser::Stmt() {
    string ident;
    Entry * temp;
    Entry * func;
    int printf_count = 0;
    bool loop_error = true;
    IEntry * exp;
    int exp_value;

    IEntry * strings;
    IEntry * p_params;
//    int printf_format = 0;
    switch (WORD_TYPE) {
        case PRINTFTK:
//            errorHandler.error_type = NORMAL;
            PRINT_WORD;//PRINT PRINTF
            printf_line = lexer.line;
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            strings = new IEntry;
            strings->strings_iEntry_id = new vector<int>;//TODo  :在内部函数初始化 会出错？
            FormatString(strings);//FIXME:拆成若干个string   %d   [normal string]
            p_params = new IEntry;
            p_params->values_Id = new vector<int>;
            while(WORD_TYPE == COMMA){
                printf_count++;
                PRINT_WORD;//PRINT ,
                GET_A_WORD;
                exp = new IEntry;///
                Exp(exp,exp_value,isInOtherFunc);
                p_params->values_Id->push_back(exp->Id);
            }
            if(lexer.printf_format_count != printf_count){//出现了不合法字符 就没必要再继续判断了
                errorHandler.error_line = printf_line;
                errorHandler.Insert_Error(PRINTF_NOT_EQUAL_COUNT);
            }
            if(WORD_TYPE != RPARENT){
                //缺少右圆括号
                errorHandler.Insert_Error(RPARENT_MISSING);
            }else{
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }
            if(WORD_TYPE != SEMICN){
                //缺少分号
                errorHandler.Insert_Error(SEMICOLON_MISSING);
            }else{
                intermediateCode.addICode(Printf,strings,p_params, nullptr);
//                intermediateCode.strings->push_back()
                PRINT_WORD;//PRINT ;
                GET_A_WORD;
            }
            //TODO:printf
            break;
        case RETURNTK:
            PRINT_WORD;//PRINT RETURN
            GET_A_WORD;
            if(WORD_TYPE == SEMICN){
                intermediateCode.addICode(IntermediateCodeType::Return, nullptr, nullptr, nullptr);//void函数返回
                PRINT_WORD;//PRINT ;
                GET_A_WORD;//return；空
            }else{//如果是另起一行 并不算return后缺少分号
                if(WORD_TYPE != RBRACE){ //如果return 后面真的是表达式
                    temp = tableManager.cur;
                    while(temp != nullptr){//无参函数返回值的错误
                        if (temp->kind == FUNC_VOID){
                            errorHandler.Insert_Error(VOID_FUNC_HAS_RETURN);
                            break;
                        }
                        temp = temp->Father_Entry;
                    }
                    temp = tableManager.cur;
                    func = nullptr;
                    while(temp != nullptr){//无参函数返回值的错误
                        if (temp->kind == FUNC_VOID || temp->kind ==FUNC_INT){
                           func = temp;//肯定不可能是空
                            break;
                        }
                        temp = temp->Father_Entry;
                    }
                    exp = new IEntry;
                    Exp(exp,exp_value,isInOtherFunc);//错误下放
                    if (WORD_TYPE != SEMICN){
                        //ERROR
                        //Print_Grammar_Output("ERROR  :");
                        errorHandler.Insert_Error(SEMICOLON_MISSING);
                    }else{
                        if (func){
                            if(func->kind == FUNC_INT){
                                auto * func_iEntry = IEntries.at(func->id);
                                //func_iEntry->return_IEntry已经生成
                                intermediateCode.addICode(IntermediateCodeType::Return,exp, nullptr, func_iEntry->return_IEntry);
                            }else{
                                intermediateCode.addICode(IntermediateCodeType::Return, nullptr, nullptr, nullptr);//void函数返回
                            }
                        }
                        PRINT_WORD;//print ;
                        GET_A_WORD;
                    }
                    //有参函数标记末尾出现return+值的语句  之后结束时in FuncDef不用输出错误
                    if (WORD_TYPE == RBRACE && tableManager.cur->kind == FUNC_INT){
                        tableManager.cur->return_error = false;
                    }
                }else{//return }
                    errorHandler.Insert_Error(SEMICOLON_MISSING);
                }
                }
            //TODO:RETURN
            break;
        case BREAKTK:
        case CONTINUETK:
            Type t;
            t = WORD_TYPE;
            PRINT_WORD;//PRINT CONTINUE
            GET_A_WORD;
            temp = tableManager.cur;
            while(temp != nullptr){
                if(temp->ident == "for"){
                    loop_error = false;
                    break;
                }
                temp = temp->Father_Entry;
            }

            if(loop_error){
                errorHandler.Insert_Error(NOT_LOOP_USING_BC);
            }else{
                //legal continue or break then find the appropriate for entry => iEntry => return_IEntry
                if (temp->kind == Kind::FOR){
                    if (t == CONTINUETK){
                        IEntry *continue_label;
                        continue_label = new IEntry("continue");
                        intermediateCode.addICode(Insert_Label,continue_label, nullptr, nullptr);
                        intermediateCode.addICode(Jump_Label,IEntries.at(temp->id)->offset_IEntry, nullptr, nullptr);
                    }else if(t == BREAKTK){
                        IEntry *break_label;
                        break_label = new IEntry("break");
                        intermediateCode.addICode(Insert_Label,break_label, nullptr, nullptr);
                        intermediateCode.addICode(Jump_Label,IEntries.at(temp->id)->return_IEntry, nullptr, nullptr);
                    }
                }
            }
            if (WORD_TYPE != SEMICN){
                //ERROR
                errorHandler.Insert_Error(SEMICOLON_MISSING);
            }else{
                PRINT_WORD;//PRINT ;
                GET_A_WORD;
            }
            break;
        case FORTK: {
            IEntry *startFor, *endFor, *condFor, *cond,*mainFor,*opFor;
            startFor = new IEntry("For_start");
            endFor = new IEntry("For_end");
            mainFor = new IEntry("For_main");
            opFor = new IEntry("For_op");
            condFor = new IEntry("For_cond");
            //condFor在后面定义了
            intermediateCode.addICode(Insert_Label, startFor, nullptr, nullptr);
            PRINT_WORD;//PRINT FOR
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            if (WORD_TYPE == SEMICN) {
                PRINT_WORD;//PRITN ;
                GET_A_WORD;
            } else {
                //For_stmt_1
                ForStmt();
                if (WORD_TYPE != SEMICN) {
                    //ERROR
                    errorHandler.Insert_Error(SEMICOLON_MISSING);//虽然for语句强调了不会出现这个错误
                } else {
                    PRINT_WORD;//PRINT ;
                    GET_A_WORD;
                }
            }
//###########################condFor############################
            intermediateCode.addICode(Insert_Label, condFor, nullptr, nullptr);
            if (WORD_TYPE == SEMICN) {
                PRINT_WORD;//PRITN ;
                GET_A_WORD;
            } else {
                cond = new IEntry;
                //cond 每次循环要重新算一遍
                Cond(cond);
                intermediateCode.addICode(Beqz, cond, endFor, nullptr);
                if (WORD_TYPE != SEMICN) {
                    //ERROR
                    errorHandler.Insert_Error(SEMICOLON_MISSING);
                } else {
                    PRINT_WORD;//PRINT ;
                    GET_A_WORD;
                }
            }
            intermediateCode.addICode(Jump_Label, mainFor, nullptr, nullptr);
//###########################opFor############################
            intermediateCode.addICode(Insert_Label, opFor, nullptr, nullptr);
            if (WORD_TYPE == RPARENT) {
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            } else {
                //For_stmt_2  包含在循环体中
//                opFor = new IEntry("For_op");
                ForStmt();
                if (WORD_TYPE != RPARENT) {
                    //error  这其实不可能会有
                    errorHandler.Insert_Error(RPARENT_MISSING);
                } else {
                    PRINT_WORD;//PRINT )
                    GET_A_WORD;
                }
            }
//##############################opFor-End############################
            intermediateCode.addICode(Jump_Label, condFor, nullptr, nullptr);//跳转到for_cond
            ident = "for";
            semantic.recordEntries(semantic.fillInfoEntry(ident, FOR));
            //向下一层符号表
            tableManager.downTable(ident);
            IEntry*_forIEntry;
            _forIEntry = new IEntry;
            _forIEntry->return_IEntry = endFor;//return_IEntry存储结束的Label
            _forIEntry->offset_IEntry = opFor;
            tableManager.cur->id = _forIEntry->Id;
            tableManager.cur->loop_count++;
//            mainFor = new IEntry("For_main");
            intermediateCode.addICode(Insert_Label,mainFor, nullptr, nullptr);
            Stmt();
            intermediateCode.addICode(Jump_Label, opFor, nullptr, nullptr);//跳转到for_stmt_2
            tableManager.cur->loop_count--;
            tableManager.upTable();
            tableManager.cur->entries->erase(ident);//for 结束了就不需要再留位置 给之后的for让路

            //向上一层符号表
            intermediateCode.addICode(Insert_Label, endFor, nullptr, nullptr);
            break;
        }
        case IFTK:{
            ident = "if";//不用担心起冲突 不会重名关键字  同时多个if是栈结构
            PRINT_WORD;//PRINT IF
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            IEntry *cond;
            IEntry *label_else,*label_end,*label_if;
            label_if = new IEntry("if");
            cond = new IEntry;
            Cond(cond); //cond 值0或者1  也可能是存储0和1 的地址 需要lw
            if (WORD_TYPE != RPARENT){
                //ERROR
                errorHandler.Insert_Error(RPARENT_MISSING);
            }else{
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }
            semantic.recordEntries(semantic.fillInfoEntry(ident,IF));
            //向下一层符号表
            tableManager.downTable(ident);
            label_end = new IEntry("end");
            label_else = new IEntry("else");
//            if (cond->canGetValue){
//                if(cond->imm == 0){
////                    goto Stmt_else;
//intermediateCode.addICode(Jump_Label,label_else, nullptr, nullptr);
//                }else{
//                    goto Stmt_if;
////intermediateCode.addICode(Jump_Label,label_if, nullptr, nullptr);
//                }
//            }

            intermediateCode.addICode(Beqz,cond, label_else, nullptr);//不能直接跳到end
            Stmt_if:
            intermediateCode.addICode(Insert_Label,label_if, nullptr, nullptr);
            Stmt();

            tableManager.upTable();
            tableManager.cur->entries->erase(ident);//for 结束了就不需要再留位置 给之后的for让路
            if (WORD_TYPE == ELSETK){
                PRINT_WORD;//PRINT ELSE
                GET_A_WORD;
                intermediateCode.addICode(Jump_Label,label_end, nullptr, nullptr);
                semantic.recordEntries(semantic.fillInfoEntry(ident,ELSE));
                //向下一层符号表
                tableManager.downTable(ident);
                //自动跳转
//                Stmt_else:
                intermediateCode.addICode(Insert_Label,label_else, nullptr, nullptr);
                Stmt();
                tableManager.upTable();
                tableManager.cur->entries->erase(ident);//for 结束了就不需要再留位置
            }else{
                intermediateCode.addICode(Insert_Label,label_else, nullptr, nullptr);//没有else时 也要让if在不满足条件时跳出来
            }
            intermediateCode.addICode(Insert_Label,label_end, nullptr, nullptr);
            break;
        }
        case LBRACE:
            //from { start   not print
            ident = "###";
            semantic.recordEntries(semantic.fillInfoEntry(ident,BLOCK));
            //向下一层符号表
            tableManager.downTable(ident);
            Block();
            tableManager.upTable();
            tableManager.cur->entries->erase(ident);
            break;
        case SEMICN:
            PRINT_WORD;//PRINT ;
            GET_A_WORD;
            break;
            
        default:
            // Exp  LVal    只有LVal有赋值符号于Stmt中
            //LVal
            //Exp
            //TODO:Exp
            auto *exp_iEntry = new IEntry;
            int value;
            Exp(exp_iEntry,value,isInOtherFunc);
            if(WORD_TYPE != SEMICN){
                //ERROR
                //缺少分号
                errorHandler.Insert_Error(SEMICOLON_MISSING);
            }else{
                PRINT_WORD;
                GET_A_WORD;
            }
            if (isLValInStmt){
                isLValInStmt = false;
            }//重置
            break;
    }
    Print_Grammar_Output("<Stmt>");
}

void  Parser::FormatString(IEntry * _strings) {
    IEntry* p ;
    int pos = 0;
//    strings->strings_iEntry_id = new vector<int>;
    string* whole = lexer.token.symbol;
    int i = 0;
    if (WORD_TYPE == ILLEGAL){
        //ERROR
        //可能还需要注意 格式字符串是否合法？
        errorHandler.Insert_Error(ILLEGAL_STRING);
    }else{
            //解析string    %d%d...%d....%d
        for (i = 0;i < whole->size();i++){
            if (whole->at(i) == '%'){
                if (i!= 0 && i-pos>=1){
                    //[pos,i)
                    p = new IEntry;
                    p->str = whole->substr(pos,i-pos);
                    _strings->strings_iEntry_id->push_back(p->Id);
                   strings.push_back(p->Id);//问题应该出现与vector的地址被改变了？！！！！
                }
                i++;//跳过d
                p = new IEntry;
                p->str = "%d";
                _strings->strings_iEntry_id->push_back(p->Id);
                strings.push_back(p->Id);
                pos = i+1;//下一个有效的可能为真正字符串的位置
            }
        }
        if (i!= 0 && i-pos>=1){
            //[pos,i)
            p = new IEntry;
            p->str = whole->substr(pos,i-pos);
            _strings->strings_iEntry_id->push_back(p->Id);
           strings.push_back(p->Id);
        }
    }

    PRINT_WORD;//PRINT StrCON or illegal
    GET_A_WORD;

    //不用语法输出
}

//void Parser::FormalChar() {
//
//}
//
//void Parser::NormalChar() {
//
//}

void Parser::InitVal(IEntry * iEntry,int & nums) { //变量数组值   iEntry存储初值 in values_Id
    if(WORD_TYPE == LBRACE){
        PRINT_WORD;//PRINT {
        GET_A_WORD;
        if (WORD_TYPE == RBRACE){
            //允许吗？  int a[1] = {}  先不考虑语义
            PRINT_WORD;//PRINT }
            GET_A_WORD;
        }else{
            InitVal(iEntry,nums);
            while(WORD_TYPE == COMMA){
                PRINT_WORD;//PRINT ,
                GET_A_WORD;
                InitVal(iEntry,nums);
            }
            if (WORD_TYPE != RBRACE){
                //ERROR
            }
            PRINT_WORD;//PRINT }
            GET_A_WORD;
        }
    }else{
        int value;
        auto *exp_iEntry = new IEntry;
        Exp(exp_iEntry, value, isInOtherFunc);//下放错误
        exp_iEntry->original_Name = iEntry->original_Name;
        if(ISGLOBAL){
            exp_iEntry->canGetValue =false;
            iEntry->values_Id->push_back(exp_iEntry->Id);
        }else{
            auto v = new IEntry;
            v->startAddress = iEntry->startAddress + 4*nums;
            v->canGetValue = false;
            iEntry->values_Id->push_back(v->Id);
            intermediateCode.addICode(Assign,exp_iEntry, nullptr,v);
        }

        //exp_iEntry->startAddress = iEntry->startAddress + 4*nums;
        //exp_iEntry->startAddress = iEntry->startAddress + 4*nums;//TODo:设置MIPS中的地址  为MIPS服务   由于我会在初始数组时sw

        nums++;
    }

    Print_Grammar_Output("<InitVal>");
}

void Parser::Cond(IEntry *iEntry) {
    //TODO:FOR循环的逻辑补充
    LOrExp(iEntry,isInOtherFunc);
    Print_Grammar_Output("<Cond>");
}

void Parser::ForStmt() {
    //TODO:ForStmt ICode  ASSIGN
    IEntry * for_stmt;
    for_stmt = new IEntry("for_stmt");//仅仅用来标识
    intermediateCode.addICode(Insert_Label,for_stmt, nullptr, nullptr);
    IEntry *lVal,*exp;
    int v1,v2;
    lVal = new IEntry;
    LVal(&lVal,v1,isInOtherFunc);
    PRINT_WORD;//PRINT =
    GET_A_WORD;
    exp = new IEntry;
    Exp(exp,v2,isInOtherFunc);
    /**
     * 鉴于LVal找到的很多数据并没有真正找到 而是拷贝版本  所以不能直接将lVal的操作当成已经操作
     */
    intermediateCode.addICode(Assign,exp, nullptr,lVal);
    lVal->canGetValue = false;
    Print_Grammar_Output("<ForStmt>");
}

void Parser::UnaryOp(int &op) {
    if (WORD_TYPE != PLUS && WORD_TYPE != MINU && WORD_TYPE != NOT){
        //ERROR
    }
    switch (WORD_TYPE) {
        case PLUS:
            op = 0;
            break;
        case MINU:
            op = 1;
            break;
        case NOT:
            op = 2;
            break;
        default:
            op = -1;
            break;
    }
    PRINT_WORD;
    GET_A_WORD;
    Print_Grammar_Output("<UnaryOp>");
}

//TODO:  _addExp使用前自己一个一个new
//RelExp-> AddExp {(< > <= >=) AddExp}
void Parser::RelExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_addExp1,*_addExp2 ,*ans;
    int _addValue1,_addValue2;
    Type t;
    _addExp1 = new IEntry;
    AddExp(_addExp1,_addValue1,isInOtherFunc);
    if(WORD_TYPE == LSS || WORD_TYPE == GRE || WORD_TYPE == LEQ || WORD_TYPE == GEQ){
        while(WORD_TYPE == LSS || WORD_TYPE == GRE || WORD_TYPE == LEQ || WORD_TYPE == GEQ){
            t = WORD_TYPE;
            Print_Grammar_Output("<RelExp>");
            PRINT_WORD;
            GET_A_WORD;
            _addExp2 = new IEntry;
            AddExp(_addExp2,_addValue2,isInOtherFunc);
            //TODO:比较的逻辑 建立中间代码
            switch (t) {
                case LSS:{
                    ans =  new IEntry;
                    intermediateCode.addICode(I_Less,_addExp1,_addExp2,ans);
                    break;
                }
                case GRE:{
                    ans =  new IEntry;
                    intermediateCode.addICode(I_Grt,_addExp1,_addExp2,ans);
                    break;
                }
                case LEQ:{
                    ans =  new IEntry;
                    intermediateCode.addICode(I_Less_eq,_addExp1,_addExp2,ans);
                    break;
                }
                case GEQ:{
                    ans =  new IEntry;
                    intermediateCode.addICode(I_Grt_eq,_addExp1,_addExp2,ans);
                    break;
                }
                default:
                    break;//no operator
            }
            intermediateCode.addICode(Assign,ans, nullptr,_addExp1);
        }
    }
intermediateCode.addICode(Assign,_addExp1, nullptr,iEntry);
    Print_Grammar_Output("<RelExp>");
}

void Parser::EqExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_relExp1,*_relExp2,*ans;
    _relExp1 = new IEntry;
    int op ;
//    int _relValue1,_relValue2;
    RelExp(_relExp1,isInOtherFunc);
    if(WORD_TYPE == EQL || WORD_TYPE ==NEQ){
        while(WORD_TYPE == EQL || WORD_TYPE ==NEQ){
            op = WORD_TYPE == EQL ? 0:1;
            Print_Grammar_Output("<EqExp>");
            PRINT_WORD;
            GET_A_WORD;
            _relExp2 = new IEntry;
            RelExp(_relExp2,isInOtherFunc);
            //TODO:逻辑
            if (op ==0){//EQL
                ans =  new IEntry;
                intermediateCode.addICode(I_Eq,_relExp1,_relExp2,ans);
            }else if (op == 1){
                ans =  new IEntry;
                intermediateCode.addICode(I_not_eq,_relExp1,_relExp2,ans);
            }else{
                //not this operator
            }
            intermediateCode.addICode(Assign,ans, nullptr,_relExp1);
        }
    }else{
        //error
    }
    intermediateCode.addICode(Assign,_relExp1, nullptr,iEntry);

    Print_Grammar_Output("<EqExp>");
}

void Parser::LAndExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_eqExp1,*_eqExp2,*ans;
    _eqExp1 = new IEntry;
    EqExp(_eqExp1,InOtherFunc);
    if(WORD_TYPE == AND){
        while(WORD_TYPE == AND){
            Print_Grammar_Output("<LAndExp>");
            PRINT_WORD;
            GET_A_WORD;
            _eqExp2 = new IEntry;
            EqExp(_eqExp2,InOtherFunc);
            //TODO:逻辑
            ans =  new IEntry;
            intermediateCode.addICode(I_And,_eqExp1,_eqExp2,ans);
            intermediateCode.addICode(Assign,ans, nullptr,_eqExp1);
        }
    }else{
        //error
    }
    intermediateCode.addICode(Assign,_eqExp1, nullptr,iEntry);
    Print_Grammar_Output("<LAndExp>");
}

void Parser::LOrExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_lAndExp1,*_lAndExp2,*ans;
    _lAndExp1  =new IEntry;
    LAndExp(_lAndExp1,InOtherFunc);
    if(WORD_TYPE == OR){
        while(WORD_TYPE == OR){
            Print_Grammar_Output("<LOrExp>");
            PRINT_WORD;
            GET_A_WORD;
            _lAndExp2 = new IEntry;
            LAndExp(_lAndExp2,InOtherFunc);
            //TODO :逻辑
            ans =  new IEntry;
            intermediateCode.addICode(I_Or,_lAndExp1,_lAndExp2,ans);
            intermediateCode.addICode(Assign,ans, nullptr,_lAndExp1);
        }
    }else{
        //error
    }
    intermediateCode.addICode(Assign,_lAndExp1, nullptr,iEntry);
    Print_Grammar_Output("<LOrExp>");
}

int Parser::Kind2Exp_type(Kind kind) {
    switch (kind) {
        case VAR:
        case CONST:
            return 0;
        case ARRAY_1_VAR:
        case ARRAY_1_CONST:
            return 1;
        case ARRAY_2_VAR:
        case ARRAY_2_CONST:
            return 2;
        default:
            return -1;//不会出现的  函数作为参数
    }
}

bool Parser::inArguments(const vector<Entry *>& arguments,string ident) {
    for (auto entry: arguments) {
        if(entry->ident == ident){
            return true;
        }
    }
    return false;
}
//我爱
//    我爱xjl徐佳琳

