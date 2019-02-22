# Sudoku-Solver
An Algorithm to Solve Sudoku's without using Brute Force.


##Problem:
Programming a Sudoku Solver (9x9 grid) by guessing the values in each empty position is pretty simple with a recursive function.
You can just keep on guessing values until you hit a point where there is no solution, and then backtrack to the last guess until the
entire grid is filled in. This process is easy to program, but isn't efficient so I figured I should find out how to systematically
solve a sudoku the proper way.

Overall rule for a Sudoku is that each Column, Row, and 3x3 Box in a Sudoku needs to have the values 1 through 9, so there are no
repeated values.

##Solving a Sudoku:
  
  You can go through every empty space in a Sudoku and use these steps to solve them.
  
  ###Step 1:
    Check to see what the valid numbers are for this empty position. A number is valid if it hasn't appeared in the same Row, Column, or 
    3x3 Box.

  In many simple Sudokus, this would be the only step that would be required. If there is only 1 valid number for that empty space,
  we know that it is the solution and should fill in the empty spot.
  
  But if there is more than 1 valid number, we need more information.

  ###Step 2:
    Check what the valid numbers are in empty spots that are in the same Row, Column, and 3x3 grid.

  With this information, we can find out exactly what empty spots have unique values in their list of valid numbers. Those unique values
  are the solutions for them.
  For Example, in a row with 3 empty spots where two of them can be either [4,6] and the third can be [3,4,6], we know that the solution 
  to the third space has to be 3, and the other 2 are either a 4 or a 6. With this step we can decrease the size of numbers that are
  valid for empty spaces.
  
  Again using Step 2, we can solve some more complicated Sudokus, but for anything more complicated, there really isn't a way to proceed
  other than guessing from our list of valid values.
  
  ###Step 3:
    We can fill in empty spots by guessing from our list of possible values. It is best to guess a value for an empty spot that has the
    smallest list of valid numbers so we can lower the chance of making a mistake.
    
   We know we have made a mistake if we hit a point where we still have empty positions in the sudoku, but with no valid numbers.
   If we make a mistake, we should go back to the point where the last guess was made, and try again.
   
   With these Steps Any Valid sudoku can be solved. 


##Implementation of Solution:
  
  The keys to the implementation of this solution had to do with the fact that for each empty spot, we need to be able to quickly 
  access the cells that are in the same Row, Column, and 3x3 Box. Using a Graph has the Data Structure for the Sudoku, give me the
  flexibility to make each spot in the sudoku a Node, edges of each node correspond to other nodes that are in the same Column, Row, and 
  3x3 Box.
  
  Using a Graph, and a LookUp Table to store valid numbers for each cell (so that checking again and again could be avoided) makes
  finding a solution quickly possible.
  
  

