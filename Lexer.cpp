//
// Created by hzt on 2023/9/19.
//

#include "Lexer.h"
#include <iostream>
#include<cctype>

static string str;//token.symbol指向它

//对外（main、Parser）的构造函数
Lexer &Lexer::initLexer(ifstream &sourceFile,ofstream & outputFile) {
    static Lexer lexer(sourceFile,outputFile);//static  可以导致单例模式？
    return lexer;
}
//真正的构造器  被上面的调用
Lexer::Lexer(ifstream &sourceFile,ofstream & outputFile):sourceFile(sourceFile), outputFile(outputFile) {
    //初始化ch为读进来的待处理的第一个字符 get本来是对int类型的额
    ch = (char )sourceFile.get();
    line = 1;
    line_lastWord = 1;
    count_flag = false;
    enableOutput = false;
    wordCategory =  //直接这样生成？
            // 单词名称和类别码的映射  仅存放保留字
            {
                    //{"Ident", "IDENFR"},
//                    {"123", INTCON},
//                    {"##", STRCON},
                    {"main", MAINTK},
                    {"const", CONSTTK},
                    {"int", INTTK},
                    {"break", BREAKTK},
                    {"continue", CONTINUETK},
                    {"else", ELSETK},
                    {"void", VOIDTK},
//                    {"!", NOT},
//                    {"*",MULT},
//                    {"=", ASSIGN},
//                    {"&&", AND},
//                    {"/", DIV},
//                    {";", SEMICN},
//                    {"||", OR},
//                    {"%", MOD},
//                    {",", COMMA},
                    {"for", FORTK},
//                    {"<", LSS},
//                    {"(", LPARENT},
//                    {")", RPARENT},
//                    {"<=", LEQ},
//                    {"[", LBRACK},
//                    {"]", RBRACK},
//                    {"+", PLUS},
//                    {"==", EQL},
//                    {"{", LBRACE},
//                    {"-", MINU},
//                    {"!=", NEQ},
//                    {"}", RBRACE},
                    {"if", IFTK},
//                    {">", GRE},
                    {"getint", GETINTTK},
                    {"return", RETURNTK},
//                    {">=", GEQ},
                    {"printf",PRINTFTK}
            };
}

void Lexer::nextSymbol() {
    if (token_type != NOTE){
        line_lastWord = line;
    }
    while(isspace(ch)){
//        if (count_flag){ // 如何做到延迟加法
//            line_lastWord++;
//            count_flag = false;
//        }
        if (ch == '\n'){ //除了字符串里面的换行符
            line++;
//            count_flag = true;
        }
        ch = (char )sourceFile.get();
    }
    //digit
    if (isdigit(ch)){
        token.number = ch - '0';
        ch = (char )sourceFile.get();
        while(isdigit(ch)){
            token.number  = token.number*10+ch-'0';
            ch = (char )sourceFile.get();
        }
        token_type = INTCON;//ch->下一个待处理的字符
    }else if (isalpha(ch)||ch == '_'){ //非数字但是字母开头 要么标识符  要么保留字
        str.clear();
        str+=ch;
        ch = (char )sourceFile.get();
        while(isalnum(ch)||ch == '_'){
            str+=ch;//?str.append(1,ch)
            ch = (char)sourceFile.get();
        }
        token.symbol = &str;//到底为何必须在构造时只能用指针symbol呢？
        if (wordCategory.find(str)==wordCategory.end()){
            //不是保留字
            token_type = IDENFR;
        }else{
            token_type = wordCategory[*token.symbol];
        }
        //string type_str = wordCategory[*(token.symbol)];
    }else{ // 还有其他字符
        str.clear();
        str+=ch;//将待处理的字符先行放进去
        switch (ch) {
            case '!':
                ch = (char )sourceFile.peek();
                if (ch == '='){
                    //!=
                    str+=ch;
                    ch =(char ) sourceFile.get();  //此时还是原来peek的
                    token_type = NEQ;
                }else{
                    //!
                    token_type = NOT;
                }
                break;
            case '&':
                //&&
                ch = (char )sourceFile.get();//认为应该又是&
                if (ch == '&'){
                    str+=ch;
                    token_type = AND;
                }else{
                    //不合法
                    token_type = ILLEGAL;
                }
                break;
            case '|':
                //||
                ch = (char )sourceFile.get();//认为应该又是|
                if (ch == '|'){
                    str+=ch;
                    token_type = OR;
                }else{
                    //不合法
                    token_type = ILLEGAL;
                }
                break;
             case '+':
                 token_type = PLUS;
                break;
             case '-':
                 token_type = MINU;
                break;
             case '*':
                 token_type = MULT;
                break;
             case '/':
                 //注释  单行 多行 除号
                 ch = (char )sourceFile.peek();
                if (ch == '*'){
                    ch = (char)sourceFile.get();//真正算上'*'
                    while(true){
                        while((ch = (char )sourceFile.peek())!= '*'){
                            ch = (char )sourceFile.get();
                            if(ch == '\n'){
                                line ++;
                            }
                        }
                        ch = (char )sourceFile.get();//真正算上*
                        ch = (char )sourceFile.peek();
                        if (ch == '/'){
                            //先真正get到‘/’
                            ch = (char )sourceFile.get();//真正算上'/'
                            token_type = NOTE;
                            break;
                        }
                    }
                }else if (ch == '/'){
                    ch = (char)sourceFile.get();//真正算上'/'
                    while((ch = (char )sourceFile.peek())!= '\n'){
                        ch = (char )sourceFile.get();
                    }
                    //退出while时  ch指向这一行注释的最后一个待处理的字符   然后最后switch块再get一个 使得ch指向\n同时输入流去掉\n 下一次调用nextSymbol函数 会首先去掉它  这里会统一处理空白字符
                    token_type = NOTE;

                }else{
                    token_type = DIV;
                }
                break;
             case '%':
                 token_type = MOD;
                break;
             case '<':
                 ch = (char )sourceFile.peek();
                if (ch == '='){
                    str+=ch;
                    ch = (char )sourceFile.get();
                    token_type = LEQ;
                }
                else{
                    //maybe 不合法
                    token_type = LSS;
                }
                break;
             case '>':
                 //>=
                 ch = (char )sourceFile.peek();
                if (ch == '='){
                    str+=ch;
                    ch = (char )sourceFile.get();
                    token_type = GEQ;
                }
                else{
                    //maybe 不合法
                    token_type = GRE;
                }
                break;
            case '=':
                ch = (char )sourceFile.peek();
                if (ch == '='){
                    str+=ch;
                    ch = (char )sourceFile.get();
                    token_type = EQL;
                }
                else{
                    //maybe 不合法  错误处理？
                    token_type = ASSIGN;
                }
                break;
            case ';':
                token_type =SEMICN;
                break;
            case ',':
                token_type = COMMA;
                break;
            case '(':
                token_type = LPARENT;
                break;
            case ')':
                token_type = RPARENT;
                break;
            case '[':
                token_type = LBRACK;
                break;
            case ']':
                token_type = RBRACK;
                break;
            case '{':
                token_type = LBRACE;
                break;
            case '}':
                token_type = RBRACE;
                break;
            case '"':
                ch = (char )sourceFile.get();
                //调用nextString ch已经指向字符串的第一个字符 且输入流已经去除了第一个字符"
                nextString();//这个函数将token设置成了字符串形式   此时ch指向末尾的“  且输入流已经没有”
                break;
            case EOF:
                token_type = FINISH;
                break;
            default:
                token_type = ILLEGAL;
                break;
        }
        token.symbol = &str;
        ch  =(char )sourceFile.get();//ok
        //应该自觉过滤NOTE
        while(token_type == NOTE){
            token.symbol = &str;
//            ch  =(char )sourceFile.get();//ok
            nextSymbol();
        }
    }
}
/**
 * 文法中明确说明  没有‘abc’
 */
