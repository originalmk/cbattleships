cmake_minimum_required(VERSION 3.20)
project(CBattleShips C)

set(CMAKE_C_STANDARD 11)

add_executable(CBattleShips main.c vectors.h vectors.c)
