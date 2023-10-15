//
// Created by hzt on 2023/9/25.
//

#include "Parser.h"
#include "Lexer.h"
#include <iostream>
using namespace std;



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
                while(isspace(lexer.ch)){
                    lexer.ch = (char )lexer.sourceFile.get();
                }
                if (lexer.ch == '('){//FuncDef 从名字<ident>开始  还没有输出functype
                    //FUNCTYPE
                    FuncType();
                    FuncDef();
                }else if(lexer.ch == ',' || lexer.ch == ';'  || lexer.ch == '=' || lexer.ch == '['){
                    Decl();//变量声明部分  入口是ident
                }else{
                    //Error
                }
            }
        }else if(WORD_TYPE == VOIDTK){
            PRINT_WORD;//PRINT VOID
            GET_A_WORD;
            FuncType();
            if(WORD_TYPE != IDENFR){
                //ERROR
            }
            FuncDef();
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
        while (isspace(lexer.ch)){
            lexer.ch = lexer.sourceFile.get();
        }
        if(lexer.ch == ',' || lexer.ch == ';'  || lexer.ch == '=' || lexer.ch == '['){
            VarDecl();//变量声明部分
        }
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
        errorHandler.Print_Error(SEMICOLON_MISSING);
//        semantic.skip(VARDECL);
        semantic.recordEntries(entries);
        return;
    }

    //不缺少分号 会运行到这儿
    PRINT_WORD;//PRINT ;
    Print_Grammar_Output("<VarDecl>");
    semantic.recordEntries(entries);
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
    if(tableManager.isRedefine(ident)){
        errorHandler.Print_Error(REDEFINE);
        error = true;
    }//重定义错误，局部化处理

    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    while(WORD_TYPE == LBRACK){
        op++;
        PRINT_WORD;//PRINT [
        GET_A_WORD;
        ConstExp();
        if (WORD_TYPE == RBRACK){
            PRINT_WORD;//PRINT ]
            GET_A_WORD;
        }else{
            //Error  缺少右中括号
            errorHandler.Print_Error(RBRACK_MISSING);
            error =true;
        }
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
    if (WORD_TYPE == ASSIGN){
        PRINT_WORD;
        GET_A_WORD;
        InitVal();
    }
    if(!error){
        entries.push_back(entry);
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
        errorHandler.Print_Error(SEMICOLON_MISSING);
//        semantic.skip(CONSTDECL);
        semantic.recordEntries(entries);
        return;
    }

    PRINT_WORD;//PRINT ;
    Print_Grammar_Output("<ConstDecl>");
    tableManager.insertIntoTable(entries);
    GET_A_WORD;//point to next word
}

void Parser::ConstDef(vector<Entry*>& entries) {
    if (WORD_TYPE != IDENFR){
        //Error
    }
    bool error = false;
    string ident = WORD_DISPLAY;
    //定义：先在这一层找   找到就报错REDEFINE  没找到就填表
    if(tableManager.isRedefine(ident)){
        errorHandler.Print_Error(REDEFINE);
        error = true;
    }

    PRINT_WORD;
    GET_A_WORD;
    int op = 0;
    while(WORD_TYPE == LBRACK){
        op++;
        PRINT_WORD;
        GET_A_WORD;
        ConstExp();
        if (WORD_TYPE == RBRACK){
            PRINT_WORD;
            GET_A_WORD;
        }else{
            //Error
            errorHandler.Print_Error(RBRACK_MISSING);
            error = true;
        }
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
        ConstInitVal();
    }
    //留下对数组长度的补充计算  以及变量值的存储
    //已经指向下一个word  ,  or ;
    if(!error){
        entries.push_back(entry);
    }
    Print_Grammar_Output("<ConstDef>");
}

void Parser::ConstInitVal() {
    if (WORD_TYPE == LBRACE){
        PRINT_WORD;//PRINT {
        GET_A_WORD;
        ConstInitVal();
        while(WORD_TYPE == COMMA){
            PRINT_WORD;//PRINT ,
            GET_A_WORD;
            ConstInitVal();
        }
        if(WORD_TYPE != RBRACE){
            //Error
        }
        PRINT_WORD;//PRINT }
        GET_A_WORD;
    }else{
        ConstExp();
    }
    Print_Grammar_Output("<ConstInitVal>");
}

