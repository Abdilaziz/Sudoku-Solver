
#include <iostream>
#include "Sudoku.cpp"

using namespace std;

bool isFileNameValid(char* filename) {
    
    // is a text file? : ends with .txt

    return true;

}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        cout << argv[0] << ": Only input argument should be the Sudoku text File." << endl;
        return 0;
    }

    // can do validation on input

    char* filename = argv[1];
    
    if (isFileNameValid(filename)) {
        Sudoku game = Sudoku();
        if ( !game.parseGame(filename) ) {
            cout << "Please fix the formatting." << endl;
        }
        else {
            cout << "File has been parsed!!" << endl;
            game.solveGame();
        }

    }
    else {
        cout << filename << " is not a valid filename" << endl;
    }

    



}