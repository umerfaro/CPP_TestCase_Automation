#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct Node {
    char data;
    Node* right;
    Node* down;

    Node(char val) : data(val), right(nullptr), down(nullptr) {}
};

class LinkedList2D {
public:
    LinkedList2D() : head(nullptr), currentRow(nullptr), lastRow(nullptr) {}

    void insert(char data) {
        if (!head) {
            head = new Node(data);
            currentRow = head;
            lastRow = head;
            cout << head->data;
            return;
        }

        if (data == '\n') {
            if (currentRow != lastRow) {
                currentRow = lastRow;
            }
            currentRow->down = new Node(data);
            lastRow = currentRow->down;
        }
        else {
            currentRow->right = new Node(data);
            currentRow = currentRow->right;
        }
    }

    void print() {
        Node* currentRowPtr = head;
        while (currentRowPtr) {
            Node* currentCol = currentRowPtr;
            while (currentCol) {
                cout << currentCol->data;
                currentCol = currentCol->right;
            }
            currentRowPtr = currentRowPtr->down;
        }
    }

    bool searchForIntMain() {
        Node* currentRowPtr = head;
        Node* currentColPtr;
        bool found = false;
        bool found1 = false;

        while (currentRowPtr) {
            currentColPtr = currentRowPtr;
            while (currentColPtr) {
                if (checkForIntMain(currentColPtr)) {
                    found = true;

                    break;
                }
                currentColPtr = currentColPtr->right;
            }
            if (found) {
                break;
            }
            currentRowPtr = currentRowPtr->down;
        }


        while (currentRowPtr) {
            currentColPtr = currentRowPtr;
            while (currentColPtr) {
                if (checkForCin(currentColPtr)) {
                    found1 = true;
                    cout << "found cin\n";
                    break;
                }
                currentColPtr = currentColPtr->right;
            }
            if (found1) {
                break;
            }
            currentRowPtr = currentRowPtr->down;
        }

        return found;
    }

    bool searchForCin() {
        Node* currentRowPtr = head;
        Node* currentColPtr;
        bool found = false;

        while (currentRowPtr) {
            currentColPtr = currentRowPtr;
            while (currentColPtr) {
                if (checkForCin(currentColPtr)) {
                    found = true;
                    break;
                }
                currentColPtr = currentColPtr->right;
            }
            if (found) {
                break;
            }
            currentRowPtr = currentRowPtr->down;
        }

        
        return found;
    }

    bool checkForIntMain(Node* currentColPtr) {
        while (currentColPtr) {
            if (currentColPtr->data == 'i' && hasSubstring("int main()", currentColPtr)) {
                return true;
            }
            currentColPtr = currentColPtr->right;
        }
        return false;
    }

    bool checkForCin(Node* currentColPtr) {
        while (currentColPtr) {
            if (currentColPtr->data == 'c' && hasSubstring("cin", currentColPtr)) {
                return true;
            }
            currentColPtr = currentColPtr->right;
        }
        return false;
    }

    bool hasSubstring(const string& substring, Node* currentColPtr) {
        for (char c : substring) {
            if (currentColPtr) {
                if (currentColPtr->data == c) {
                    currentColPtr = currentColPtr->right;
                }
                else {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        return true;
    }

    ~LinkedList2D() {
        while (head) {
            Node* currentRowPtr = head;
            head = head->down;
            while (currentRowPtr) {
                Node* currentCol = currentRowPtr;
                currentRowPtr = currentRowPtr->right;
                delete currentCol;
            }
        }
    }

private:
    Node* head;
    Node* currentRow;
    Node* lastRow;
};

int main() {
    LinkedList2D linkedList;
    char character;
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;

    ifstream inputFile("Parsed_code.txt");
    if (!inputFile.is_open()) {
        cerr << "Error opening the file." << endl;
        return 1;
    }

    while (inputFile.get(character)) {
        if (character == '/' && inputFile.peek() == '/') {
            inSingleLineComment = true;
        }
        else if (character == '/' && inputFile.peek() == '*') {
            inMultiLineComment = true;
        }
        else if (character == '*' && inputFile.peek() == '/') {
            inMultiLineComment = false;
            inputFile.get(); // Consume the closing '*'
        }
        else if (character == '\n') {
            inSingleLineComment = false;
        }

        if (!inSingleLineComment && !inMultiLineComment) {
            linkedList.insert(character);
        }
    }

    if (linkedList.searchForIntMain()) {
        cout << "Found 'int main()' in the code." << endl;

        if (linkedList.searchForCin()) {
            cout << "Found 'cin' in the code after 'int main()'." << endl;
        }
        else {
            cout << "Did not find 'cin' in the code after 'int main()'." << endl;
        }
    }
    else {
        cout << "Did not find 'int main()' in the code." << endl;
    }

    return 0;
}
