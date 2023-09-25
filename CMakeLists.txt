cmake_minimum_required(VERSION 3.26)
project(Lex)

set(CMAKE_CXX_STANDARD 17)

add_executable(Lex main.cpp
        Lexer.cpp
        Lexer.h)
