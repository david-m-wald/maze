#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <ctime>

using namespace std;

HANDLE hStdout, hStdin;

enum class MazeState {
	WALL,
	PATH,
	START,
	END,
	TRAVERSED, //For maze solution finder only
	SOLUTION,
	UNASSIGNED //For maze generator only
};

struct Direction {
	int deltaRow;
	int deltaCol;
};

class Maze {
private:
	MazeState **maze2D;
	MazeState *maze1D;
	const int nRows, nCols;
	int startRow, startCol, endRow, endCol;
public:
	//Constructor generates a new maze for a given input # of rows and columns
	Maze(int rows, int cols) : nRows(rows), nCols(cols)
	{
		//Dynamically create 2D array for maze states in contiguous memory
		maze2D = new MazeState*[nRows];		    //Allocate memory for 2D array as an array of pointers to row subarrays
		maze1D = new MazeState[nRows * nCols];  //Allocate memory for 1D array equivalent of flattened 2D array
		for (int row = 0; row < nRows; row++)
			maze2D[row] = &maze1D[row * nCols];	//"unflatten" 1D array into row subarrays of 2D array

		initializeMaze();

		int firstRow = 1 + (2 * (rand() % (nRows / 2))); //First active row for path creator is odd-indexed
		int firstCol = 1 + (2 * (rand() % (nCols / 2))); //First active column for path creator is odd-indexed
		createPath(firstRow, firstCol);
		findSolution(startRow, startCol);
		printMaze();
	}

	//Destructor deallocates dynamic memory for maze array
	~Maze()
	{
		delete[] maze1D;
		delete[] maze2D;
	}

	//Member function performs initialization steps for maze generation
	void initializeMaze()
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

	/*Recursive member function generates fully interconnected path through maze with no branching subpath
	  intersecting itself or its parent path other than at branching point*/
	void createPath(int activeRow, int activeCol)
	{
		int nRemainingDirections;
		const static Direction allDirections[4] = { {-2, 0}, {2, 0}, {0, -2}, {0, 2} }; //Up, down, left, right
		Direction direction, remainingDirections[4];
		int moveRow, moveCol;
		bool oob;

		maze2D[activeRow][activeCol] = MazeState::PATH;

		while (true) {
			nRemainingDirections = 0;

			//Find in-bounds directions in which a path has not yet been connected
			for (int i = 0; i <= 3; i++) {
				moveRow = activeRow + allDirections[i].deltaRow;
				moveCol = activeCol + allDirections[i].deltaCol;
				oob = !(moveRow >= 0 && moveRow <= nRows - 1 && moveCol >= 0 && moveCol <= nCols - 1);
				if (!oob && maze2D[moveRow][moveCol] == MazeState::UNASSIGNED)
					remainingDirections[nRemainingDirections++] = allDirections[i];
			}
			
			if (nRemainingDirections == 0) return; //All directions have a connected path
			else {
				direction = remainingDirections[rand() % nRemainingDirections];
				//Create path between active and adjacent position (removing preset wall)
				maze2D[activeRow + direction.deltaRow / 2][activeCol + direction.deltaCol / 2] = MazeState::PATH;

				//Recurse
				createPath(activeRow + direction.deltaRow, activeCol + direction.deltaCol);
			}
		}
	}
	
	//Recursive member function finds solution to maze
	bool findSolution(int activeRow, int activeCol)
	{
		const static Direction allDirections[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} }; //Up, down, left, right
		int moveRow, moveCol;
		bool oob;

		//Cycle through possible in-bounds moves along yet untraversed path
		for (int i = 0; i <= 3; i++) {
			moveRow = activeRow + allDirections[i].deltaRow;
			moveCol = activeCol + allDirections[i].deltaCol;
			oob = !(moveRow >= 0 && moveRow <= nRows - 1 && moveCol >= 0 && moveCol <= nCols - 1);
			if (oob) continue;
			if (maze2D[moveRow][moveCol] == MazeState::END) //Solution found?
				return true;
			else if (maze2D[moveRow][moveCol] == MazeState::PATH) {
				maze2D[moveRow][moveCol] = MazeState::TRAVERSED;
				if (findSolution(moveRow, moveCol)) {		//Recurse and mark path when solution is found
					maze2D[moveRow][moveCol] = MazeState::SOLUTION;
					return true;
				}
			}
		}