//void Lexer::nextChar() {
//    while(isspace(ch))
//        sourceFile.get();
//    str.clear();
//    str.append(1,ch);
//
//}


/**
 * 存在于printf（""）
 */
void Lexer::nextString() {
    printf_format_count = 0;
    str.clear();
    //"  : 34
    while(ch == 32||ch == 33|| (ch >=40 && ch <= 126) || ch == '%'){ //包括换行符
        str.append(1,ch);
        if(ch == 92 && (char)sourceFile.peek() !='n' || ch == '%'&&(char)sourceFile.peek() !='d'){ //ch == \   \n
            token_type = ILLEGAL;
            while(ch != '"'){
                ch = (char )sourceFile.get();
            }
            return;
        }
        if((char)sourceFile.peek() =='d' && ch == '%'){
            printf_format_count++;
        }
        ch = (char )sourceFile.get();//强制转换即可  最后反正都是英文
    }
    //留出错误处理的formatString格式
    if (ch == '"'){
        //"满足格式的字符串"
        token_type = STRCON;
        token.symbol = & str;
    }else{
        //ILLEGAL
        //"中间出现不满足文法的格式字符串"
        token_type = ILLEGAL;
        while(ch != '"'){
            ch = (char )sourceFile.get();
        }
    }
}

void Lexer::analyze() {
    nextSymbol();
    while(token_type != FINISH){
        if (enableOutput&&token_type!=NOTE&&token_type!=ILLEGAL){
            printOutput();
        }
        nextSymbol();
    }
}

void Lexer::printOutput() {
    if (enableOutput){
        outputFile<<ENUM_TO_STRING(token_type)<<" ";
        if (token_type == INTCON){
            outputFile<<token.number<<endl;
        }else{
            if (token_type == STRCON){
                outputFile<<"\""+*token.symbol+"\""<<endl;
            }else{
                outputFile<<*token.symbol<<endl;//要加*号吗？
            }
        }
    }



//    cout<<ENUM_TO_STRING(token_type)<<" ";
//    if (token_type == INTCON){
//        cout<<token.number<<endl;
//    }else{
//        if (token_type == STRCON){
//            cout<<"\""+*token.symbol+"\""<<endl;
//        }else{
//            cout<<*token.symbol<<endl;//要加*号吗？
//        }
//    }
}


