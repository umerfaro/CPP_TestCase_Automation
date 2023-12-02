#pragma once
#include<iostream>
#include<string>
using namespace std;
#ifndef CODEANALYSER_H
#define CODEANALYSER_H


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


#endif // !CODEANALYSER_H
