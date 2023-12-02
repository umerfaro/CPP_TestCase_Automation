#pragma once
#include<iostream>
#include<string>
using namespace std;
#ifndef NODE_H
#define NODE_H

struct Node {
    string varName;
    string varType;
    Node* next;
};


#endif // !NODE_H