		return false; //Dead-end - no possible moves remain
	}
	   	 
	//Member function prints maze
	void printMaze()
	{
		COORD activeCell;
		DWORD nWriteChar;
		WORD attributes;

		for (int row = 0; row < nRows; row++) {
			for (int col = 0; col < nCols; col++) {
				activeCell = { static_cast<SHORT>(col), static_cast<SHORT>(row) };

				if (maze2D[row][col] == MazeState::WALL) {
					attributes = 255; //White letters on white background;
					cout << '#';
				}
				else if (maze2D[row][col] == MazeState::START) {
					attributes = BACKGROUND_GREEN | BACKGROUND_INTENSITY; //Black letters on green background;
					cout << 'S';
				}
				else if (maze2D[row][col] == MazeState::END) {
					attributes = BACKGROUND_RED | BACKGROUND_INTENSITY; //Black letters on red background;
					cout << 'E';
				}
				else {
					attributes = 15; //White letters on black background;
					cout << ' ';
				}

				WriteConsoleOutputAttribute(hStdout, &attributes, 1, activeCell, &nWriteChar);
			}
			cout << endl;
		}
	}

	//Member function prints solution to maze
	void printSolution() {
		COORD activeCell;
		DWORD nWriteChar;
		WORD attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; //Cyan letters on black background
		
		for (int row = 0; row < nRows; row++) {
			for (int col = 0; col < nCols; col++) {
				activeCell = { static_cast<SHORT>(col), static_cast<SHORT>(row) };

				if (maze2D[row][col] == MazeState::SOLUTION) {
					WriteConsoleOutputAttribute(hStdout, &attributes, 1, activeCell, &nWriteChar);
					WriteConsoleOutputCharacter(hStdout, "o", 1, activeCell, &nWriteChar);
				}
			}
		}
	}

	//Member function gets starting position of maze
	COORD getStartPos() {
		return { static_cast<SHORT>(startCol), static_cast<SHORT>(startRow) };
	}
};

bool isValidMove(COORD position, int nRows, int nCols);
void move(COORD& currentPos, COORD& nextPos);
void changeOldMark(COORD position);
void setNewMark(COORD position);
bool isSolved(COORD position);

