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
#include"Node.h"
#include"LinkedList.h"
#include"Parameter.h"
#include"FuntionNode.h"
#include"FuntionList.h"
#include"CondditionList.h"
#include"ClangCode.h"
#include"CodeAnalyser.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

string variableInfo;
string functionInfo;
string conditionInfo;









string generatePrompt(const LinkedList& userInputList, const FunctionList& functionList, const ConditionList& conditionList) {
    std::stringstream prompt;

    // Scenario description
    prompt << "Given the following scenario:";

    // Display variables
    prompt << "Variables:";
    userInputList.display(variableInfo);
    prompt << variableInfo;
    prompt << "";

    // Display functions
    prompt << "Functions:";
    functionList.display(functionInfo);
    prompt << functionInfo;
    prompt << "";

    // Display conditions
    prompt << "Conditions:";
    conditionList.display(conditionInfo);
    prompt << conditionInfo;
    prompt << "";

    // Test case generation
    prompt << ".Generate test cases in the following format:";

    // Iterate through user input variables and create test cases
    Node* current = userInputList.head;
    int testCaseNumber = 1;

    while (current) {
        prompt << "Test Case " << testCaseNumber << ":";
        prompt << current->varName << " = [value], " << current->next->varName << " = [value]";
        current = current->next->next;  // Assuming pairs of variables (name, type)
        testCaseNumber++;
    }

    prompt << ".Provide the minimum set of test cases covering all possible scenarios.";

    return prompt.str();
}

struct TestCase {
    double num1;
    double num2;
};


std::vector<TestCase> parseTestCases(const std::string& response) {
    // Create a regular expression pattern to match test case lines
    std::regex testCasePattern(R"(Test Case \d+:\nnum1 = ([^,]+), num2 = ([^\n]+))");

    // Create a vector to store test cases
    std::vector<TestCase> testCases;

    // Use regex iterator to find test case lines in the response
    auto testCaseIt = std::sregex_iterator(response.begin(), response.end(), testCasePattern);
    auto testCaseEnd = std::sregex_iterator();

    while (testCaseIt != testCaseEnd) {
        TestCase testCase;
        testCase.num1 = std::stod((*testCaseIt)[1].str());
        testCase.num2 = std::stod((*testCaseIt)[2].str());
        testCases.push_back(testCase);
        ++testCaseIt;
    }

    return testCases;
}



void mapTestCasesToCode(const std::vector<TestCase>& testCases, const LinkedList& userInputList, const FunctionList& functionList) {
    std::cout << "Mapping Test Cases to Code Components:\n";

    // Iterate through test cases
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << i + 1 << ":\n";
        std::cout << "num1 = " << testCases[i].num1 << ", num2 = " << testCases[i].num2 << "\n";

        // Map test case to variables
        Node* currentVariable = userInputList.head;
        while (currentVariable) {
            std::cout << "Variable: " << currentVariable->varName << " = " << currentVariable->varType << " -> Value: ";
            if (currentVariable->varType == "double") {
                std::cout << (currentVariable->varName == "num1" ? testCases[i].num1 : testCases[i].num2);
            }
            std::cout << "\n";
            currentVariable = currentVariable->next;
        }

        // Map test case to functions
        FunctionNode* currentFunction = functionList.head;
        while (currentFunction) {
            std::cout << "Function: " << currentFunction->funcName << "\n";
            // You can add more logic here to map test cases to specific functions if needed
            currentFunction = currentFunction->next;
        }

        std::cout << "--------------------------------\n";
    }
}

/////////////////////////////////////////////

int executeCommand(const std::string& command) {
#ifdef _WIN32
    return system(command.c_str());
#else
    // On Unix-like systems, use fork and exec
    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Error: Fork failed.\n";
        std::exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        std::cerr << "Error: Exec failed.\n";
        std::exit(EXIT_FAILURE);
    }
    else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
#endif
}

