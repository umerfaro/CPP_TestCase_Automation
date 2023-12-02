#pragma once
#ifndef CLANGCODE_H
#define CLANGCODE_H
#include<iostream>
#include<string>
#include"FuntionList.h"
#include"CondditionList.h"
#include"LinkedList.h"
#include"Parameter.h"
#include"Node.h"
#include <clang-c/Index.h>


string sourceFilename;


// --- inserts user input variable and their types into a link list ---
bool IsUserInputVariable(const string& varName, const string& sourceCode, LinkedList& userInputList, string vartype) {
    bool isUserInput = sourceCode.find("cin >> " + varName) != string::npos;
    if (isUserInput) {
        // Assuming the type is always a string for simplicity, you can modify it based on your needs
        userInputList.insert(varName, vartype);
    }
    return isUserInput;
}

// --- file analyzing check end ---
// --- extracting condition from text ---
string extractConditionText(CXTranslationUnit tu, CXToken* tokens, unsigned numTokens) {
    string conditionText;

    // Check if the statement is a case statement
    if (numTokens > 2 &&
        clang_getTokenKind(tokens[0]) == CXToken_Keyword &&
        strcmp(clang_getCString(clang_getTokenSpelling(tu, tokens[0])), "case") == 0) {

        // Handle case statements with integer constant
        CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[1]);
        conditionText = clang_getCString(tokenSpelling);
        clang_disposeString(tokenSpelling);
    }
    else {
        // Handle conditions enclosed in parentheses
        unsigned startIdx = 0;
        while (startIdx < numTokens && clang_getCString(clang_getTokenSpelling(tu, tokens[startIdx]))[0] != '(') {
            ++startIdx;
        }

        unsigned endIdx = startIdx + 1;
        while (endIdx < numTokens && clang_getCString(clang_getTokenSpelling(tu, tokens[endIdx]))[0] != ')') {
            ++endIdx;
        }

        if (startIdx < numTokens && endIdx < numTokens) {
            for (unsigned i = startIdx + 1; i < endIdx; ++i) {
                CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[i]);
                const char* tokenStr = clang_getCString(tokenSpelling);
                conditionText += tokenStr;
                clang_disposeString(tokenSpelling);
            }
        }
    }

    return conditionText;
}


