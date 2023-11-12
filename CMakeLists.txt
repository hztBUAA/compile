cmake_minimum_required(VERSION 3.22.1)
project(Lex)

set(CMAKE_CXX_STANDARD 17)

add_executable(Lex main.cpp
        Lexer.cpp
        Lexer.h
        Parser.cpp
        Parser.h
        ErrorHandler.cpp
        ErrorHandler.h
        TableManager.cpp
        TableManager.h
        Semantic.cpp
        Semantic.h
        IntermediateCode.cpp
        IntermediateCode.h
        MipsCode.cpp
        MipsCode.h
        IntermediateCode.h
)
