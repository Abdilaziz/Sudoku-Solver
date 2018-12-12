
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "SudokuGraph.cpp"

#define SUDOKU_SIZE 9

using namespace std;

class Sudoku {

    private:
        // Class used to represent the Sudoku
        // Reasoning can be found in SudokuGraph Class
        SudokuGraph game;
        
        // Keep count of empty spaces still left so we know we are not yet complete
        int numberOfEmptyNodes = 0;

        // Flag used to determine if a new value has been entered into the sudoku
        bool newNumberEntered = false;        

        bool sudokuComplete = false;

        // Flag used to determine wether a value has been guessed or not
        bool prevValueGuessed = false;

        // Set of cell positions where values have been guessed. Important so that we do not repeatedly guess possible values of the same cell when reverting
        set<int> guessedPositions = {};

        // can use a lookUpTable so that valid numbers for each node doesn't need to be recalculated if there hasn't been any new values found
        vector<vector<int>> lookUpTable;

        vector<Node*> getEdges(int currentNode);

        bool vectorIsSubset( vector<int> largerVector, vector<int> smallerVector );

        bool isNumberValid(int position, int value);

        vector<int> getValidNumbers(int position, bool forceReinitalization);

        void reduceNumberOfPossibleValues(bool& reducedLookUpTable);

        void solveUsingConstraints();

        bool solveWithGuessing();

        void sudokuSolver();

    public:

        Sudoku();

        bool parseGame(char* filename);
        void solveGame();

        void prettyPrint();

};

Sudoku::Sudoku() {
    // Initialize our Representation of the sudoku
    this->game = SudokuGraph( SUDOKU_SIZE);

    // intialize the size of our lookUpTable
    int current = 0;
    while ( current < SUDOKU_SIZE*SUDOKU_SIZE ) {
        this->lookUpTable.push_back({});
        current++;
    }

}

/*
 *  Parse the text file and insert digits into the Sudoku
 */
// returns false if there was an error parsing
// the in-memory representation of the game is a class member
bool Sudoku::parseGame(char* fileName) {

    bool formattedFile = true;

    ifstream inputFile;
    string line;

    inputFile.open(fileName);

    // display an error if the fileName submitted by the user couldn't be open 
    if ( !inputFile.is_open() ) {
        cout << "Can't open file: " << fileName << endl;
        formattedFile = false;
    }
    else {

        int numberOfLines = 0;

        while ( getline( inputFile, line ) ) {

            // counts the number of commas to determine the number of digits per line in the text file
            int digitsPerLine = 0;
            numberOfLines++;

            if (numberOfLines <= SUDOKU_SIZE) {

                // flag to determine whether 2 characters in a row are digits (needs to be a digit followed by a comma)
                bool digitRead = false;
                
                for ( int characterIndex = 0; characterIndex < line.length(); characterIndex++ ) {
                    
                    char character = line.at(characterIndex);

                    //ensure it is formatted correctly
                    if ( character == ',' ) {
                        digitRead = false;
                        digitsPerLine++;
                    }
                    else if ( ((int) character) == 13 ) {
                        // Carriage return is read (enter key) ignore it
                    }
                    // if its not a comma, the character should be a digit
                    else {

                        int digit = character - '0'; // converts a digit character to its integer representation
                        
                        // two digits have been read without a , so the formatting is incorrect
                        if ( digitRead ) {
                            cout << "Formatting Error: Each digit should be seperated by a comma." << endl;
                            cout << "Error on line: " << numberOfLines << endl;
                            formattedFile = false;
                            inputFile.close();
                            break;
                        }
                        else if ( digit > 0 && digit <= SUDOKU_SIZE ) {
                            // the digit is valid so add it to the sudoku graph
                            (*this->game.getGraph()).at( digitsPerLine + SUDOKU_SIZE*(numberOfLines-1) ).setValue(digit);
                        }
                        else {
                            cout << "Formatting Error: An incorrect character has been entered." << endl;
                            cout << "Error on line: " << numberOfLines << endl;
                            formattedFile = false;
                            inputFile.close();
                            break;
                        }

                        digitRead = true;
                    }                    

                }

                // each line should have 9 commas except the last one should have 8
                if ( (digitsPerLine != SUDOKU_SIZE) && ( digitsPerLine != SUDOKU_SIZE-1 ) && (numberOfLines == SUDOKU_SIZE) ) {
                    
                    cout << "Formatting Error: There should be " << SUDOKU_SIZE << " commas per line ";
                    cout << "and " << SUDOKU_SIZE-1 << " on the last" << endl;
                    cout << "Error on line: " << numberOfLines << endl;
                    cout << "Digits read: " << digitsPerLine << endl;
                    formattedFile = false;
                    inputFile.close();
                    break;
                }


            }
            // you have too many lines in your text file
            else {
                cout << "You have more than " << SUDOKU_SIZE << " lines in you text file." << endl;
                formattedFile = false;
                break;
            }

        }

        // if there isn't enough lines in the text file
        if ( formattedFile && numberOfLines < SUDOKU_SIZE ) {
            cout << "Formatting Error: There should be " << SUDOKU_SIZE << " lines in the text file." << endl;
            formattedFile = false;
        }

        inputFile.close();
    }

    return formattedFile;

}

