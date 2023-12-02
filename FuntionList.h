#pragma once
#ifndef FUNTIONLIST_H
#define FUNTIONLIST_H
#include<iostream>
#include<string>
#include"FuntionNode.h"
using namespace std;

class FunctionList {
public:
    FunctionNode* head;

    FunctionList() : head(nullptr) {}

    void insert(const string& funcName, const string& returnType, Parameter* parameters) {
        FunctionNode* newNode = new FunctionNode{ funcName, returnType, parameters, nullptr };
        if (!head) {
            head = newNode;
        }
        else {
            FunctionNode* current = head;
            while (current->next) {
                current = current->next;
            }
            current->next = newNode;
        }
    }

    void display(string & functionInfo) const {
        FunctionNode* current = head;
        while (current) {
            //cout << "Function: " << current->funcName << " (Return Type: " << current->returnType << ")";
            functionInfo += "Function: " + current->funcName + " (Return Type: " + current->returnType + ")";
            Parameter* param = current->parameters;
            while (param) {
                // cout << " Parameter: " << param->paramName << " (Type: " << param->paramType << ") ";
                functionInfo += " Parameter: " + param->paramName + " (Type: " + param->paramType + ") ";
                param = param->next;
            }
            functionInfo += " ";
            //cout << endl;
            current = current->next;
        }
    }

    void display2() const {
        FunctionNode* current = head;
        while (current) {
            cout << "Function: " << current->funcName << " (Return Type: " << current->returnType << ")";
            Parameter* param = current->parameters;
            while (param) {
                cout << " Parameter: " << param->paramName << " (Type: " << param->paramType << ") ";
                
                param = param->next;
            }
            cout << endl;
            current = current->next;
        }
    }

    ~FunctionList() {
        FunctionNode* current = head;
        while (current) {
            FunctionNode* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
    }
};


#endif // !FUNTIONLIST_H
