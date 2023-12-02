#pragma once
#include<iostream>
#include<string>
using namespace std;
#include"Node.h"

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

class LinkedList {
public:
    Node* head;

    LinkedList() : head(nullptr) {}

    void insert(const string& varName, const string& varType) {
        Node* newNode = new Node{ varName, varType, nullptr };
        if (!head) {
            head = newNode;
        }
        else {
            Node* current = head;
            while (current->next) {
                current = current->next;
            }
            current->next = newNode;
        }
    }

    void display(string& variableInfo) const {
        Node* current = head;
        while (current) {
            // cout << "Variable: " << current->varName << " (Type: " << current->varType << ")" << endl;
            variableInfo += "Variable: " + current->varName + " (Type: " + current->varType + ") ";
            current = current->next;
        }
    }

    void display2() const {
        Node* current = head;
        while (current) {
             cout << "Variable: " << current->varName << " (Type: " << current->varType << ")" << endl;
            current = current->next;
        }
    }


    ~LinkedList() {
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
    }
};


#endif // !LINKEDLIST_H