void Sudoku::solveGame() {
    // Solve the Sudoku
    this->sudokuSolver();

    // Sudoku should have been completely solved, so we will print the solution
    this->prettyPrint();
}

void Sudoku::sudokuSolver() {

    // Turns out there isn't one easy method to solving a sudoku so a combination of methods should be used, all constrained by
    // the rules that a Sudoku can't have the same number in the same row, column, or 3x3 box (with the graph implementation it is each cell and its connected node)

    while (!this->sudokuComplete) {

        // start by setting flag for new numbers entered false
        this->newNumberEntered = false;

        // Try and solve the sudoku's empty spots using the contraints applied to sudokus (1-9 in row, column, and box)
        this->solveUsingConstraints();

        bool reducedLookUpTable = false;

        // if we never found any empty spaces in the sudoku while solving using constraints we are done!
        if (this->numberOfEmptyNodes == 0) {
            this->sudokuComplete = true;
        }
        // in the case that we went through every cell and haven't been able to find a new valid number for an empty spot
        // another method needs to be used
        else if ( !this->newNumberEntered  ) {

            // find cells with the same set (or a subset) of possible values and remove them from others in the same row, column, or box
            this->reduceNumberOfPossibleValues(reducedLookUpTable);
            
            if ( !reducedLookUpTable ) {
                // if we can't reduce the number of possible values, the only thing we can do is guess values for cells with our list of possible values
                
                this->solveWithGuessing();
            }
        }
    }
    
}

