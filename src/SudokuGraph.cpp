
#include <vector>
#include <math.h>


using namespace std;


/*
    At first I figured a 2D array would be a simple way to represent a sudoku, I was even trying to think of a way a Set could be used to represent values in a row, a column, or a box (since they need to be unqiue)
    After spending some time thinking about how I would solve the Sudoku, I knew that being able to easily check cells in the same row, column, or box
    would be very important to solving a sudoku efficiently. That is when I thought of using a graph to represent a cells relationship with other cells in the same row, column, or box

    Graphs can be represented using:
        Example Graph:
            0 -  1         
              /  |      
            3  -  2
    Edge Lists:
        A list of edges that exist between 2 nodes:
        (Seperate array per edge)
        Ex. [[0,1], [1,2], [1,3], [2,3]]

    Adjacency Lists:
        List of nodes that have edges to eachother:
        (Seperate array per node)
        Ex. [ [1] ,  [0,2,3] , [1,3] ,    [1,2] ]

    Adjacency Matrix:
        Set matrix of 0 and 1 representing if there is a edge between that cells row and column index.
        Useful for when you have 2 nodes and want to know if there is an edge between them.
        Not memory efficient when a graph is sparse (low number of edges)
        Each edge is also represented twice.
        Ex.
            id: 0,id:1, id:2 id:3
            id:0   [[0,  1,  0,  0],
            id:1   [1,  0,  1,  1],
            id:2   [0,  1,  0,  1],
            id:3   [0,  1,  1,  0]]

    For a sudoku, the best implementation is with an Adjacency list due to space

*/

class Node {

    // A Node has:
    private:
        int id; // the Node/Cells Index
        int value; // The value in the position of the Sudoku
        bool guessedFlag = false; // if the value stored in this cell has been found by guessing a value at some point
        int guessedIndex = -1; // index from the list of possible values for this cell that was last guessed
        vector<Node*> edges = {}; // list of edges in the same row, column, and box as this cell

    public:

        Node(int value, int id) {
            this->value = value;
            this->id = id;
        };

        int getID() {
            return this->id;
        }

        int getValue() {
            return this->value;
        }

        bool isGuessed() {
            return this->guessedFlag;
        }

        void setGuessedFlag(bool guessedFlag) {
            this->guessedFlag = guessedFlag;
        }

        void setGuessedIndex( int guessedIndex ) {
            this->guessedIndex = guessedIndex;
        }

        int getGuessedIndex() {
            return this->guessedIndex;
        }

        void setValue(int value) {
            this->value = value;
        }

        vector<Node*> getEdges() {
            return this->edges;
        }

        // insert an edge to the list of edges to this node/cell
        void insertEdge(Node* edge) {
            this->edges.push_back(edge);
        }

};

class SudokuGraph {

    private:

        vector<Node> sudokuGraph; // Game is represented by a vector of Nodes containing a value, and an adjacency list of what other nodes it has edges with

        int sudokuSize;

        void insertEdge(int node_from, int node_to) {
            Node* fromNode = &this->sudokuGraph.at(node_from);
            Node* toNode = &this->sudokuGraph.at(node_to);
            (*fromNode).insertEdge(toNode);
        }

    public:
        // default the sudoku size to 9 if it isn't specified
        SudokuGraph() {
            this->sudokuSize = 9;
        }
        SudokuGraph( int sudokuSize );

        vector<Node>* getGraph() {
            return &(this->sudokuGraph);
        }

        void prettyPrint();

};

SudokuGraph::SudokuGraph(int sudokuSize) {

    this->sudokuSize = sudokuSize;

    // initialize the nodes and edges for the representation of the sudoku

    // Nodes 
    // Initialized with a value of 0, represents an empty spot in the sudoku
    for ( int cellNumb = 0; cellNumb < this->sudokuSize*this->sudokuSize; cellNumb++ ) {
        this->sudokuGraph.push_back(Node(0, cellNumb));
    }

    int numberOfBoxesInRow = sqrt(this->sudokuSize);

    // Edges
    // Each node should have an edge with 8 other is its row, 8 others in its column, and 4 others from its box (not accounted for in its row in and column)
    for ( int cellNumb = 0; cellNumb < this->sudokuSize*this->sudokuSize; cellNumb++ ) {

        // if cell is 0: row is 1-8, col: is 9,18,27,36,45,54,63,72, box is: 10,11,19,20

        int currentCellsRow = cellNumb/this->sudokuSize;
        int currentCellsCol = cellNumb%this->sudokuSize;

        // add edges for nodes in the same row and column
        for ( int index = 0; index< this->sudokuSize; index++ ) {
            
            // add edges for cells in the same row
            if ( index != currentCellsCol  ) {
                this->insertEdge( cellNumb, currentCellsRow*this->sudokuSize + index );
            }

            // add edges for cells in the same column
            if ( index != currentCellsRow ) {
                this->insertEdge( cellNumb, index*this->sudokuSize + currentCellsCol );
            }
        }

        int currentBoxCol = currentCellsCol/numberOfBoxesInRow;
        int currentBoxRow = currentCellsRow/numberOfBoxesInRow;
        // loop through current box row
        for ( int boxRowIndex = 0; boxRowIndex < numberOfBoxesInRow; boxRowIndex++ ) {
            // loop through current box column
            for ( int boxColIndex = 0; boxColIndex < numberOfBoxesInRow; boxColIndex++ ) {

                // convert the current box index into row and column values, and finaly the current cells index
                int colNumb = (numberOfBoxesInRow*currentBoxCol) + boxColIndex;
                int rowNumb = (numberOfBoxesInRow*currentBoxRow) + boxRowIndex;

                int currentBoxCell = colNumb + rowNumb*this->sudokuSize;
                // if the current cell isn't in the same row or column (already accounted for), add the edge
                if ( (colNumb != currentCellsCol) && (rowNumb != currentCellsRow) ) {
                    this->insertEdge(cellNumb , currentBoxCell);
                }
            }
        }
    }


}

void SudokuGraph::prettyPrint() {

    cout << endl;

    for ( int i=0; i < this->sudokuSize*this->sudokuSize; i++ ) {

        cout << this->sudokuGraph.at(i).getValue();
        if ( i%this->sudokuSize == (this->sudokuSize-1) ) {
            cout << endl;
        }
        else {
            cout << ',';
        }
        
    }
    cout << endl;
}