int main()
{
	//Maze generator variables
	int nRows, nCols;
	int maxRows = 25, maxCols = 25;

	//Game display properties
	HWND consoleWindow;

	//Input event information for maze traversal
	INPUT_RECORD events[1], event;
	KEY_EVENT_RECORD keyEvent;
	DWORD nReadEvents;

	//Other maze traversal variables
	COORD currentPos, nextPos;
	bool newMaze = true;
	bool isArrowKey;

	//Game setup
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	consoleWindow = GetConsoleWindow();

	srand(static_cast<unsigned int>(time(NULL)));

	while (newMaze) {
		//Set up maze
		system("cls");

		cout << "Enter number of path rows (1-" << maxRows << "): ";
		cin >> nRows;
		while (true) {
			if (cin.fail() || nRows <= 0 || nRows > maxRows) { //Improper data type, overflow, or out-of-limits
				cout << "Invalid input. Re-enter number of path rows (1-" << maxRows << "): ";
				cin.clear();
				cin.ignore(32767, '\n');
				cin >> nRows;
			}
			else {
				cin.ignore(32767, '\n');
				break;
			}
		}

		cout << "Enter number of path columns (1-" << maxCols << "): ";
		cin >> nCols;
		while (true) {
			if (cin.fail() || nCols <= 0 || nCols > maxCols) { //Improper data type, overflow, or out-of-limits
				cout << "Invalid input. Re-enter number of path columns (1-" << maxCols << "): ";
				cin.clear();
				cin.ignore(32767, '\n');
				cin >> nCols;
			}
			else {
				cin.ignore(32767, '\n');
				break;
			}
		}

		system("cls");

		//Update from path rows/columns to total rows/columns (path + walls)
		nRows = nRows * 2 + 1;
		nCols = nCols * 2 + 1;

		Maze maze(nRows, nCols);
		currentPos = maze.getStartPos();

		//Gameplay
		while (true) {
			//Read event and process pressed arrow key
			ReadConsoleInput(hStdin, events, 1, &nReadEvents);
			event = events[0];
			if (event.EventType == KEY_EVENT) {
				keyEvent = event.Event.KeyEvent;
				if (keyEvent.bKeyDown) {
					isArrowKey = true;
					switch (keyEvent.wVirtualKeyCode) {
						case VK_RIGHT:
							nextPos = { currentPos.X + 1, currentPos.Y };
							break;
						case VK_LEFT:
							nextPos = { currentPos.X - 1, currentPos.Y };
							break;
						case VK_UP:
							nextPos = { currentPos.X, currentPos.Y - 1 };
							break;
						case VK_DOWN:
							nextPos = { currentPos.X, currentPos.Y + 1 };
							break;
						default: //Non-arrow key
							isArrowKey = false;
							break;
					}

					if (isArrowKey && isValidMove(nextPos, nRows, nCols)) { //Move position indicator?
						move(currentPos, nextPos);

						if (isSolved(currentPos)) { //Maze solved?
							maze.printSolution();
							newMaze = MessageBox(consoleWindow, "Maze solved!!!\n Replay?", "Maze Solved.", MB_YESNO | MB_ICONQUESTION) == IDYES ? true : false;
							break;
						}
					}
				}
			}
		}
	}

	return 0;
}

//Function checks if attempted move is valid (i.e., not into wall, out-of-bounds, or re-traced onto start position)
bool isValidMove(COORD position, int nRows, int nCols)
{
	bool oob = false;
	int row = static_cast<int>(position.Y);
	int col = static_cast<int>(position.X);
	TCHAR character;
	DWORD nReadChar;

	if (row < 0 || row > nRows - 1 || col < 0 || col > nCols - 1) oob = true;
	ReadConsoleOutputCharacter(hStdout, &character, 1, position, &nReadChar);

	return (character != '#' && character != 'S' && !oob);
}

//Function delegates move display operations and updates position indicator
void move(COORD& currentPos, COORD& nextPos)
{
	TCHAR character;
	DWORD nReadChar;

	ReadConsoleOutputCharacter(hStdout, &character, 1, currentPos, &nReadChar);
	if (character != 'S') changeOldMark(currentPos);

	ReadConsoleOutputCharacter(hStdout, &character, 1, nextPos, &nReadChar);
	if (character != 'E') setNewMark(nextPos);

	currentPos = nextPos;
}

//Function changes display of previous position indicator 
void changeOldMark(COORD position)
{
	DWORD nWriteChar;
	WORD attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; //Yellow letters on black background

	WriteConsoleOutputAttribute(hStdout, &attributes, 1, position, &nWriteChar);
	WriteConsoleOutputCharacter(hStdout, "o", 1, position, &nWriteChar);
}

//Function displays current position indicator
void setNewMark(COORD position)
{
	DWORD nWriteChar;
	WORD attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; //Cyan letters on black background


	WriteConsoleOutputAttribute(hStdout, &attributes, 1, position, &nWriteChar);
	WriteConsoleOutputCharacter(hStdout, "X", 1, position, &nWriteChar);
}

//Function tests if maze is solved
bool isSolved(COORD position)
{
	TCHAR character;
	DWORD nReadChar;

	ReadConsoleOutputCharacter(hStdout, &character, 1, position, &nReadChar);
	return (character == 'E');
}