void Sudoku::solveUsingConstraints() {

    this->numberOfEmptyNodes = 0;

    // to start, go through the sudoku and find what numbers can be entered into each empty node
    // without breaking the rules of a Sudoku
    for ( int cellIndex = 0; cellIndex < SUDOKU_SIZE*SUDOKU_SIZE; cellIndex++ ) {
        
        Node* cell = &(*this->game.getGraph()).at(cellIndex);

        // a node is empty if it has its default value of 0
        if ( (*cell).getValue() == 0 ) {
            
            // there are empty positions in the sudoku that need values.
            this->numberOfEmptyNodes++;
            
            // get a list of possible values for this cellIndex 
            vector<int> possibleCellValues = this->getValidNumbers( cellIndex, false );
            
            int possibleCellSize = possibleCellValues.size();

            // if there is only 1 valid number for this cellIndex, it is the correct digit.
            if (possibleCellSize == 1 ) {

                // cout << "Setting CellIndex: " << cellIndex << " As the only valid number: " << validCellNumbers.at(0) << endl;

                // reset lookUpTable for this index as the number has been acquired
                (*cell).setValue( possibleCellValues.at(0) ); 
                this->lookUpTable.at(cellIndex) = {};

                // A new value has been entered, so set the flag to true
                this->newNumberEntered = true; 

                // if a cells value has been previously guessed, we should mark this cell as there is a chance that its value is wrong and has been determined with incorrect information
                if (this->prevValueGuessed) {
                    (*cell).setGuessedFlag(true);
                }
            }
            // if there is more than 1 valid number, check if there is a unique valid number 
            // for this location compared to others in its row, column, and box.
            // Example: how many cells in the same row, column, or box is 8 valid? If there is only 1 spot, it is the answer in that cell
            else {
                
                // list of nodes in the same row, column, and box as the cellIndex
                vector<Node*> edges = (*cell).getEdges();

                // check if all the the possible values in the current cell are valid in cells in the same row, column, or box
                for ( int possibleCellValueIndex = 0; possibleCellValueIndex < possibleCellSize; possibleCellValueIndex++ ) {

                    // flag is true when if this cellIndex has possible that are unique
                    bool possibleValueIsUniqueToThisNode = true;

                    int possibleValue = possibleCellValues.at(possibleCellValueIndex);

                    // get the possible values for all the cells in the same row, column, and box that are empty
                    for ( int edgesIndex = 0; edgesIndex < edges.size(); edgesIndex++ ) {
                        
                        Node* edgeNode = edges.at(edgesIndex);

                        if ( (*edgeNode).getValue() == 0 ) {
                        
                            int nodeIndex = (*edgeNode).getID();

                            vector<int> possibleEdgeValues = this->getValidNumbers( nodeIndex, false );

                            for ( int possibleEdgeValueIndex = 0; possibleEdgeValueIndex < possibleEdgeValues.size(); possibleEdgeValueIndex++ ) {
                                
                                int possibleEdgeValue = possibleEdgeValues.at(possibleEdgeValueIndex);

                                // if the current cells value is the same as a possible value from its edge, it is not unique
                                if ( possibleEdgeValue == possibleValue ) {
                                    possibleValueIsUniqueToThisNode = false;
                                    break;
                                }
                            }

                            // if we already found one instance of the current cells possible value not being unique, move on the the next possible value of the current cell
                            if ( !possibleValueIsUniqueToThisNode ) {
                                break;
                            }

                        }

                    }

                    // After comparing a valid Number with the cells in the same row, column, and box and it is still unique
                    //  it must be the solution for this cell

                    if (possibleValueIsUniqueToThisNode) {
                        // cout << "Setting: " << validNumber << " at: " << (*cell).getID() << endl;
                        this->newNumberEntered = true;

                        (*cell).setValue( possibleValue );
                        this->lookUpTable.at((*cell).getID()) = {};

                        // if a cells value has been previously guessed, we should mark this cell as there is a chance that its value is wrong and has been determined with incorrect information
                        if ( this->prevValueGuessed ) {
                            (*cell).setGuessedFlag(true);
                        }

                        break;
                    }
                }
            }
        }
    }
}