// --- visitor data ---
struct VisitorData
{
    LinkedList* userInputList;
    FunctionList* functionList;
    ConditionList* conditionList;
};
// --- visits each line of code and stores into AST ---
CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    if (!clang_Location_isFromMainFile(location)) {
        return CXChildVisit_Continue;
    }
    VisitorData* d = reinterpret_cast<VisitorData*>(client_data);
    ifstream cppFile(sourceFilename);
    stringstream buffer;
    buffer << cppFile.rdbuf();
    string sourceCode = buffer.str();

    CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
    if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {
        CXString functionName = clang_getCursorSpelling(cursor);
        CXType returnType = clang_getCursorResultType(cursor);

        cout << "Function: " << clang_getCString(functionName) << " (Return Type: " << clang_getCString(clang_getTypeSpelling(returnType)) << ")\n";

        Parameter* parameters = nullptr;
        int numArgs = clang_Cursor_getNumArguments(cursor);
        for (int i = 0; i < numArgs; ++i) {
            CXCursor arg = clang_Cursor_getArgument(cursor, i);
            CXString argName = clang_getCursorSpelling(arg);
            CXType argType = clang_getCursorType(arg);

            Parameter* newParam = new Parameter{ clang_getCString(argName), clang_getCString(clang_getTypeSpelling(argType)), nullptr };
            if (!parameters) {
                parameters = newParam;
            }
            else {
                Parameter* current = parameters;
                while (current->next) {
                    current = current->next;
                }
                current->next = newParam;
            }

            clang_disposeString(argName);
        }

        d->functionList->insert(clang_getCString(functionName), clang_getCString(clang_getTypeSpelling(returnType)), parameters);
        clang_disposeString(functionName);

    }
    else if (clang_getCursorKind(cursor) == CXCursor_VarDecl) {
        CXString varName = clang_getCursorSpelling(cursor);
        CXType varType = clang_getCursorType(cursor);
        string varTypeStr = clang_getCString(clang_getTypeSpelling(varType));

        if (IsUserInputVariable(clang_getCString(varName), sourceCode, *(d->userInputList), varTypeStr)) {
            cout << "User input variable: " << clang_getCString(varName) << " (Type: " << clang_getCString(clang_getTypeSpelling(varType)) << ")" << endl;
        }
        else {
            cout << "Variable: " << clang_getCString(varName) << " (Type: " << clang_getCString(clang_getTypeSpelling(varType)) << ")" << endl;
        }

        clang_disposeString(varName);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_ParmDecl) {
        CXString paramName = clang_getCursorSpelling(cursor);
        CXType paramType = clang_getCursorType(cursor);

        cout << "Parameter: " << clang_getCString(paramName) << " (Type: " << clang_getCString(clang_getTypeSpelling(paramType)) << ")" << endl;

        clang_disposeString(paramName);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_IfStmt) {
        CXSourceRange ifStmtRange = clang_getCursorExtent(cursor);

        CXToken* tokens;
        unsigned numTokens;
        clang_tokenize(tu, ifStmtRange, &tokens, &numTokens);

        string conditionText = extractConditionText(tu, tokens, numTokens);
        cout << "If Statement - Condition: (";
        if (!conditionText.empty()) {
            cout << conditionText;
        }
        cout << ") \n";
        d->conditionList->insert("If Statement", conditionText);
        clang_disposeTokens(tu, tokens, numTokens);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_ForStmt) {
        CXSourceRange forStmtRange = clang_getCursorExtent(cursor);

        CXToken* tokens;
        unsigned numTokens;
        clang_tokenize(tu, forStmtRange, &tokens, &numTokens);

        string conditionText = extractConditionText(tu, tokens, numTokens);
        cout << "For loop condition: (";
        if (!conditionText.empty()) {
            cout << conditionText;
        }
        cout << " ) \n";
        d->conditionList->insert("For loop", conditionText);
        clang_disposeTokens(tu, tokens, numTokens);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_WhileStmt) {
        CXSourceRange whileStmtRange = clang_getCursorExtent(cursor);

        CXToken* tokens;
        unsigned numTokens;
        clang_tokenize(tu, whileStmtRange, &tokens, &numTokens);

        string conditionText = extractConditionText(tu, tokens, numTokens);
        cout << "While loop condition: ( ";
        if (!conditionText.empty()) {
            cout << conditionText;
        }
        cout << ") \n";
        d->conditionList->insert("While loop", conditionText);
        clang_disposeTokens(tu, tokens, numTokens);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_SwitchStmt) {
        CXSourceRange switchStmtRange = clang_getCursorExtent(cursor);

        CXToken* tokens;
        unsigned numTokens;
        clang_tokenize(tu, switchStmtRange, &tokens, &numTokens);

        string conditionText = extractConditionText(tu, tokens, numTokens);
        cout << "Switch statement condition: (";
        if (!conditionText.empty()) {
            cout << conditionText;
        }
        cout << ") \n";
        d->conditionList->insert("Switch statement", conditionText);
        clang_disposeTokens(tu, tokens, numTokens);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_CaseStmt) {
        CXSourceRange caseStmtRange = clang_getCursorExtent(cursor);

        CXToken* tokens;
        unsigned numTokens;
        clang_tokenize(tu, caseStmtRange, &tokens, &numTokens);

        // Find the start of the condition
        unsigned startIdx = 0;
        while (startIdx < numTokens && strcmp(clang_getCString(clang_getTokenSpelling(tu, tokens[startIdx])), "case") != 0) {
            ++startIdx;
        }

        // Skip the 'case' keyword
        startIdx += 1;

        // The condition ends with ':'
        unsigned endIdx = startIdx;
        while (endIdx < numTokens && clang_getCString(clang_getTokenSpelling(tu, tokens[endIdx]))[0] != ':') {
            ++endIdx;
        }

        // Extract the condition
        string conditionText;
        if (startIdx < numTokens && endIdx < numTokens) {
            for (unsigned i = startIdx; i < endIdx; ++i) {
                CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[i]);
                const char* tokenStr = clang_getCString(tokenSpelling);
                if (i != startIdx && conditionText.back() != ' ') {
                    conditionText += ' ';
                }
                conditionText += tokenStr;
                clang_disposeString(tokenSpelling);
            }
        }

        cout << "Case statement condition: ";
        if (!conditionText.empty()) {
            cout << conditionText;
        }
        cout << "\n";
        d->conditionList->insert("Case statement", conditionText);
        clang_disposeTokens(tu, tokens, numTokens);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_DoStmt) {
        CXSourceRange doStmtRange = clang_getCursorExtent(cursor);

        CXToken* tokens;
        unsigned numTokens;
        clang_tokenize(tu, doStmtRange, &tokens, &numTokens);

        // Find the start and end of the condition
        unsigned startIdx = 0;
        while (startIdx < numTokens && strcmp(clang_getCString(clang_getTokenSpelling(tu, tokens[startIdx])), "while") != 0) {
            ++startIdx;
        }

        // Skip the 'while' keyword and the opening parenthesis
        startIdx += 2;

        unsigned endIdx = startIdx;
        while (endIdx < numTokens && clang_getCString(clang_getTokenSpelling(tu, tokens[endIdx]))[0] != ')') {
            ++endIdx;
        }

        // Extract the condition
        string conditionText;
        if (startIdx < numTokens && endIdx < numTokens) {
            for (unsigned i = startIdx; i < endIdx; ++i) {
                CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[i]);
                const char* tokenStr = clang_getCString(tokenSpelling);
                if (i != startIdx && conditionText.back() != ' ') {
                    conditionText += ' ';
                }
                conditionText += tokenStr;
                clang_disposeString(tokenSpelling);
            }
        }

        cout << "Do-While loop condition: ( ";
        if (!conditionText.empty()) {
            cout << conditionText;
        }
        cout << ") \n";
        d->conditionList->insert("Do-While loop", conditionText);
        clang_disposeTokens(tu, tokens, numTokens);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_ReturnStmt) {
        CXSourceRange returnStmtRange = clang_getCursorExtent(cursor);

        CXToken* tokens;
        unsigned numTokens;
        clang_tokenize(tu, returnStmtRange, &tokens, &numTokens);

        string returnStmtText;

        // Skip the "return" keyword
        if (numTokens > 0) {
            for (unsigned i = 1; i < numTokens; ++i) {
                CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[i]);
                const char* tokenStr = clang_getCString(tokenSpelling);

                returnStmtText += tokenStr;

                clang_disposeString(tokenSpelling);
            }
        }

        cout << "Return statement expression: " << returnStmtText << "\n";
        //  d->conditionList->insert("Return statement expression", returnStmtText);

        clang_disposeTokens(tu, tokens, numTokens);
    }
    else if (clang_getCursorKind(cursor) == CXCursor_DefaultStmt) {
        cout << "Default statement\n";
        d->conditionList->insert("Default statement", "");
    }
    return CXChildVisit_Recurse;
}


#endif // !CLANGCODE_H
