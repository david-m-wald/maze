#include <iostream>
#include <cstdlib>
#include <ctime>

enum class MazeState {
	WALL,
	PATH,
	START,
	END,
	UNASSIGNED
};

struct Direction {
	int deltaRow;
	int deltaCol;
};

void generateMaze(int nRows, int nCols);
void initializeMaze(MazeState **maze2D, int nRows, int nCols, int& startRow, int& startCol, int& endRow, int& endCol);
void createPath(MazeState **maze2D, int nRows, int nCols, int activeRow, int activeCol);
void printMaze(MazeState **maze2D, int nRows, int nCols);

int main()
{
	int nRows, nCols;
	std::cout << "Enter number of rows: ";
	std::cin >> nRows;
	std::cout << "Enter number of columns: ";
	std::cin >> nCols;
	system("cls");

	srand(static_cast<unsigned int>(time(NULL)));

	generateMaze(nRows, nCols);

	system("pause");
	return 0;
}

//Function generates a new maze for a given input # of rows and columns
void generateMaze(int nRows, int nCols)
{
	//dynamically create 2D array for maze states in contiguous memory
	MazeState **maze2D = new MazeState*[nRows];		  //allocate memory for 2D array as an array of pointers to row subarrays
	MazeState *maze1D = new MazeState[nRows * nCols]; //allocate memory for 1D array equivalent of flattened 2D array
	for (int row = 0; row < nRows; row++)
		maze2D[row] = &maze1D[row * nCols];			  //"unflatten" 1D array into row subarrays of 2D array

	int startRow, startCol, endRow, endCol;
	initializeMaze(maze2D, nRows, nCols, startRow, startCol, endRow, endCol);

	int firstRow = 1 + (2 * (rand() % (nRows / 2))); //first active row for path creator is odd-indexed
	int firstCol = 1 + (2 * (rand() % (nCols / 2))); //first active column for path creator is odd-indexed
	createPath(maze2D, nRows, nCols, firstRow, firstCol);

	printMaze(maze2D, nRows, nCols);
}

//Function performs initialization steps for maze generation
void initializeMaze(MazeState **maze2D, int nRows, int nCols, int& startRow, int& startCol, int& endRow, int& endCol)
{
	//Preset even rows and columns as walls and remaining squares as unassigned paths to ensure full maze coverage
	for (int row = 0; row < nRows; row++) {
		for (int col = 0; col < nCols; col++) {
			if (row % 2 == 0 || col % 2 == 0)
				maze2D[row][col] = MazeState::WALL;
			else
				maze2D[row][col] = MazeState::UNASSIGNED;
		}
	}

	//Choose start position
	//Randomize start row - must be first, last, or odd-indexed (interior) row
	startRow = nRows + 1;
	while (!(startRow == 0 || startRow == nRows - 1 || startRow % 2 == 1))
		startRow = rand() % nRows;

	//Randomize start column
	if (startRow == 0 || startRow == nRows - 1)      //First or last row?
		startCol = 1 + (2 * (rand() % (nCols / 2))); //Only odd-indexed (interior) columns are permitted
	else											 //Interior row?
		startCol = (nCols - 1) * (rand() % 2);       //Only first or last columns are permitted

	//Choose end position
	//Ensure that end position is not the same as start position
	do {
		//Randomize end row - must be first, last, or odd-indexed (interior) row
		endRow = nRows + 1;
		while (!(endRow == 0 || endRow == nRows - 1 || endRow % 2 == 1))
			endRow = rand() % nRows;

		//Randomize end column
		if (endRow == 0 || endRow == nRows - 1)        //First or last row?
			endCol = 1 + (2 * (rand() % (nCols / 2))); //Only odd-indexed (interior) columns are permitted
		else										   //Interior row?
			endCol = (nCols - 1) * (rand() % 2);       //Only first or last columns are permitted
	} while (endRow == startRow && endCol == startCol);

	//Add start and end positions to maze path
	maze2D[startRow][startCol] = MazeState::START;
	maze2D[endRow][endCol] = MazeState::END;
}

/*Recursive function generates fully interconnected path through maze with no branching subpath intersecting
  itself or its parent path other than at branching point*/
void createPath(MazeState **maze2D, int nRows, int nCols, int activeRow, int activeCol)
{
	int nRemainingDirections;
	Direction direction, remainingDirections[4];
	
	maze2D[activeRow][activeCol] = MazeState::PATH;

	while (true) {
		nRemainingDirections = 0;

		//Find directions in which a path has not yet been connected
		if (activeRow - 2 >= 0 && maze2D[activeRow - 2][activeCol] == MazeState::UNASSIGNED)		 //Up
			remainingDirections[nRemainingDirections++] = { -2, 0 };
		if (activeRow + 2 <= nRows - 1 && maze2D[activeRow + 2][activeCol] == MazeState::UNASSIGNED) //Down
			remainingDirections[nRemainingDirections++] = { 2, 0 };
		if (activeCol - 2 >= 0 && maze2D[activeRow][activeCol - 2] == MazeState::UNASSIGNED)		 //Left
			remainingDirections[nRemainingDirections++] = { 0, -2 };
		if (activeCol + 2 <= nCols - 1 && maze2D[activeRow][activeCol + 2] == MazeState::UNASSIGNED) //Right
			remainingDirections[nRemainingDirections++] = { 0, 2 };

		if (nRemainingDirections == 0) return; //All directions have a connected path
		else {
			direction = remainingDirections[rand() % nRemainingDirections];
			//Create path between active and adjacent position (removing preset wall)
			maze2D[activeRow + direction.deltaRow / 2][activeCol + direction.deltaCol / 2] = MazeState::PATH;

			//Recurse
			createPath(maze2D, nRows, nCols, activeRow + direction.deltaRow, activeCol + direction.deltaCol);
		}
	}
}

//Function to print maze
void printMaze(MazeState **maze2D, int nRows, int nCols)
{
	for (int row = 0; row < nRows; row++) {
		for (int col = 0; col < nCols; col++) {
			if (maze2D[row][col] == MazeState::WALL)
				std::cout << /*std::setw(2) << */'#';
			else if (maze2D[row][col] == MazeState::START)
				std::cout << 'S';
			else if (maze2D[row][col] == MazeState::END)
				std::cout << 'E';
			else
				std::cout << ' ';
		}
		std::cout << std::endl;
	}
}