void Sudoku::reduceNumberOfPossibleValues(bool& reducedLookUpTable) {

    // find cells in the same row, column, or grid with the same set (or a subset) of valid numbers.
    // Ex. if 2 cells in the same row have the possible values of [4, 8], one of them has to be a 4 and the other has to be an 8
    // meaning we can remove 4 and 8 from the list of possible values from all other nodes in the same row. 

    int boxWidth = sqrt(SUDOKU_SIZE);

    for ( int cellIndex = 0; cellIndex < SUDOKU_SIZE*SUDOKU_SIZE; cellIndex++ ) {
        
        Node* cell = &(*this->game.getGraph()).at(cellIndex);
        // for empty cells
        if ( (*cell).getValue() == 0 ) {

            // get a list of possible values for this cellIndex
            vector<int> possibleCellValues = this->getValidNumbers(cellIndex, false );

            vector<Node*> matchingColumnValidNumbers = {};
            vector<Node*> matchingRowValidNumbers = {};
            vector<Node*> matchingBoxValidNumbers = {};

            vector<Node*> edgeNodes = (*cell).getEdges();

            for ( int edgeNodeIndex = 0; edgeNodeIndex < edgeNodes.size(); edgeNodeIndex++ ) {

                int edgeNodeId = (*edgeNodes.at(edgeNodeIndex)).getID();

                if ( (*edgeNodes.at(edgeNodeIndex)).getValue() == 0 ) {

                    // get a list of possible values for this edgeNodeId
                    vector<int> possibleEdgeValues = this->getValidNumbers(edgeNodeId, false );

                    // check if the the current cells list of possible values is equal to or is a subset of the current edges possible values
                    if ( this->vectorIsSubset( possibleCellValues, possibleEdgeValues) ) {

                        Node* edge = &(*this->game.getGraph()).at(edgeNodeId);

                        
                        int cellRowNumber = cellIndex/SUDOKU_SIZE;
                        int cellColNumber = cellIndex%SUDOKU_SIZE;
                        int edgeRowNumber = edgeNodeId/SUDOKU_SIZE;
                        int edgeColNumber = edgeNodeId%SUDOKU_SIZE;

                        // we need to determine if the current cell and the current edge are in the the same row and box or the same column and box.

                        if ( cellColNumber == edgeColNumber ) {
                            matchingColumnValidNumbers.push_back(edge); 
                        }
                        else if ( cellRowNumber == edgeRowNumber ) {
                            matchingRowValidNumbers.push_back(edge);
                        }

                        // check if same box
                        if ( ( (cellRowNumber/boxWidth) == (edgeRowNumber/boxWidth) )  && ( (cellColNumber/boxWidth) == (edgeColNumber/boxWidth) ) ) {
                            matchingBoxValidNumbers.push_back(edge);
                        } 
                        
                    }

                }

            }
            
            // if we have found edges with matching matching possible values in the same column
            // the plus 1 here is from the current cell not being included in matchingColumnValidNumbers
            if ( matchingColumnValidNumbers.size() != 0 && ( ( matchingColumnValidNumbers.size() + 1 ) == possibleCellValues.size() ) ) {

                vector<Node*> matchingCells = {cell};
                matchingCells.insert( matchingCells.end(), matchingColumnValidNumbers.begin(), matchingColumnValidNumbers.end());

                // go through all the rows in this column and reduce the size of their possible values if their index doesn't have matching valid numbers
                for ( int rowIndex = 0; rowIndex < SUDOKU_SIZE; rowIndex++ ) {

                    int columnCellIndex = cellIndex%SUDOKU_SIZE + rowIndex*SUDOKU_SIZE;

                    for ( int possibleCellValueIndex = 0; possibleCellValueIndex < possibleCellValues.size(); possibleCellValueIndex++ ) {

                        int possibleValue = possibleCellValues.at(possibleCellValueIndex);

                        bool isOtherCell = true;
                        // set isOtherCell flag false if the current cell isn't either the current cell, or the cells with the matching possible values
                        for ( int matchingNodeIndex = 0; matchingNodeIndex < matchingCells.size(); matchingNodeIndex++ ) {
                            int ignorableIDs = (*matchingCells[matchingNodeIndex]).getID();
                            if ( ignorableIDs == columnCellIndex ) {
                                isOtherCell = false;
                            }
                        }

                        // remove the current possible value from others in the column
                        // ideally valid numbers list would be reduced to just one, and that answer be entered into the sudoku in the loop
                        if ( isOtherCell ) {

                            vector<int> columnsPossibleValues = this->lookUpTable.at(columnCellIndex);
                            vector<int> newValidNumbers = {};

                            for ( int i = 0; i < columnsPossibleValues.size(); i++ ) {
                                int columnsPossibleValue = columnsPossibleValues[i];

                                if (columnsPossibleValue != possibleValue ) {
                                    newValidNumbers.push_back(columnsPossibleValue);
                                }
                                else {
                                    // cout << "Column: A valid number has been removed from the lookUpTable. ";
                                    // cout << "Valid Number: " << columnsPossibleValue << " Cell: " << columnCellIndex << endl;

                                    reducedLookUpTable = true;

                                }

                            }

                            // set the smaller set of possibilites back into the look up table
                            this->lookUpTable.at(columnCellIndex) = newValidNumbers;
                        }

                    }

                }

            }
            
            if ( matchingRowValidNumbers.size() != 0  && ( ( matchingRowValidNumbers.size() + 1 ) == possibleCellValues.size() ) ) {

                int currentCellRow = cellIndex/SUDOKU_SIZE;

                vector<Node*> matchingCells = {cell};
                matchingCells.insert( matchingCells.end(), matchingRowValidNumbers.begin(), matchingRowValidNumbers.end());

                // go through all the columns in this row and reduce the size of their list of possible values
                for ( int columnIndex = 0; columnIndex < SUDOKU_SIZE; columnIndex++ ) {

                    int rowCellIndex = columnIndex%SUDOKU_SIZE + currentCellRow*SUDOKU_SIZE;

                    for ( int validNumberIndex = 0; validNumberIndex < possibleCellValues.size(); validNumberIndex++ ) {
                        // current cells valid number
                        int possibleValue = possibleCellValues.at(validNumberIndex);

                        bool isOtherCell = true;
                        // set isOtherCell flag false if the current cell isn't either the current cell, or the cells with the matching possible values
                        for ( int matchingNodeIndex = 0; matchingNodeIndex < matchingCells.size(); matchingNodeIndex++ ) {
                            int ignorableIDs = (*matchingCells[matchingNodeIndex]).getID();
                            if ( ignorableIDs == rowCellIndex ) {
                                isOtherCell = false;
                            }
                        }

                        // remove valid numbers from the same row
                        // ideally valid numbers list would be reduced to just one, and that answer be entered into the sudoku in the next while loop
                        if ( isOtherCell ) {
                            // a cell in the same columns valid number
                            vector<int> rowsPossibleValues = this->lookUpTable.at(rowCellIndex);
                            vector<int> newValidNumbers = {};

                            for ( int i = 0; i < rowsPossibleValues.size(); i++ ) {
                                int validNumber = rowsPossibleValues[i];

                                if (validNumber != possibleValue ) {
                                    newValidNumbers.push_back(validNumber);
                                }
                                else {
                                    // cout << "Row: A valid number has been removed from the lookUpTable. ";
                                    // cout << "Valid Number: " << validNumber << " Cell: " << rowCellIndex << endl;

                                    reducedLookUpTable = true;

                                }
                            }
                            // set the smaller set of possibilites back into the look up table
                            this->lookUpTable.at(rowCellIndex) = newValidNumbers;
                        }
                    }
                }
            }

            if ( matchingBoxValidNumbers.size() != 0  && ( ( matchingBoxValidNumbers.size() + 1 ) == possibleCellValues.size() ) ) {

                vector<Node*> matchingCells = {cell};
                matchingCells.insert( matchingCells.end(), matchingBoxValidNumbers.begin(), matchingBoxValidNumbers.end());

                int currentBoxCol = (cellIndex%SUDOKU_SIZE)/boxWidth;
                int currentBoxRow = (cellIndex/SUDOKU_SIZE)/boxWidth;
                // loop through current box row
                for ( int boxRowIndex = 0; boxRowIndex < boxWidth; boxRowIndex++ ) {
                    // loop through current box column
                    for ( int boxColIndex = 0; boxColIndex < boxWidth; boxColIndex++ ) {

                        // convert the current box index into row and column values, and finaly the current cells index
                        int colNumb = (boxWidth*currentBoxCol) + boxColIndex;
                        int rowNumb = (boxWidth*currentBoxRow) + boxRowIndex;

                        int cellNumb = colNumb + rowNumb*SUDOKU_SIZE;

                        for ( int possibleCellValueIndex = 0; possibleCellValueIndex < possibleCellValues.size(); possibleCellValueIndex++ ) {

                            int possibleValue = possibleCellValues.at(possibleCellValueIndex);

                            bool isOtherCell = true;
                            // set isOtherCell flag false if the current cell isn't either the current cell, or the cells with the matching possible values
                            for ( int matchingNodeIndex = 0; matchingNodeIndex < matchingCells.size(); matchingNodeIndex++ ) {
                                int ignorableIDs = (*matchingCells[matchingNodeIndex]).getID();
                                if ( ignorableIDs == cellNumb ) {
                                    isOtherCell = false;
                                }
                            }

                            // remove valid numbers from the box
                            if ( isOtherCell ) {

                                vector<int> boxsPossibleValues = this->lookUpTable.at(cellNumb);
                                vector<int> newValidNumbers = {};

                                for ( int i = 0; i < boxsPossibleValues.size(); i++ ) {
                                    int validNumber = boxsPossibleValues[i];

                                    if (validNumber != possibleValue ) {
                                        newValidNumbers.push_back(validNumber);
                                    }
                                    else {
                                        // cout << "Box: A valid number has been removed from the lookUpTable. ";
                                        // cout << "Valid Number: " << validNumber << " Cell: " << cellNumb << endl;

                                        reducedLookUpTable = true;
                                    }
                                }
                                // set the smaller set of possibilites back into the look up table
                                this->lookUpTable.at(cellNumb) = newValidNumbers;
                            }
                        }
                    }
                }
            }
        }
    }
}

