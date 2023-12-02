#pragma once
#ifndef CONDITIONLIST_H
#define CONDITIONLIST_H
#include<iostream>
#include<string>
using namespace std;

// --- ConditionNode ---
struct ConditionNode
{
    string conditionName;
    string conditionText;
    ConditionNode* next;
};
// --- Condition List ---
class ConditionList {
public:
    ConditionNode* head;

    ConditionList() : head(nullptr) {}

    void insert(const string& conditionName, const string& conditionText) {
        ConditionNode* newNode = new ConditionNode{ conditionName, conditionText, nullptr };
        if (!head) {
            head = newNode;
        }
        else {
            ConditionNode* current = head;
            while (current->next) {
                current = current->next;
            }
            current->next = newNode;
        }
    }

    void display(string & conditionInfo) const {
        ConditionNode* current = head;
        while (current) {
            if (!current->conditionText.empty()) {
                conditionInfo += "Condition: " + current->conditionName + " (Text: " + current->conditionText + ") ";
                //cout << "Condition: " << current->conditionName << " (Text: " << current->conditionText << ")" << endl;
            }
            else
            {
                conditionInfo += "Condition: " + current->conditionName + " ";
                //cout << "Condition: " << current->conditionName << endl;
            }
            current = current->next;
        }
    }

    void display2() const {
        ConditionNode* current = head;
        while (current) {
            if (!current->conditionText.empty()) {
                
                cout << "Condition: " << current->conditionName << " (Text: " << current->conditionText << ")" << endl;
            }
            else
            {
                
                cout << "Condition: " << current->conditionName << endl;
            }
            current = current->next;
        }
    }



    ~ConditionList() {
        ConditionNode* current = head;
        while (current) {
            ConditionNode* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
    }
};




#endif // !CONDITIONLIST_H