/// <summary>
/// instrumentCodeForCoverage
/// </summary>
/// <param name="sourceFile"></param>
/// <param name="outputExecutable"></param>
void instrumentCodeForCoverage(const std::string& sourceFile, const std::string& outputExecutable) {
    // Compile the code with GCC and coverage options
    std::string compileCommand = "D:\\CodeBlocks\\MinGW\\bin\\g++ -fprofile-arcs -ftest-coverage -o " + outputExecutable + " " + sourceFile;
    if (executeCommand(compileCommand) != 0) {
        std::cerr << "Error: Compilation failed.\n";
        std::exit(EXIT_FAILURE);
    }

    // Run the generated executable to generate coverage data
    std::string runCommand = outputExecutable;
    if (executeCommand(runCommand) != 0) {
        std::cerr << "Error: Execution failed.\n";
        std::exit(EXIT_FAILURE);
    }

    // Use gcov to generate coverage reports
    std::string gcovCommand = "gcov " + sourceFile;
    if (executeCommand(gcovCommand) != 0) {
        std::cerr << "Error: Gcov command failed.\n";
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Code instrumentation and coverage generation complete.\n";
}




int main() {
    CXIndex index = clang_createIndex(0, 0);
    const char* source_filename = "question2.cpp";
    const char* command_line_args[] = { "-std=c++14" };
    sourceFilename = "question2.cpp";
    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, source_filename, sizeof(command_line_args) / sizeof(command_line_args[0]), command_line_args, 0, 0);

    if (!tu) {
        printf("Error: Failed to create translation unit\n");
        return 1;
    }

    LinkedList userInputList;
    FunctionList functionList;
    ConditionList conditionList;
    VisitorData data = { &userInputList, &functionList , &conditionList };
    clang_visitChildren(clang_getTranslationUnitCursor(tu), visitor, &data);
    // --- linked list display ---
    cout << "............................................. \n";
    cout << "User Input Variables through linked list: " << endl;
    userInputList.display2();
    cout << variableInfo;
    cout << "............................................. \n";
    cout << "Functions through linked list: " << endl;
    functionList.display2();
    cout << functionInfo;
    cout << "............................................. \n";
    cout << "Conditions through linked list: " << endl;
    conditionList.display2();
    cout<<conditionInfo;
    cout << "............................................. \n";
    // --- boundary values display ---
    cout << "Finding Boundary Values: \n";

    string prompt = generatePrompt(userInputList, functionList, conditionList);
    std::cout << "Prompt:\n" << prompt << std::endl;

    std::string apiKey = "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";
    std::string apiUrl = "https://api.openai.com/v1/chat/completions";

    // Create an instance of HttpRequest
    HttpRequest httpRequest(apiKey, apiUrl);

   
    std::string requestData = R"(
    {
        "model": "gpt-3.5-turbo",
        "messages": [
            {
                "role": "user",
                "content": ")" + prompt + R"("
            }
        ]
    }
    )";

    // Make the HTTP request
    httpRequest.post(requestData);


    // Display the response
    //std::cout << "Response:\n" << httpRequest.getResponse() << std::endl;
     httpRequest.extractContent(httpRequest.getResponse());

     cout<<endl;

     cout<< "............................................. \n";
     cout << httpRequest.promtInfo << endl;
     

     cout << endl;

     cout << "............................................. \n";
     
     cout << "after pasing \n";

     // Call the function to parse test cases
     std::vector<TestCase> testCases = parseTestCases(httpRequest.promtInfo);

     //// Display the extracted test case information
     //for (size_t i = 0; i < testCases.size(); ++i) {
     //    std::cout << "Test Case " << i + 1 << ":\n";
     //    std::cout << "num1 = " << testCases[i].num1 << ", num2 = " << testCases[i].num2 << "\n";
     //}

     mapTestCasesToCode(testCases, userInputList, functionList);



     //use after gunit test has been run

     // Instrument the code for coverage
     //std::string sourceFilePath = "question2.cpp";
     //std::string outputExecutablePath = "b";

     //// Call the function to instrument the code
     //instrumentCodeForCoverage(sourceFilePath, outputExecutablePath);


    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}


//Prompt:
//
//Given the following scenario :
//
//Variables:
//num1(Type : double)
//num2(Type : double)
//Function :
//    main(Return Type : int)
//    Conditions :
//    Do - While loop(Text : sum < 10)
//    Switch statement(Text : attempt)
//    Case statement(Text : 1)
//    Case statement(Text : 2)
//    Case statement(Text : 3)
//    Default statement
//    If Statement(Text: sum > 10)
//    Generate test cases in the following format :
//
//Test Case 1 :
//
//    num1 = [value], num2 = [value]
//    Test Case 2 :
//
//    num1 = [value], num2 = [value]
//    ...
//
//    Provide the minimum set of test cases covering all possible scenarios.