bool Sudoku::solveWithGuessing() {

    // check to see if a previoius guess was made and if it has lead to a situation where it needs to be reverted.
    // we know that our previous guess is wrong when an empty node has no possible values

    if (this->prevValueGuessed) {

        bool incorrectGuess = false;

        // find cells with no value and no possible values
        for ( int cellIndex = 0; cellIndex < SUDOKU_SIZE*SUDOKU_SIZE; cellIndex++ ) {

            Node* cell = &(*this->game.getGraph()).at(cellIndex);

            if ( (*cell).getValue() == 0 ) {

                // get possible values
                vector<int> possibleValues = this->getValidNumbers(cellIndex, false);

                if ( possibleValues.size() == 0 ) {
                    // this cell has no value, with no more possible values, meaning a previous guess has lead to a mistake being made (or no solution exists???/)
                    incorrectGuess = true;
                    break;
                }

            }
        }

        // empty each cell whose value has been found by making the incorrect guess
        // if we are at the point where we need to guess again and there are no empty positions with no possible values, we have not broken any sudoku rules 
        // and can assume that the previous guess was correct (set guessed flag on each node to false)
        for ( int cellIndex = 0; cellIndex < SUDOKU_SIZE*SUDOKU_SIZE; cellIndex++ ) {

            Node* cell = &(*this->game.getGraph()).at(cellIndex);

            if ( (*cell).isGuessed() ) {

                if ( incorrectGuess ) {
                    (*cell).setValue(0);
                }

                (*cell).setGuessedFlag(false);
            }

        }
  
    }

    // guess a value from the cell with the fewest amount of possibilites (best chances of success)
    int minPosition = -1;
    int minSize = SUDOKU_SIZE;
    for ( int cellIndex = 0; cellIndex < SUDOKU_SIZE*SUDOKU_SIZE; cellIndex++ ) {
        Node* cell = &(*this->game.getGraph()).at(cellIndex);
        if ( (*cell).getValue() == 0 ) {
            // if a previous guess has just been reverted, we need our list of valid numbers to be re-initialized to a proper state
            vector<int> possibleValues = this->getValidNumbers(cellIndex, true);
            int cellIndexSize = possibleValues.size();

            // has the current index already been completely guessed
            bool indexHasBeenChecked = this->guessedPositions.find(cellIndex) != this->guessedPositions.end();
            // if the current cell has the smallest number of possible values, and has not already been completely guessed, we should keep its position (minPosition) 
            if ( (cellIndexSize < minSize) && (cellIndexSize > 0)  &&  (!indexHasBeenChecked) ) {
                minSize = cellIndexSize;
                minPosition = cellIndex;
            }
        }
    }

    // guess a value for the minPosition in the current cell

    Node* cell = &(*this->game.getGraph()).at(minPosition);

    int previouslyGuessedIndex = (*cell).getGuessedIndex();
    int newlyGuessedIndex = previouslyGuessedIndex+1;

    vector<int> possibleValues = this->lookUpTable.at(minPosition);

    // the value guessed should be a value from our possible values that we have not already guessed
    int newGuess = possibleValues[newlyGuessedIndex];
    (*cell).setGuessedIndex(newlyGuessedIndex);
    (*cell).setGuessedFlag(true);

    // cout << "Guessed the value : " << newGuess << " in position: " << (*cell).getID() << endl;

    (*cell).setValue(newGuess);
    this->lookUpTable.at(minPosition) = {}; // empty our list of possible values stored for this position

    // if the index of the newly guessed value is the last one, we should add this cell to our set of cells whose possible values have been completely guessed
    if ( newlyGuessedIndex == (possibleValues.size()-1) ) {
        this->guessedPositions.insert(minPosition);
    }

    // ensure we set the value guessed flag to true so that we know to check if this guess is a mistake when we need to guess again
    this->prevValueGuessed = true;

}

