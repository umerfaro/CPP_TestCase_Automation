#include <iostream>
using namespace std;

int main() {
    // Declare variables
    double num1, num2, sum;

    // Get input from the user
    cout << "Enter the first number: ";
    cin >> num1;

    cout << "Enter the second number: ";
    cin >> num2;

    // Calculate the sum
    sum = num1 + num2;

    // Display the result
    cout << "The sum of " << num1 << " and " << num2 << " is: " << sum << std::endl;

    // Use a do-while loop to ask for input up to 3 times
    int attempt = 1;
    do {
        // Ask for new input
        cout << "The sum is not greater than 10. Enter new numbers." << endl;

        cout << "Enter the first number: ";
        cin >> num1;

        cout << "Enter the second number: ";
        cin >> num2;

        // Recalculate the sum
        sum = num1 + num2;

        // Display the result
        cout << "The sum of " << num1 << " and " << num2 << " is: " << sum << std::endl;

        // Use a switch statement to handle different attempts
        switch (attempt) {
        case 1:
            cout << "First attempt." << endl;
            break;
        case 2:
            cout << "Second attempt." << endl;
            break;
        case 3:
            cout << "Third attempt." << endl;
            break;
        default:
            cout << "Invalid attempt." << endl;
            break;
        }

        attempt++;
    } while (sum < 10);

    // Display a message once the sum is greater than 10
    if (sum > 10) {
        cout << "The sum is greater than 10." << std::endl;
    }
    else {
        cout << "You have reached the maximum number of attempts." << std::endl;
    }

    return 0;
}
