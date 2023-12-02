#pragma once
#ifndef FUNTIONNODE_H
#define FUNTIONNODE_H
#include<iostream>
#include<string>
#include"Parameter.h"

struct FunctionNode
{
    string funcName;
    string returnType;
    Parameter* parameters;
    FunctionNode* next;
};


#endif // !FUNTIONNODE_H