// Print the game
void Sudoku::prettyPrint() {

    this->game.prettyPrint();
}

// get list of cells in the same row, column, and box as the nodeID
vector<Node*> Sudoku::getEdges(int nodeID) {
    return (*this->game.getGraph()).at(nodeID).getEdges();
}

// get list of possible values for the cell at the index: position
vector<int> Sudoku::getValidNumbers(int position, bool forceReinitalization) {

    vector<int> validNumbers = {};

    // if we have previous possible values for this index already stored, we don't need to get them from scratch again
    if ( this->lookUpTable.at(position).size() == 0 || forceReinitalization ) {
        // check what values from 1 to 9 are valid for the current cell
        for ( int number = 1; number <= SUDOKU_SIZE; number++ ) {
            if ( this->isNumberValid(position, number) ) {
                validNumbers.push_back(number);
            }
        }
        this->lookUpTable.at(position) = validNumbers;
    }
    else {
        vector<int> oldValidNumbers = this->lookUpTable.at(position);
        // instead of just returning our list of possible values, we can check to see if it is still valid!!!
        for ( int oldValidNumber = 0; oldValidNumber < oldValidNumbers.size(); oldValidNumber++ ) {
            if ( this->isNumberValid(position, oldValidNumbers[oldValidNumber]) ) {
                validNumbers.push_back(oldValidNumbers[oldValidNumber]);
            }
        }
        this->lookUpTable.at(position) = validNumbers;
    }

    return validNumbers;
}