void Parser::ConstExp() {
    AddExp();
    Print_Grammar_Output("<ConstExp>");
}

void Parser::AddExp() {
    MulExp();
    if (WORD_TYPE == PLUS || WORD_TYPE == MINU){
        while(WORD_TYPE == PLUS || WORD_TYPE == MINU){
            if (!isLValInStmt)
                Print_Grammar_Output("<AddExp>");
            PRINT_WORD;//print + -
            GET_A_WORD;
            MulExp();

        }
    }else{
        //error
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<AddExp>");
    //已经指向下一个
}
void Parser::MulExp() {
    UnaryExp();
    if (WORD_TYPE == MULT || WORD_TYPE == DIV || WORD_TYPE == MOD){
        while(WORD_TYPE == MULT || WORD_TYPE == DIV || WORD_TYPE == MOD){
            if (!isLValInStmt)
                Print_Grammar_Output("<MulExp>");
            PRINT_WORD;//print the */%
            GET_A_WORD;
            UnaryExp();

        }
    }else{
        //error
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<MulExp>");
    //已经指向下一个
}
void Parser::UnaryExp() {
    if (WORD_TYPE == LPARENT || WORD_TYPE == INTCON){
        PrimaryExp();
    }else if(WORD_TYPE == IDENFR){
        //lexer.ch = PEEK_A_LETTER;   为了去除所有的空白字符  用peek无法做到
        //之所以不用GET_A_WORD   是因为  这样会丢失ident    造成约束不一致
        //GET_A_WORD总是从ch当前属于的字符进行划分
        while(isspace(lexer.ch)){
            lexer.ch = (char )lexer.sourceFile.get();
        }
        if(lexer.ch == '('){
            int RParams_count = 0;
            //进入FuncRParams
            PRINT_WORD;//PRINT IDENT
            string ident = WORD_DISPLAY;
            if( ! semantic.isEverDefined(ident,FUNC_INT, false)){//先认为是int函数   不做过多要求
                errorHandler.Print_Error(NOT_DEFINE);
            }
            //无论是否报错  都继续  报了错就是正常的
            GET_A_WORD;//算上 （
            PRINT_WORD;//PRINT (
            GET_A_WORD;//指向FuncRParams  也可能是无参少了右括号？？？？
            //可以没有参数
            if(WORD_TYPE == RPARENT){
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }else if(WORD_TYPE == LPARENT || WORD_TYPE == INTCON || WORD_TYPE == IDENFR || WORD_TYPE == PLUS || WORD_TYPE == MINU){
                //函数实参的first集
                FuncRParams();
                if (WORD_TYPE != RPARENT){
                    //Error  缺少右括号）
                    errorHandler.Print_Error(RPARENT_MISSING);
                }else {
                    PRINT_WORD;//PRINT )
                    GET_A_WORD;//point to next WORD
                }
            }else{
                    //函数无参 且缺失 ‘）’
                errorHandler.Print_Error(RPARENT_MISSING);
            }
        }else if(lexer.ch == '['){
            PrimaryExp();
        }else{
            //Error  or  PrimaryExp 's LVal
            PrimaryExp();//下放错误
        }
    }else{
        UnaryOp();
        UnaryExp();
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<UnaryExp>");
}

void Parser::PrimaryExp() {
    if (WORD_TYPE == LPARENT){
        PRINT_WORD;//PRINT (
        GET_A_WORD;
        Exp();
        if (WORD_TYPE != RPARENT){
            //Error
            errorHandler.Print_Error(RPARENT_MISSING);
        }else{
            PRINT_WORD;//PRINT )
            GET_A_WORD;
        }
    }else if(WORD_TYPE == INTCON){
        Number();
    }else{  //  指向ident
        LVal();//不在这一层报错？   放到下一层LVal
        if (WORD_TYPE == ASSIGN){
            PRINT_WORD;//PRINT =
            GET_A_WORD;
            if (WORD_TYPE == GETINTTK){
                PRINT_WORD;//PRITN GETINT
                GET_A_WORD;
                PRINT_WORD;//PRINT (
                GET_A_WORD;
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            } else {
                Exp();
            }
            isLValInStmt = true;
        }
    }
    if (!isLValInStmt)
        Print_Grammar_Output("<PrimaryExp>");
}

void Parser::LVal() {
    if (WORD_TYPE != IDENFR){
        //Error
    }
    PRINT_WORD;//PRINT IDENT
    string ident = WORD_DISPLAY;
    Kind kind;
    int op  =0;

    //无论是否报错  都继续  报了错就是正常的
    GET_A_WORD;
    while(WORD_TYPE == LBRACK){
        op++;
        PRINT_WORD;//PRINT [
        GET_A_WORD;
        Exp();
        if (WORD_TYPE != RBRACK){
            //Error  缺少有中括号]
            errorHandler.Print_Error(RBRACK_MISSING);
        }else{
            PRINT_WORD;//PRINT ]
            GET_A_WORD;//POINT TO NEXT WORD
        }
    }
    if(op == 0){
        kind = VAR;
    }else if(op == 1){
        kind = ARRAY_1_VAR;
    }else{
        kind = ARRAY_2_VAR;
    }
    //错误的先后顺序  先检查方括号匹配 ---判断具体类型 再判断是否已经定义过 一行只有一个错误 不可能考到这个
    if(WORD_TYPE == ASSIGN && ! semantic.isEverDefined(ident,kind, true)){
        errorHandler.Print_Error(NOT_DEFINE);
    }
    Print_Grammar_Output("<LVal>");
}

void Parser::Number() {
    if (WORD_TYPE != INTCON){
        //Error
    }
    PRINT_WORD;
    Print_Grammar_Output("<Number>");
    GET_A_WORD;//NOT PW
}

void Parser::Exp() {
    AddExp();
    if (!isLValInStmt)
        Print_Grammar_Output("<Exp>");
}

void Parser::MainFuncDef() {
    if (WORD_TYPE != MAINTK){
        //ERROR
    }
    PRINT_WORD;//PRINT MAIN
    GET_A_WORD;//POINT TO (
    PRINT_WORD;//PRINT (
    GET_A_WORD;//point to )
    if (WORD_TYPE != RPARENT){//point to {
        errorHandler.Print_Error(RPARENT_MISSING);
    }else{
        PRINT_WORD;//print )
        GET_A_WORD;//point to {
    }
    Block();
    Print_Grammar_Output("<MainFuncDef>");
}

void Parser::FuncRParams() {
    //依次取出Exp的类型 然后判断函数符号表中的vector   类型只需要看 0 1 2 普通变量 一维数组 二维数组
    //可是应该怎么比较好的完成这个Exp类型的判断呢
    Exp();
    while(WORD_TYPE == COMMA){
        PRINT_WORD;
        GET_A_WORD;
        Exp();
    }
    Print_Grammar_Output("<FuncRParams>");
}

void Parser::FuncDef() {
    //FuncDef 从名字<ident>开始
    bool error = false;
    string ident =WORD_DISPLAY;

    //定义：先在这一层找   找到就报错REDEFINE  没找到就填表
    if(tableManager.isRedefine(ident)){
        errorHandler.Print_Error(REDEFINE);
        error = true;
        ident = "main";
    }
    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    PRINT_WORD;//PRINT (
    GET_A_WORD;
    if(WORD_TYPE == RPARENT || WORD_TYPE == LBRACE){//可能的情况func({  空参数漏了）
        //no FUNCFParams
        INFO_ENTRY = semantic.fillInfoEntry(ident,FUNC_INT);//空参数
        INFO_ENTRY->fParams = new vector<Entry *>;
        semantic.recordEntries(INFO_ENTRY);
        if (WORD_TYPE == RPARENT){
            PRINT_WORD;
            GET_A_WORD;//point to {
        }else{
            errorHandler.Print_Error(RPARENT_MISSING);
        }
        tableManager.downTable(ident);
        Block();
        tableManager.upTable();
    }else{
        vector<Entry *> entries;
        if(WORD_TYPE == INTTK){
            FuncFParams(entries);
            //指向）
            if(WORD_TYPE != RPARENT){
                //ERROR
                errorHandler.Print_Error(RPARENT_MISSING);
            }else{
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }
            INFO_ENTRY = semantic.fillInfoEntry(ident,FUNC_INT);
            INFO_ENTRY->fParams = &entries;
            semantic.recordEntries(INFO_ENTRY);
            tableManager.downTable(ident);//对于有参函数   参数和block都属于这个函数名的对应符号表level
            semantic.recordEntries(entries);//参数也重新放进这个函数中 当做局部变量
            Block();
            tableManager.upTable();
        }else{
            //Error  形参只能int打头


        }
    }
    tableManager.cur->entries->erase("main");//如果是重定义的函数 需要抹掉它

    Print_Grammar_Output("<FuncDef>");
}

void Parser::FuncType() {
    Print_Grammar_Output("<FuncType>");
}

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
    if(tableManager.isRedefine(ident)){
        errorHandler.Print_Error(REDEFINE);
    }

    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    if(WORD_TYPE != LBRACK){
        //认为就是普通变量  这里不会有error
        op = 0;
    }else{
        PRINT_WORD;//PRINT [
        op++;
        GET_A_WORD;
        if(WORD_TYPE != RBRACK){
            //ERROR  缺少右中括号
            op = -1;
            errorHandler.Print_Error(RBRACK_MISSING);
            goto END;
        }
        PRINT_WORD;//PRINT ]   一维数组变量
        GET_A_WORD;
        if(WORD_TYPE != LBRACK){
            //认为就是一维数组变量  这里不会有error
        }else{
            //二维数组变量
            op++;
            PRINT_WORD;//PRINT [
            GET_A_WORD;
            if(WORD_TYPE == RBRACK){
                //ERROR  二维数组 最后一维需要常数
            }
            ConstExp();
            if(WORD_TYPE != RBRACK){
                //ERROR  缺少右中括号
                op = -1;
                errorHandler.Print_Error(RBRACK_MISSING);
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
        PRINT_WORD;//PRINT }
        GET_A_WORD;
    }else{
        while(WORD_TYPE != RBRACE){
            BlockItem();//这一级不处理错误
            //只看最后一句有没有return  在这里进行一个peek
        }
        if(WORD_TYPE != RBRACE){
            //ERROR
            errorHandler.Print_Error(RBRACE_MISSING);
        }else{
            if (tableManager.cur->kind == FUNC_INT && tableManager.cur->return_error){
                errorHandler.Print_Error(INT_FUNC_NO_RETURN);
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

void Parser::Stmt() {
    string ident;
    Entry * temp;
    int printf_line = 0;
    int printf_count = 0;
    int printf_format = 0;
    switch (WORD_TYPE) {
        case PRINTFTK:
            PRINT_WORD;//PRINT PRINTF
            printf_line = lexer.line;
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            FormatString(printf_line);
            while(WORD_TYPE == COMMA){
                printf_count++;
                PRINT_WORD;//PRINT ,
                GET_A_WORD;
                Exp();
            }
            if(printf_format != printf_count){
                errorHandler.Print_Error(PRINTF_NOT_EQUAL_COUNT);
            }
            if(WORD_TYPE != RPARENT){
                //缺少右圆括号
                errorHandler.Print_Error(RPARENT_MISSING);
            }else{
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }
            if(WORD_TYPE != SEMICN){
                //缺少分号
                errorHandler.Print_Error(SEMICOLON_MISSING);
            }else{
                PRINT_WORD;//PRINT ;
                GET_A_WORD;
            }
            break;
        case RETURNTK:
            PRINT_WORD;//PRINT RETURN
            GET_A_WORD;
            if(WORD_TYPE == SEMICN){
                PRINT_WORD;//PRINT ;
                GET_A_WORD;//return；空
            }else{
                Exp();//错误下放
                if (WORD_TYPE != SEMICN){
                    //ERROR
                    //Print_Grammar_Output("ERROR  :");
                    errorHandler.Print_Error(SEMICOLON_MISSING);
                }else{
                    PRINT_WORD;//print ;
                    GET_A_WORD;
                }
            }
            temp = tableManager.cur;
            while(temp != nullptr){
                if (temp->kind == FUNC_VOID){
                    errorHandler.Print_Error(VOID_FUNC_HAS_RETURN);
                    break;
                }
            }
            if (WORD_TYPE == RBRACE && tableManager.cur->kind == FUNC_INT){
                tableManager.cur->return_error = false;
            }
            break;
        case BREAKTK:
        case CONTINUETK:
            PRINT_WORD;//PRINT CONTINUE
            GET_A_WORD;
            if(tableManager.cur->loop_count == 0){
                errorHandler.Print_Error(NOT_LOOP_USING_BC);
            }
            if (WORD_TYPE != SEMICN){
                //ERROR
                errorHandler.Print_Error(SEMICOLON_MISSING);
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
                    errorHandler.Print_Error(SEMICOLON_MISSING);//虽然for语句强调了不会出现这个错误
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
                    errorHandler.Print_Error(SEMICOLON_MISSING);
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
                    errorHandler.Print_Error(RPARENT_MISSING);
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
                errorHandler.Print_Error(RPARENT_MISSING);
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
            Block();
            break;
        case SEMICN:
            PRINT_WORD;//PRINT ;
            GET_A_WORD;
            break;
        default:
            // Exp  LVal    只有LVal有赋值符号于Stmt中
            //LVal
            //Exp
            Exp();
            if(WORD_TYPE != SEMICN){
                //ERROR
                //缺少分号
                errorHandler.Print_Error(SEMICOLON_MISSING);
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

int  Parser::FormatString(int printf_line) {

    if (WORD_TYPE == ILLEGAL){
        //ERROR
        //可能还需要注意 格式字符串是否合法？
        errorHandler.error_line = printf_line;
        errorHandler.Print_Error(ILLEGAL_STRING);
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

void Parser::InitVal() { //变量初值
    if(WORD_TYPE == LBRACE){
        PRINT_WORD;//PRINT {
        GET_A_WORD;
        if (WORD_TYPE == RBRACE){
            //允许吗？  int a[1] = {}  先不考虑语义
            PRINT_WORD;//PRINT }
            GET_A_WORD;
        }else{
            InitVal();
            while(WORD_TYPE == COMMA){
                PRINT_WORD;//PRINT ,
                GET_A_WORD;
                InitVal();
            }
            if (WORD_TYPE != RBRACE){
                //ERROR
            }
            PRINT_WORD;//PRINT }
            GET_A_WORD;
        }
    }else{
        Exp();//下放错误
    }

    Print_Grammar_Output("<InitVal>");
}

void Parser::Cond() {
    LOrExp();
    Print_Grammar_Output("<Cond>");
}

void Parser::ForStmt() {
    LVal();
    PRINT_WORD;//PRINT =
    GET_A_WORD;
    Exp();
    Print_Grammar_Output("<ForStmt>");
}

void Parser::UnaryOp() {
    if (WORD_TYPE != PLUS && WORD_TYPE != MINU && WORD_TYPE != NOT){
        //ERROR
    }
    PRINT_WORD;
    GET_A_WORD;
    Print_Grammar_Output("<UnaryOp>");
}

void Parser::RelExp() {
    AddExp();
    if(WORD_TYPE == LSS || WORD_TYPE == GRE || WORD_TYPE == LEQ || WORD_TYPE == GEQ){
        while(WORD_TYPE == LSS || WORD_TYPE == GRE || WORD_TYPE == LEQ || WORD_TYPE == GEQ){
            Print_Grammar_Output("<RelExp>");
            PRINT_WORD;
            GET_A_WORD;
            AddExp();
        }
    }
    Print_Grammar_Output("<RelExp>");
}

void Parser::EqExp() {
    RelExp();
    if(WORD_TYPE == EQL || WORD_TYPE ==NEQ){
        while(WORD_TYPE == EQL || WORD_TYPE ==NEQ){
            Print_Grammar_Output("<EqExp>");
            PRINT_WORD;
            GET_A_WORD;
            RelExp();

        }
    }else{
        //error
    }
    Print_Grammar_Output("<EqExp>");
}

void Parser::LAndExp() {
    EqExp();
    if(WORD_TYPE == AND){
        while(WORD_TYPE == AND){
            Print_Grammar_Output("<LAndExp>");
            PRINT_WORD;
            GET_A_WORD;
            EqExp();
        }
    }else{
        //error
    }
    Print_Grammar_Output("<LAndExp>");
}

void Parser::LOrExp() {
    LAndExp();
    if(WORD_TYPE == OR){
        while(WORD_TYPE == OR){
            Print_Grammar_Output("<LOrExp>");
            PRINT_WORD;
            GET_A_WORD;
            LAndExp();

        }
    }else{
        //error
    }
    Print_Grammar_Output("<LOrExp>");
}
//我爱
//    我爱xjl徐佳琳

