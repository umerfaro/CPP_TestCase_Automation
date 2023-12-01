#include <clang-c/Index.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <regex>
#include <limits> // for finding bounday values 
#include"HTTPRequest.h"
#include <nlohmann/json.hpp>

using namespace std;

// --- Linked List ---
struct Node {
    string varName;
    string varType;
    Node* next;
};

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

    void display() {
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
// struct for parameter
struct Parameter
{
    string paramName;
    string paramType;
    Parameter* next;
};
// node for function
struct FunctionNode
{
    string funcName;
    string returnType;
    Parameter* parameters;
    FunctionNode* next;
};
// --- list for function ---
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

    void display() {
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

// --- checks for analyzing ---
bool IsComment(const string& line) {
    return regex_match(line, regex("^[ \\t]*//.*")) || regex_match(line, regex("^[ \\t]*/\\*.*\\*/[ \\t]*"));
}

bool IsPreprocessorDirective(const string& line) {
    return regex_match(line, regex("^[ \\t]#.*"));
}

bool IsUsingNamespaceStd(const string& line) {
    return regex_search(line, std::regex("using[ \\t]+namespace[ \\t]+std;"));
}

int CountLinesInFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Failed to open file" << endl;
        return -1;
    }

    int lineCount = 0;
    string line;
    while (getline(file, line)) {
        lineCount++;
    }

    file.close();
    return lineCount;
}

void AnalyzeCode(const char* cpp_filename) {
    const char* text_filename = "Parsed_code.txt";

    ifstream cpp_file(cpp_filename);
    if (!cpp_file.is_open()) {
        cerr << "Error: Failed to open source file" << std::endl;
        return;
    }

    ofstream text_file(text_filename);
    if (!text_file.is_open()) {
        cerr << "Error: Failed to open text file" << std::endl;
        return;
    }

    string line;
    bool inside_multiline_comment = false;

    while (getline(cpp_file, line)) {
        line = regex_replace(line, regex("^[ \\t]+|[ \\t]+$"), "");

        if (line.empty() || IsUsingNamespaceStd(line) || IsComment(line) || IsPreprocessorDirective(line)) {
            continue;
        }

        if (!inside_multiline_comment) {
            if (line.find("/*") != string::npos) {
                inside_multiline_comment = true;
            }
            else {
                text_file << line << '\n';
            }
        }

        if (inside_multiline_comment && line.find("*/") != string::npos) {
            inside_multiline_comment = false;
        }
    }

    cpp_file.close();
    text_file.close();

    cout << "C++ source file parsed and stored in: " << text_filename << endl;

    int lines = CountLinesInFile(text_filename);

    if (lines >= 0) {
        cout << "Total lines in " << text_filename << ": " << lines << endl;
    }
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

    void display() {
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
// --- bounday values check ---
//void findBoundaryValues(const LinkedList& userInputList) {
//    Node* current = userInputList.head;
//
//    while (current) {
//        cout << "Variable: " << current->varName << " (Type: " << current->varType << ")";
//
//        if (current->varType == "int") {
//            cout << " Boundary Values: [" << -numeric_limits<int>::min() << ", " << numeric_limits<int>::max() << "]" << endl;
//        }
//        else if (current->varType == "float") {
//            cout << " Boundary Values: [" << -numeric_limits<float>::max() << ", " << numeric_limits<float>::max() << "]" << endl;
//        }
//        else if (current->varType == "double") {
//            cout << " Boundary Values: [" << -numeric_limits<double>::max() << ", " << numeric_limits<double>::max() << "]" << endl;
//        }
//        else if (current->varType == "char") {
//            cout << " Boundary Values: [" << 0 << ", " << 255 << "]" << endl;
//        }
//        else {
//            cout << " Unknown variable type : " << current->varType << endl;
//        }
//        current = current->next;
//    }
//}
//// --- main function ---
//int main() {
//    CXIndex index = clang_createIndex(0, 0);
//    const char* source_filename = "question2.cpp";
//    const char* command_line_args[] = { "-std=c++14" };
//    sourceFilename = "question2.cpp";
//    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, source_filename, sizeof(command_line_args) / sizeof(command_line_args[0]), command_line_args, 0, 0);
//
//    if (!tu) {
//        printf("Error: Failed to create translation unit\n");
//        return 1;
//    }
//
//    LinkedList userInputList;
//    FunctionList functionList;
//    ConditionList conditionList;
//    VisitorData data = { &userInputList, &functionList , &conditionList };
//    clang_visitChildren(clang_getTranslationUnitCursor(tu), visitor, &data);
//    // --- linked list display ---
//    cout << "............................................. \n";
//    cout << "User Input Variables through linked list: " << endl;
//    userInputList.display();
//    cout << "............................................. \n";
//    cout << "Functions through linked list: " << endl;
//    functionList.display();
//    cout << "............................................. \n";
//    cout << "Conditions through linked list: " << endl;
//    conditionList.display();
//    cout << "............................................. \n";
//    // --- boundary values display ---
//    cout << "Finding Boundary Values: \n";
//    findBoundaryValues(userInputList);
//    clang_disposeTranslationUnit(tu);
//    clang_disposeIndex(index);
//
//    return 0;
//}

int main()
{
    // Replace "YOUR_OPENAI_API_KEY" and "DUMMY_URL" with your actual OpenAI API key and URL
    std::string apiKey = "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";
    std::string apiUrl = "https://api.openai.com/v1/chat/completions";

    // Create an instance of HttpRequest
    HttpRequest httpRequest(apiKey, apiUrl);

    // Define the JSON payload with messages
    std::string requestData = R"(
    {
        "model": "gpt-3.5-turbo",
        "messages": [
            {"role": "user", "content": "How are you?"}
        ]
    }
    )";

    // Make the HTTP request
    httpRequest.post(requestData);

    // Display the response
    //std::cout << "Response:\n" << httpRequest.getResponse() << std::endl;
    httpRequest.extractContent(httpRequest.getResponse());

    return 0;
}