// check if the passed value is valid for the passed cell position
bool Sudoku::isNumberValid(int position, int value) {

    bool isValid = true;
    vector<Node*> edges = this->getEdges(position);
    // a value is valid if it doesn't match any cell in the same row, column, or box
    for ( int edgesIndex = 0; edgesIndex < edges.size(); edgesIndex++ ) {
        if ( (*edges.at(edgesIndex)).getValue() == value) {
            isValid = false;
            break;
        }
    }

    return isValid;
}

// vector is a subset of another if all the values of the larger vector can be found in the smaller vector
bool Sudoku::vectorIsSubset( vector<int> largerVector, vector<int> smallerVector ) {
    bool isASubSet = false;
    if ( largerVector.size() >= smallerVector.size() ) {
        int matchingValues = 0;
        for ( int largerVectorIndex = 0; largerVectorIndex < largerVector.size(); largerVectorIndex++ ) {
            for ( int smallerVectorIndex = 0; smallerVectorIndex < smallerVector.size(); smallerVectorIndex++  ) {
                if ( largerVector[largerVectorIndex] == smallerVector[smallerVectorIndex] ) {
                    matchingValues++;
                    break;
                }
            }
        }
        if ( matchingValues == smallerVector.size() ) {
            isASubSet = true;
        }
    }

    return isASubSet;
}