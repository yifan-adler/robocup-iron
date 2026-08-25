/*
 * File: main.cpp
 * Author : ShiQiao Chen(陈世侨)
 * Affiliation: WuHan University of Technology
 */
#include "rdfw.hpp"

int main(int argc, char **argv)
{
    auto rdfw = make_shared<_home::RDFW>();
    rdfw->Init(argc, argv);
    rdfw->Run();
    return 0;
}
