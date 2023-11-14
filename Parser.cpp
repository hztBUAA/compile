//
// Created by hzt on 2023/9/25.
//



#include "Parser.h"
#include "Lexer.h"
#include <iostream>
using namespace std;
extern vector<IEntry *> IEntries;
string funcLabel;
bool isInOtherFunc;//区分中间代码是在主函数还是自定义函数   --注意定义还要分一个全局--既不是主函数 也不是

void Parser::Print_Grammar_Output(string s) {
    if (enablePrint){

        //cout << s << endl;
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
        ConstExp(exp_iEntries[op],values[op],isInOtherFunc);
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
        total_length = dim1_length = values[1];
        iEntry = new IEntry(total_length);
    }else if(op == 2){
        total_length = values[1]*values[2];//FIXME；数组初始大小
        dim1_length = values[2];
        iEntry = new IEntry(total_length);
    }

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
    if (WORD_TYPE == ASSIGN){
        hasValue = true;
        PRINT_WORD;
        GET_A_WORD;
        iEntry->values_Id = new vector<int>;//values_Id给变量数组    values直接给常量数组
        InitVal(iEntry,nums);
    }
    if(!error){
        entries.push_back(entry);
        semantic.recordEntries(entry);
        entry->id = iEntry->Id;
        if (ISGLOBAL){
            iEntry->isGlobal = true;//MIPS依据这个生成标签或者地址  lw
        }
        if (hasValue){
            if (op == 0){
                intermediateCode.addDef(ISGLOBAL,VAR_Def_Has_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
            }else{
                intermediateCode.addDef(ISGLOBAL,ARRAY_VAR_Def_Has_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
            }
        }else{
            if (op == 0){
                intermediateCode.addDef(ISGLOBAL,VAR_Def_No_Value,iEntry, nullptr, nullptr);//FIXME:addDef本身也是加入ICode  多了一个isGlobal参数
            }else{
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
        total_length = dim1_length = values[1];
        iEntry = new IEntry(total_length);
    }else if(op == 2){
        total_length = values[1]*values[2];//FIXME；数组初始大小
        dim1_length = values[2];
        iEntry = new IEntry(total_length);
    }

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
//常量定义  必须赋值
    if (WORD_TYPE != ASSIGN){
        //Error
    }else{
        PRINT_WORD;
        GET_A_WORD;
        //FIXME:这里的iEntry可以作为存放初值  --数组  --普通变量
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
        iEntry->dim1_length = nums;
    }else{
        int value;
        auto* _constExp = new IEntry;
        ConstExp(_constExp, value, isInOtherFunc);
        iEntry->values->push_back(_constExp->imm);//值要不统一存到values中  定义时  因为你不知道是数组还是啥 TODO： imm是确定的中间变量再用？
        nums++;
    }
    Print_Grammar_Output("<ConstInitVal>");
}

//FIXME:至多只有两层   append
//FIXME:nums表示目前的个数  一个ConstExp只有一个值   value表示ConstExp当前的算出值
void Parser::ConstExp(IEntry *iEntry,int&value,bool InOtherFunc) {
    iEntry = new IEntry;
    AddExp(iEntry, value, InOtherFunc);
//    iEntry->values->push_back(value);//FIXME: ConstExp一定可以算出来
    Print_Grammar_Output("<ConstExp>");
}

//Exp->AddExp
//AddExp->MulExp{[+-]MulExp}
//认为iEntry由下一级反馈  如果是空 说明值已经算出来放进了value   非空说明值为临时变量
void Parser::AddExp(IEntry *iEntry,int&value,bool iInOtherFunc) {
    int value1,value2;
    auto*iEntry1 = new IEntry ,*iEntry2 = new IEntry;
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
            MulExp(iEntry2, value2, iInOtherFunc);
            if (op == 0){
                if (iEntry1->canGetValue &&  iEntry2->canGetValue){
                    value = iEntry1->imm + iEntry2->imm;
                    iEntry1->imm = value;
                    iEntry1->canGetValue = true;
                }else {
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Add, iEntry1, iEntry2, ans);
                    iEntry1 = ans;
                }
            }else{
                if (iEntry1->canGetValue &&  iEntry2->canGetValue){
                    value = iEntry1->imm - iEntry2->imm;
                    iEntry1->imm = value;
                    iEntry1->canGetValue = true;
                }else{
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Sub,iEntry1,iEntry2,ans);
                    iEntry1 = ans;
                }
            }

        }
    }else{
        //error
    }
    //TODO: imm & address
    if (iEntry1->canGetValue){
        iEntry->imm = iEntry1->imm;
        iEntry->canGetValue = true;
    }else{
        iEntry->canGetValue = false;
        iEntry->startAddress = iEntry1->startAddress;//TODO:新理解 要么给个具体值  要么给个将来运行时的具体地址
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<AddExp>");
    //已经指向下一个
}
//MulExp是项  下面有因子
//MulExp →UnaryExp [*/%] {UnaryExp}
void Parser::MulExp(IEntry *iEntry,int&value,bool InOtherFunc) {
    int value1,value2;
    auto*iEntry1 = new IEntry ,*iEntry2 = new IEntry;
    IEntry * ans;
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
            UnaryExp(iEntry2, value2, InOtherFunc);
            if (op == 0){
                if (iEntry1->canGetValue &&  iEntry2->canGetValue){
                    value = iEntry1->imm * iEntry2->imm;
                    ans = new IEntry;
                    ans->canGetValue = true;
                    ans->imm = value;
                }else {
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Mult, iEntry1, iEntry2, ans);
                }
            }else if(op ==1){
                if (iEntry1->canGetValue &&  iEntry2->canGetValue){
                    value = iEntry1->imm / iEntry2->imm;
                    ans = new IEntry;
                    ans->canGetValue = true;
                    ans->imm = value;
                }else {
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Div, iEntry1, iEntry2, ans);
                }
            }else{
                if (iEntry1->canGetValue &&  iEntry2->canGetValue){
                    value = iEntry1->imm / iEntry2->imm;
                    ans = new IEntry;
                    ans->canGetValue = true;
                    ans->imm = value;
                }else {
                    ans = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Mod, iEntry1, iEntry2, ans);
                }
            }
            iEntry1 = ans;
        }
    }else{
        //error
    }
    if (iEntry1->canGetValue){
        iEntry->imm = iEntry1->imm;
        iEntry->canGetValue = true;
    }else{
        iEntry->canGetValue = false;
        iEntry->startAddress = iEntry1->startAddress;//TODO:新理解 要么给个具体值  要么给个将来运行时的具体地址
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<MulExp>");
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
//        cout << "1   "+to_string(errorHandler.error_line);
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
                }else{
                    if (func->kind == FUNC_INT){
                        intermediateCode.addICode(FuncCall,IEntries.at(func->id), nullptr, iEntry);//FIXME:又返回值的函数  把值给到这个新建的iEntry  需要自己新建iEntry
                    }else{
                        intermediateCode.addICode(FuncCall,IEntries.at(func->id), nullptr, nullptr);
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
                auto * find_func = IEntries.at(func->id);
                auto *func_rParams = find_func->values_Id;//FIXME:values_address解决了有些值可能不是直接imm显示的
                FuncRParams(func_ident_line,func_rParams);
                if (WORD_TYPE != RPARENT){
                    //Error  缺少右括号）
                    errorHandler.Insert_Error(RPARENT_MISSING);
                }else {
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
            if(func->kind == FUNC_INT)
                Exp_type = 0;
            else if(func->kind == FUNC_VOID)//void  或者根本没有定义的函数 且已经报错未定义
                Exp_type = -5;
            else
                Exp_type = -4;
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
            if (iEntry->canGetValue){
                value = -1 * value;
            }
        }else if(op ==2 ){
            //FIXME:仅出现在条件表达式？TODO
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
    }else{  //  指向ident
        LVal(iEntry, value, InOtherFunc);//不在这一层报错？   放到下一层LVal = getint() | Exp
        if (WORD_TYPE == ASSIGN){//FIXME:从Stmt-> LVal = ...来的
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
                intermediateCode.addICode(GetInt, nullptr, nullptr,iEntry);
            } else {
                IEntry*exp;
                Exp(exp, value, InOtherFunc);//FIXME:直接将LVal的IEntry赋值到Exp中 表示Exp的最终结果就是LVal的内存所在区域的值！  如果不是直接求出值  那么
                if (exp->canGetValue){
                    iEntry->imm = exp->imm;//值传递  修改值就行
                }else{
                    intermediateCode.addICode(Assign,exp, nullptr,iEntry);//一般的传递
                }
            }
            //FIXME:这里是用来表示LVal是真正的左值  也就是语法树中不被算作Exp的  也就是本来LVal = getint（） | Exp这些是在Stmt中的  我的写法会让它在Stmt-》Exp中进行推导完成  无伤大雅  在此告诉自己
            isLValInStmt = true;
        }
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<PrimaryExp>");
}

void Parser::LVal(IEntry * iEntry,int & value,bool inOtherFunc) { // 这里面中的容易错的地方 ident  line已经指向下一个字符前所在的行
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
    Entry *find;
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

    int dim1_length = find->dim1_length;
    int index = 0;
    IEntry * index_entry;
    if (Exp_type == -3){
        //没找到 未定义
//        errorHandler.Insert_Error(NOT_DEFINE);
    }else if(Exp_type < 0){
        //超出维数的引用   不出现
    }else if(Exp_type == 0){
        //
        if (op == 2){
            if(array_exps[2]->canGetValue && array_exps[1]->canGetValue){
                index = values[1]*dim1_length + values[2];
                if (find->kind == ARRAY_2_CONST){
                    iEntry = new IEntry;
                    iEntry->canGetValue = true;
//                    value = find->values.at(index);
                    value =IEntries.at(find->id)->values->at(index);
                    iEntry->imm = value;
                }else{
                    intermediateCode.addICode(GetArrayElement,index,IEntries.at(find->id),iEntry);
                }
            }else{
                if (array_exps[1]->canGetValue){
                    int t = values[1]*dim1_length;
                    intermediateCode.addICode(IntermediateCodeType::Add,t,array_exps[2],index_entry);
                }
                else if(array_exps[2]->canGetValue){
                    auto *t = new IEntry;
                    intermediateCode.addICode(IntermediateCodeType::Mult,dim1_length,array_exps[1],t);
                    intermediateCode.addICode(IntermediateCodeType::Add,values[2],t,index_entry);
                }else{
                    intermediateCode.addICode(IntermediateCodeType::Add,array_exps[1],array_exps[2],index_entry);
                }
                intermediateCode.addICode(GetArrayElement,IEntries.at(find->id),index_entry,iEntry);
            }
        }else if(op == 1){
            if(array_exps[1]->canGetValue) {
                index = values[1];
                if (find->kind == ARRAY_1_CONST){
                    iEntry->canGetValue = true;
                    value = IEntries.at(find->id)->values->at(index);
                    iEntry->imm = value;
                }else{
                    intermediateCode.addICode(GetArrayElement,index,IEntries.at(find->id),iEntry);
                }
            }
        }else{
//            intermediateCode.addICode(Assign, intermediateCode.getIEntries.at(find->id), nullptr, iEntry);
                iEntry = IEntries.at(find->id);//引用时 引用的是本身      区别与函数调用时的普通变量  会是新生成IEntry
        }
    }else if(Exp_type == 1){ //find就是对应的曾经定义过的Entry   iEntry标识直接传递地址
        //一维地址
        /**
         * 重新生成一个IEntry   用来表示内存位置
         * TODO:先不考虑烦人的array找不到对应的值的情况---需要进一步生成index的中间代码
         */
         if(op == 1){
             //原生的一维数组
             //只可能在函数实参中出现和函数形参
             iEntry = new IEntry;
             iEntry->startAddress =IEntries.at(find->id)->startAddress;//这样传的就是地址  只不是体现在我的程序中IEntry是新的  这只是为了不要弄脏起初定义数组时的数据格子 指的都是同一个
             iEntry->type = 1;
         }else if(op == 2){
            index = find->dim1_length *values[1];
             iEntry = new IEntry;
             iEntry->type = 1;
             iEntry->startAddress = index + IEntries.at(find->id)->startAddress;
         }
    }else{
        //2维地址
        iEntry = new IEntry;
        iEntry->type = 1;
        iEntry->startAddress = IEntries.at(find->id)->startAddress;
    }






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
    value = lexer.token.number;
//    iEntry = new IEntry;FIXME:高层建立iEntry  传下来  不然似乎会有野指针问题？
    iEntry->canGetValue = true;
    iEntry->imm = value;
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

//TODO:函数实参一一赋值给已经占住位置的形参
void Parser::FuncRParams(int func_ident_line,vector<int>  *FParams) {
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
    IEntry * exp_iEntry;
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
//    FParams->push_back(exp_iEntry->Id);
    intermediateCode.addICode(Assign, exp_iEntry,nullptr,IEntries.at(FParams->at(cnt-1)));

    while(WORD_TYPE == COMMA){
        PRINT_WORD;
        GET_A_WORD;
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
//        FParams->push_back(exp_iEntry->Id);
        intermediateCode.addICode(Assign, exp_iEntry,nullptr,IEntries.at(FParams->at(cnt-1)));
    }
    if(cnt < FArguments.size() &&!already_error_func_type&&!already_error_func_count&& func != nullptr){//实际调用参数少
        errorHandler.Insert_Error(FUNC_RPARAMS_COUNT_ERROR,func_ident_line);//不会出现一行两个错误 既有
    }
    Print_Grammar_Output("<FuncRParams>");
}

void Parser::FuncDef(Kind func_type) {
    isInOtherFunc = true;
    //FuncDef 从名字<ident>开始
    bool error = false;
    string ident =WORD_DISPLAY;
    funcLabel = ident;
    auto* func = new IEntry ;//FIXME:表示函数定义时的相关信息头： 形参values_Id 有没有返回值

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
        func->values_Id = new vector<int>;
        INFO_ENTRY->fParams = new vector<Entry *>;
        INFO_ENTRY->id = func->Id;//后续被引用时才知道是这个IEntry的函数头
        semantic.recordEntries(INFO_ENTRY);
        if (WORD_TYPE == RPARENT){
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
        func->values_Id = new vector<int>;
        INFO_ENTRY->id = func->Id;
        semantic.recordEntries(INFO_ENTRY);
        tableManager.downTable(ident);//对于有参函数   参数和block都属于这个函数名的对应符号表level
        vector<Entry *> entries;//涉及到堆上内存引用？
        if(WORD_TYPE == INTTK){
            FuncFParams(entries);
            //指向）
            //FIXME:将IEntry的对应形参存储完善
            for (auto entry :entries) {
                func->values_Id->push_back(entry->id);//传递的最终都是4字节的  值或地址！
            }
            if(WORD_TYPE != RPARENT){
                //ERROR
                errorHandler.Insert_Error(RPARENT_MISSING);
            }else{
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }

            tableManager.cur->fParams = new vector(entries);//还真是   如果直接&entries  会丢失
//            semantic.recordEntries(entries);//参数也重新放进这个函数中 当做局部变量
            Block();
            tableManager.upTable();
        }else{
            //Error  形参只能int打头    如果改成增加double！！！
            //TODO:完善
        }
    }
    tableManager.cur->entries->erase("main");//如果是重定义的函数 需要抹掉它
    //TODO:FuncCall中间代码 FIXME：完成FuncCALL   src1为函数头
    intermediateCode.addICode(FuncCall,func, nullptr, nullptr);

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
                //ERROR  二维数组 最后一维需要常数
            }
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
        }else if(op == 1){
            kind = ARRAY_1_VAR;//关于kind 类型匹配 需要放宽 const常量  int变量？说明：常量数组不允许加到参数中  所以都是VAR类型即可
        }else{
            kind = ARRAY_2_VAR;
        }
        INFO_ENTRY = semantic.fillInfoEntry(ident,kind);
        INFO_ENTRY->id = (new IEntry)->Id;//FIXME：函数形参站住位置
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
    int printf_count = 0;
    bool loop_error = true;
    IEntry * exp;
    int exp_value;
//    int printf_format = 0;
    switch (WORD_TYPE) {
        case PRINTFTK:
//            errorHandler.error_type = NORMAL;
            PRINT_WORD;//PRINT PRINTF
            printf_line = lexer.line;
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            FormatString();

            while(WORD_TYPE == COMMA){
                printf_count++;
                PRINT_WORD;//PRINT ,
                GET_A_WORD;
                Exp(exp,exp_value,isInOtherFunc);
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
                PRINT_WORD;//PRINT ;
                GET_A_WORD;
            }
            //TODO:printf
            break;
        case RETURNTK:
            PRINT_WORD;//PRINT RETURN
            GET_A_WORD;
            if(WORD_TYPE == SEMICN){
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

                    Exp(exp,exp_value,isInOtherFunc);//错误下放
                    if (WORD_TYPE != SEMICN){
                        //ERROR
                        //Print_Grammar_Output("ERROR  :");
                        errorHandler.Insert_Error(SEMICOLON_MISSING);
                    }else{
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
            }
            if (WORD_TYPE != SEMICN){
                //ERROR
                errorHandler.Insert_Error(SEMICOLON_MISSING);
            }else{
                PRINT_WORD;//PRINT ;
                GET_A_WORD;
            }
            break;
        case FORTK:
            PRINT_WORD;//PRINT FOR
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            if (WORD_TYPE == SEMICN){
                PRINT_WORD;//PRITN ;
                GET_A_WORD;
            }else{
                ForStmt();
                if (WORD_TYPE != SEMICN){
                    //ERROR
                    errorHandler.Insert_Error(SEMICOLON_MISSING);//虽然for语句强调了不会出现这个错误
                }else{
                    PRINT_WORD;//PRINT ;
                    GET_A_WORD;
                }
            }
            if (WORD_TYPE == SEMICN){
                PRINT_WORD;//PRITN ;
                GET_A_WORD;
            }else{
                Cond();
                if (WORD_TYPE != SEMICN){
                    //ERROR
                    errorHandler.Insert_Error(SEMICOLON_MISSING);
                }else{
                    PRINT_WORD;//PRINT ;
                    GET_A_WORD;
                }
            }
            if(WORD_TYPE == RPARENT){
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }else{
                ForStmt();
                if (WORD_TYPE != RPARENT){
                    //error  这其实不可能会有
                    errorHandler.Insert_Error(RPARENT_MISSING);
                }else{
                    PRINT_WORD;//PRINT )
                    GET_A_WORD;
                }
            }
            ident ="for";
            semantic.recordEntries(semantic.fillInfoEntry(ident,FOR));
            //向下一层符号表
            tableManager.downTable(ident);
            tableManager.cur->loop_count++;
            Stmt();
            tableManager.cur->loop_count--;
            tableManager.upTable();
            tableManager.cur->entries->erase(ident);//for 结束了就不需要再留位置 给之后的for让路
            //向上一层符号表
            break;
        case IFTK:
            ident = "if";
            PRINT_WORD;//PRINT IF
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            Cond();
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
            Stmt();
            tableManager.upTable();
            tableManager.cur->entries->erase(ident);//for 结束了就不需要再留位置 给之后的for让路
            if (WORD_TYPE == ELSETK){
                PRINT_WORD;//PRINT ELSE
                GET_A_WORD;
                semantic.recordEntries(semantic.fillInfoEntry(ident,ELSE));
                //向下一层符号表
                tableManager.downTable(ident);
                Stmt();
                tableManager.upTable();
                tableManager.cur->entries->erase(ident);//for 结束了就不需要再留位置
            }
            break;
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
            IEntry *exp_iEntry;
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

void  Parser::FormatString() {

    if (WORD_TYPE == ILLEGAL){
        //ERROR
        //可能还需要注意 格式字符串是否合法？
        errorHandler.Insert_Error(ILLEGAL_STRING);
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
        iEntry->dim1_length = nums;
    }else{
        int value;
        auto *exp_iEntry = new IEntry;
        Exp(exp_iEntry, value, isInOtherFunc);//下放错误
        iEntry->values_Id->push_back(exp_iEntry->Id);
        nums++;
    }

    Print_Grammar_Output("<InitVal>");
}

void Parser::Cond() {
    //TODO:FOR循环的逻辑补充
    IEntry *iEntry;
    LOrExp(iEntry,isInOtherFunc);
    Print_Grammar_Output("<Cond>");
}

void Parser::ForStmt() {
    //TODO:ForStmt ICode
    IEntry *lVal,*exp;
    int v1,v2;
    LVal(lVal,v1,isInOtherFunc);
    PRINT_WORD;//PRINT =
    GET_A_WORD;
    Exp(exp,v2,isInOtherFunc);
    if (exp->canGetValue){
        lVal->imm = v2;
    }else{
        intermediateCode.addICode(Assign,exp, nullptr,lVal);
    }
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

//RelExp-> AddExp {(< > <= >=) AddExp}
void Parser::RelExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_addExp1,*_addExp2;
    int _addValue1,_addValue2;
    AddExp(_addExp1,_addValue1,isInOtherFunc);
    if(WORD_TYPE == LSS || WORD_TYPE == GRE || WORD_TYPE == LEQ || WORD_TYPE == GEQ){
        while(WORD_TYPE == LSS || WORD_TYPE == GRE || WORD_TYPE == LEQ || WORD_TYPE == GEQ){
            Print_Grammar_Output("<RelExp>");
            PRINT_WORD;
            GET_A_WORD;
            AddExp(_addExp2,_addValue2,isInOtherFunc);
            //TODO:比较的逻辑 建立中间代码
        }
    }
    Print_Grammar_Output("<RelExp>");
}

void Parser::EqExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_relExp1,*_relExp2;
//    int _relValue1,_relValue2;
    RelExp(_relExp1,isInOtherFunc);
    if(WORD_TYPE == EQL || WORD_TYPE ==NEQ){
        while(WORD_TYPE == EQL || WORD_TYPE ==NEQ){
            Print_Grammar_Output("<EqExp>");
            PRINT_WORD;
            GET_A_WORD;
            RelExp(_relExp2,isInOtherFunc);
//TODO:逻辑
        }
    }else{
        //error
    }
    Print_Grammar_Output("<EqExp>");
}

void Parser::LAndExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_eqExp1,*_eqExp2;
    EqExp(_eqExp1,InOtherFunc);
    if(WORD_TYPE == AND){
        while(WORD_TYPE == AND){
            Print_Grammar_Output("<LAndExp>");
            PRINT_WORD;
            GET_A_WORD;
            EqExp(_eqExp2,InOtherFunc);
            //TODO:逻辑
        }
    }else{
        //error
    }
    Print_Grammar_Output("<LAndExp>");
}

void Parser::LOrExp(IEntry * iEntry,bool InOtherFunc) {
    IEntry *_lAndExp1,*_lAndExp2;
    LAndExp(_lAndExp1,InOtherFunc);
    if(WORD_TYPE == OR){
        while(WORD_TYPE == OR){
            Print_Grammar_Output("<LOrExp>");
            PRINT_WORD;
            GET_A_WORD;
            LAndExp(_lAndExp2,InOtherFunc);
            //TODO :逻辑
        }
    }else{
        //error
    }
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

