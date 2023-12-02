#pragma once
#include<iostream>
#include<string>
using namespace std;
#ifndef PARAMETER_H
#define PARAMETER_H

struct Parameter
{
    string paramName;
    string paramType;
    Parameter* next;
};


#endif // !PARAMETER_H
