//
// Created by hzt on 2023/9/25.
//

#include "Parser.h"
#include "Lexer.h"
#include <iostream>
using namespace std;

#define WORD_TYPE lexer.token_type
#define GET_A_WORD lexer.nextSymbol()
#define PEEK_A_LETTER lexer.sourceFile.peek()
#define PRINT_WORD lexer.printOutput()


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
   VarDef();//有错误就下放到下一层 按照文法编写
   while(WORD_TYPE == COMMA){
       PRINT_WORD;//PRINT ,
       GET_A_WORD;
       VarDef();
   }
    if (WORD_TYPE != SEMICN){
        //Error;  缺少分号
    }
    PRINT_WORD;//PRINT ;
    Print_Grammar_Output("<VarDecl>");
    GET_A_WORD;//POINT TO NEXT
}

void Parser::VarDef() {
    /**
     * WORD_TYPE正指向 IDENTFR
     */
    if (WORD_TYPE != IDENFR){
        //Error
    }
    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    while(WORD_TYPE == LBRACK){
        PRINT_WORD;//PRINT [
        GET_A_WORD;
        ConstExp();
        if (WORD_TYPE != RBRACK){
            //Error  缺少右中括号
        }
        PRINT_WORD;//PRINT ]
        GET_A_WORD;
        //需要预读取查看
    }
    if (WORD_TYPE == ASSIGN){
        PRINT_WORD;
        GET_A_WORD;
        InitVal();
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
    ConstDef();// point to the ident and into ConstDef()
    while(WORD_TYPE == COMMA){
        PRINT_WORD;//PRINT COMMA
        GET_A_WORD;
        ConstDef();
    }
    if (WORD_TYPE != SEMICN){
        //Error
    }
    PRINT_WORD;//PRINT ;
    Print_Grammar_Output("<ConstDecl>");
    GET_A_WORD;//point to next word
}

void Parser::ConstDef() {
    if (WORD_TYPE != IDENFR){
        //Error
    }
    PRINT_WORD;
    GET_A_WORD;
    while(WORD_TYPE == LBRACK){
        PRINT_WORD;
        GET_A_WORD;
        ConstExp();
        if (WORD_TYPE == RBRACK){
            PRINT_WORD;
            GET_A_WORD;
        }else{
            //Error
        }
    }
    if (WORD_TYPE != ASSIGN){
        //Error
    }else{
        PRINT_WORD;
        GET_A_WORD;
        ConstInitVal();
    }
    //已经指向下一个word
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
        //lexer.ch = PEEK_A_LETTER;
        while(isspace(lexer.ch)){
            lexer.ch = (char )lexer.sourceFile.get();
        }
        if(lexer.ch == '('){
            //进入FuncRParams
            PRINT_WORD;//PRINT IDENT
            GET_A_WORD;//算上 （
            PRINT_WORD;//PRINT (
            GET_A_WORD;//指向FuncRParams
            //可以没有参数
            if(WORD_TYPE == RPARENT){
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }else{
                FuncRParams();
                if (WORD_TYPE != RPARENT){
                    //Error  缺少右括号）
                }
                PRINT_WORD;//PRINT )
                GET_A_WORD;//point to next WORD
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
        }
        PRINT_WORD;//PRINT )
        GET_A_WORD;
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
    GET_A_WORD;
    while(WORD_TYPE == LBRACK){
        PRINT_WORD;//PRINT [
        GET_A_WORD;
        Exp();
        if (WORD_TYPE != RBRACK){
            //Error  缺少有中括号]
        }
        PRINT_WORD;//PRINT ]
        GET_A_WORD;//POINT TO NEXT WORD
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
    GET_A_WORD;
    PRINT_WORD;
    GET_A_WORD;
    PRINT_WORD;
    GET_A_WORD;
    Block();
    Print_Grammar_Output("<MainFuncDef>");
}

void Parser::FuncRParams() {
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
    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    PRINT_WORD;//PRINT (
    GET_A_WORD;
    if(WORD_TYPE == RPARENT){
        //no FUNCFParams
        PRINT_WORD;
        GET_A_WORD;
        Block();
    }else{
        if(WORD_TYPE == INTTK){
            FuncFParams();
            //指向）
            if(WORD_TYPE != RPARENT){
                //ERROR
            }
            PRINT_WORD;//PRINT )
            GET_A_WORD;
            Block();
        }else{
            //Error
        }
    }
    Print_Grammar_Output("<FuncDef>");
}

void Parser::FuncType() {
    Print_Grammar_Output("<FuncType>");
}

void Parser::FuncFParams() {
    FuncFParam();
    while(WORD_TYPE == COMMA){
        PRINT_WORD;//PRINT ,
        GET_A_WORD;
        FuncFParam();
    }
    Print_Grammar_Output("<FuncFParams>");
}

void Parser::FuncFParam() {
    if(WORD_TYPE != INTTK){
        //error
    }
    PRINT_WORD;//PRINT INT
    GET_A_WORD;
    PRINT_WORD;//PRINT IDENT
    GET_A_WORD;
    if(WORD_TYPE != LBRACK){
        //认为就是普通变量  这里不会有error
    }else{
        PRINT_WORD;//PRINT [
        GET_A_WORD;
        if(WORD_TYPE != RBRACK){
            //ERROR  缺少右中括号
        }
        PRINT_WORD;//PRINT ]   一维数组变量
        GET_A_WORD;
        if(WORD_TYPE != LBRACK){
            //认为就是一维数组变量  这里不会有error
        }else{
            //二维数组变量
            PRINT_WORD;//PRINT [
            GET_A_WORD;
            if(WORD_TYPE == RBRACK){
                //ERROR  二维数组 最后一维需要常数
            }
            ConstExp();
            if(WORD_TYPE != RBRACK){
                //ERROR  缺少右中括号
            }
            PRINT_WORD;//PRINT ]
            GET_A_WORD;//指向下一个
        }
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
        //空语句块
        PRINT_WORD;//PRINT }
        GET_A_WORD;
    }else{
        while(WORD_TYPE != RBRACE){
            BlockItem();//这一级不处理错误
        }
        if(WORD_TYPE != RBRACE){
            //ERROR
        }
        PRINT_WORD;
        GET_A_WORD;
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
    switch (WORD_TYPE) {
        case PRINTFTK:
            PRINT_WORD;//PRINT PRINTF
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            FormatString();
            while(WORD_TYPE == COMMA){
                PRINT_WORD;//PRINT ,
                GET_A_WORD;
                Exp();
            }
            if(WORD_TYPE != RPARENT){
                //缺少右圆括号
            }
            PRINT_WORD;//PRINT )
            GET_A_WORD;
            if(WORD_TYPE != SEMICN){
                //缺少分号
            }
            PRINT_WORD;//PRINT ;
            GET_A_WORD;
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
                }
                PRINT_WORD;
                GET_A_WORD;
            }
            break;
        case BREAKTK:
        case CONTINUETK:
            PRINT_WORD;//PRINT CONTINUE
            GET_A_WORD;
            if (WORD_TYPE != SEMICN){
                //ERROR
            }
            PRINT_WORD;//PRINT ;
            GET_A_WORD;
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
                }
                PRINT_WORD;//PRINT ;
                GET_A_WORD;
            }
            if (WORD_TYPE == SEMICN){
                PRINT_WORD;//PRITN ;
                GET_A_WORD;
            }else{
                Cond();
                if (WORD_TYPE != SEMICN){
                    //ERROR
                }
                PRINT_WORD;//PRINT ;
                GET_A_WORD;
            }
            if(WORD_TYPE == RPARENT){
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }else{
                ForStmt();
                if (WORD_TYPE != RPARENT){
                    //error
                }
                PRINT_WORD;//PRINT )
                GET_A_WORD;
            }
            Stmt();
            break;
        case IFTK:
            PRINT_WORD;//PRINT IF
            GET_A_WORD;
            PRINT_WORD;//PRINT (
            GET_A_WORD;
            Cond();
            if (WORD_TYPE != RPARENT){
                //ERROR
            }
            PRINT_WORD;//PRINT )
            GET_A_WORD;
            Stmt();
            if (WORD_TYPE == ELSETK){
                PRINT_WORD;//PRINT ELSE
                GET_A_WORD;
                Stmt();
            }
            break;
        case LBRACE:
            //from { start not print
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
            }
            PRINT_WORD;
            GET_A_WORD;
            if (isLValInStmt){
                isLValInStmt = false;
            }//重置
            break;
    }
    Print_Grammar_Output("<Stmt>");
}

void Parser::FormatString() {

    if (WORD_TYPE != STRCON){
        //ERROR
        //可能还需要注意 格式字符串是否合法？
    }
    PRINT_WORD;//PRINT StrCON
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

