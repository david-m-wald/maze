#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <ctime>

#define GAME_WIDTH 80  //Max window width
#define GAME_HEIGHT 20 //Max window height

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

	/*Recursive member function generates random, fully interconnected path through maze with no branching subpath
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
					attributes = 119; //Gray letters on gray background;
					WriteConsoleOutputCharacter(hStdout, "#", 1, activeCell, &nWriteChar);
				}
				else if (maze2D[row][col] == MazeState::START) {
					attributes = BACKGROUND_GREEN | BACKGROUND_INTENSITY; //Black letters on green background;
					WriteConsoleOutputCharacter(hStdout, "S", 1, activeCell, &nWriteChar);
				}
				else if (maze2D[row][col] == MazeState::END) {
					attributes = BACKGROUND_RED | BACKGROUND_INTENSITY; //Black letters on red background;
					WriteConsoleOutputCharacter(hStdout, "E", 1, activeCell, &nWriteChar);
				}
				else {
					attributes = 15; //White letters on black background;
					WriteConsoleOutputCharacter(hStdout, " ", 1, activeCell, &nWriteChar);
				}

				WriteConsoleOutputAttribute(hStdout, &attributes, 1, activeCell, &nWriteChar);
			}
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

	//Member function gets ending position of maze
	COORD getEndPos() {
		return { static_cast<SHORT>(endCol), static_cast<SHORT>(endRow) };
	}
};

void setConsoleSizeAndPosition(int screenBWidth, int screenBHeight, int windowWidth, int windowHeight, CONSOLE_FONT_INFOEX font);
bool isValidMove(COORD position, int nRows, int nCols);
void move(COORD& currentPos, COORD nextPos, COORD scrollAhead);
void changeOldMark(COORD position);
void setNewMark(COORD position);
bool isSolved(COORD position);
void focusPosition(COORD position, int nRows, int nCols);

int main()
{
	//Maze generator variables
	int nRows, nCols;
	const int minRows = 8, minCols = 8; //Ensures screen buffer size > system minimum
	const int maxRows = 100, maxCols = 100; //Works for 2MB stack memory

	//Fixed game display properties
	HWND consoleWindow;
	LONG_PTR oldConsoleWindowStyle, newConsoleWindowStyle;
	CONSOLE_FONT_INFOEX oldFontInfo, newFontInfo;
	COORD fontSize = { 0, 16 };
	int fontWeight = 1000;
	CONSOLE_CURSOR_INFO cursor = { 25, false };

	//Input event information for maze traversal
	INPUT_RECORD events[1], event;
	KEY_EVENT_RECORD keyEvent;
	DWORD nReadEvents;

	//Other maze traversal variables
	COORD currentPos, nextPos, scrollAheadPos;
	bool newMaze = true, generateNew = false, restart = false, solved = false;
	bool isArrowKey, validInput;

	//Game setup
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	consoleWindow = GetConsoleWindow();

	oldConsoleWindowStyle = GetWindowLongPtr(consoleWindow, GWL_STYLE);
	newConsoleWindowStyle = oldConsoleWindowStyle ^ (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX); //Prevent resizing
	SetWindowLongPtr(consoleWindow, GWL_STYLE, newConsoleWindowStyle);

	oldFontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	GetCurrentConsoleFontEx(hStdout, false, &oldFontInfo);
	newFontInfo = oldFontInfo;
	newFontInfo.dwFontSize = fontSize;
	newFontInfo.FontWeight = fontWeight;
	SetConsoleCursorInfo(hStdout, &cursor);
	SetConsoleTitle("Maze");

	srand(static_cast<unsigned int>(time(NULL)));

	setConsoleSizeAndPosition(GAME_WIDTH, GAME_HEIGHT, GAME_WIDTH, GAME_HEIGHT, newFontInfo);
	cout << "Maze\n\n";
	cout << "Use the arrow keys to traverse the maze.\n\n";
	cout << "Special key commands:\n";
	cout << "  n = create new maze\n";
	cout << "  r = restart current maze\n";
	cout << "  c = refocus on current position\n";
	cout << "  e = focus on end position\n";
	cout << "  s = auto-solve maze\n";
	cout << "  q = quit\n\n";
	cout << "Press any key to start!";

	while (true) {
		//Read events and process first pressed key to start game
		ReadConsoleInput(hStdin, events, 1, &nReadEvents);
		if (events[0].EventType == KEY_EVENT) break;
	}

	while (newMaze) {
		//Set up maze
		system("cls");
		setConsoleSizeAndPosition(GAME_WIDTH, GAME_HEIGHT, GAME_WIDTH, GAME_HEIGHT, newFontInfo);

		cout << "Enter number of path rows (" << minRows << "-" << maxRows << "): ";
		cin >> nRows;
		while (true) {
			if (cin.fail() || nRows < minRows || nRows > maxRows) { //Improper data type, overflow, or out-of-limits
				cout << "Invalid input. Re-enter number of path rows (" << minRows << "-" << maxRows << "): ";
				cin.clear();
				cin.ignore(32767, '\n');
				cin >> nRows;
			}
			else {
				cin.ignore(32767, '\n');
				break;
			}
		}

		cout << "Enter number of path columns (" << minCols << "-" << maxCols << "): ";
		cin >> nCols;
		while (true) {
			if (cin.fail() || nCols < minCols || nCols > maxCols) { //Improper data type, overflow, or out-of-limits
				cout << "Invalid input. Re-enter number of path columns (" << minCols << "-" << maxCols << "): ";
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

		//Set screen buffer size to maze size and console window size to smaller of maze size or max window size
		setConsoleSizeAndPosition(nCols, nRows, nCols > GAME_WIDTH ? GAME_WIDTH : nCols, nRows > GAME_HEIGHT ? GAME_HEIGHT : nRows, newFontInfo);

		Maze maze(nRows, nCols);
		currentPos = maze.getStartPos();
		focusPosition(currentPos, nRows, nCols); //Auto-scroll to show start position

		//Gameplay
		while (true) {
			//Read event and process pressed arrow key or specified non-arrow keys
			ReadConsoleInput(hStdin, events, 1, &nReadEvents);
			event = events[0];
			if (event.EventType == KEY_EVENT) {
				keyEvent = event.Event.KeyEvent;
				if (keyEvent.bKeyDown) {
					isArrowKey = false;
					switch (keyEvent.wVirtualKeyCode) {
						case VK_RIGHT:
							isArrowKey = true;
							nextPos = { currentPos.X + 1, currentPos.Y };
							scrollAheadPos = { currentPos.X + GAME_WIDTH / 2, currentPos.Y };
							break;
						case VK_LEFT:
							isArrowKey = true;
							nextPos = { currentPos.X - 1, currentPos.Y };
							scrollAheadPos = { currentPos.X - GAME_WIDTH / 2, currentPos.Y };
							break;
						case VK_UP:
							isArrowKey = true;
							nextPos = { currentPos.X, currentPos.Y - 1 };
							scrollAheadPos = { currentPos.X, nextPos.Y - GAME_HEIGHT / 2 };
							break;
						case VK_DOWN:
							isArrowKey = true;
							nextPos = { currentPos.X, currentPos.Y + 1 };
							scrollAheadPos = { currentPos.X, nextPos.Y + GAME_HEIGHT / 2 };
							break;
						case 0x43: //C key - focus on current position
							focusPosition(currentPos, nRows, nCols);
							break;
						case 0x45: //E key - focus on end position
							focusPosition(maze.getEndPos(), nRows, nCols);
							break;
						case 0x4E: //N key - new maze
							generateNew = true;
							break;
						case 0x51: //Q key - quit program
							exit(0);
							break;
						case 0x52: //R key - restart maze
							restart = true;
							break;
						case 0x53: //S key - auto-solve maze
							solved = true;
							break;
						default:   //Other keys
							break;
					}

					if (generateNew) { //New maze?
						newMaze = true;
						generateNew = false;
						break;
					}

					if (restart) { //Restart maze?
						system("cls");
						maze.printMaze();
						currentPos = maze.getStartPos();
						focusPosition(currentPos, nRows, nCols); //Auto-scroll to show start position
						restart = false;
					}

					if (isArrowKey && isValidMove(nextPos, nRows, nCols)) { //Move position indicator?
						move(currentPos, nextPos, scrollAheadPos);
						solved = isSolved(currentPos);
					}

					if (solved) { //Maze solved?
						maze.printSolution();
						int result = MessageBox(consoleWindow, "Maze solved!!!\n Generate new maze?", "Maze Solved.", MB_YESNOCANCEL | MB_ICONQUESTION);
						if (result == IDYES)	 //Yes - new maze
							newMaze = true;
						else if (result == IDNO) //No - game over
							newMaze = false;
						else {					 //Cancel - solved maze can be scrolled
							validInput = false;
							while (!validInput) {
								//Read event and process pressed key to start new game or close program
								ReadConsoleInput(hStdin, events, 1, &nReadEvents);
								event = events[0];
								if (event.EventType == KEY_EVENT) {
									keyEvent = event.Event.KeyEvent;
									if (keyEvent.bKeyDown) {
										switch (keyEvent.wVirtualKeyCode) {
											case 0x4E: //N key - new maze
												newMaze = true;
												validInput = true;
												break;
											case 0x51: //Q key - quit program
												exit(0);
												break;
											default:   //Other keys
												break;
										}
									}
								}
							}
						}
						solved = false;
						break;
					}
				}
			}
		}
	}

	return 0;
}

//Function sets screen buffer size and console window size/position
void setConsoleSizeAndPosition(int screenBWidth, int screenBHeight, int windowWidth, int windowHeight, CONSOLE_FONT_INFOEX font)
{
	HWND consoleWindow, desktopScreen;
	RECT oldConsoleWindowRect, desktopScreenRect;
	int desktopScreenWidth, desktopScreenHeight, consoleWindowWidth, consoleWindowHeight;
	int newConsoleWindowPosX, newConsoleWindowPosY;
	COORD screenBSize = { static_cast<SHORT>(screenBWidth), static_cast<SHORT>(screenBHeight) };
	SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(windowWidth) - 1, static_cast<SHORT>(windowHeight) - 1};

	if (SetConsoleWindowInfo(hStdout, true, &windowSize))
		SetConsoleScreenBufferSize(hStdout, screenBSize);
	else {
		SetConsoleScreenBufferSize(hStdout, screenBSize);
		SetConsoleWindowInfo(hStdout, true, &windowSize);
	}
	SetCurrentConsoleFontEx(hStdout, false, &font); //Font set/reset here to ensure correct window size for screen resolution

	consoleWindow = GetConsoleWindow();
	desktopScreen = GetDesktopWindow();
	GetWindowRect(consoleWindow, &oldConsoleWindowRect);
	GetWindowRect(desktopScreen, &desktopScreenRect);
	desktopScreenWidth = desktopScreenRect.right - desktopScreenRect.left;
	desktopScreenHeight = desktopScreenRect.bottom - desktopScreenRect.top;
	consoleWindowWidth = oldConsoleWindowRect.right - oldConsoleWindowRect.left;
	consoleWindowHeight = oldConsoleWindowRect.bottom - oldConsoleWindowRect.top;
	newConsoleWindowPosX = desktopScreenWidth / 2 - consoleWindowWidth / 2;	  //Center window on screen
	newConsoleWindowPosY = desktopScreenHeight / 2 - consoleWindowHeight / 2; //Center window on screen
	SetWindowPos(consoleWindow, HWND_TOP, newConsoleWindowPosX, newConsoleWindowPosY, 0, 0, SWP_NOSIZE);
}

//Function checks if attempted move is valid (i.e., not into wall, out-of-bounds, or re-traced onto start position)
bool isValidMove(COORD position, int nRows, int nCols)
{
	bool oob;
	int row = static_cast<int>(position.Y);
	int col = static_cast<int>(position.X);
	TCHAR character;
	DWORD nReadChar;

	oob = row < 0 || row > nRows - 1 || col < 0 || col > nCols - 1;
	ReadConsoleOutputCharacter(hStdout, &character, 1, position, &nReadChar);

	return (character != '#' && character != 'S' && !oob);
}

//Function delegates move display operations and updates position indicator
void move(COORD& currentPos, COORD nextPos, COORD scrollAheadPos)
{
	TCHAR character;
	DWORD nReadChar;

	ReadConsoleOutputCharacter(hStdout, &character, 1, currentPos, &nReadChar);
	if (character != 'S') changeOldMark(currentPos);

	ReadConsoleOutputCharacter(hStdout, &character, 1, nextPos, &nReadChar);
	if (character != 'E') setNewMark(nextPos);

	SetConsoleCursorPosition(hStdout, scrollAheadPos);
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

/*Function auto-scrolls window to display given position with up to 50% window forward visibility ensured
  in all directions*/
void focusPosition(COORD position, int nRows, int nCols)
{
	//If scrolling by 50% fails, focus on edge of maze
	if (!SetConsoleCursorPosition(hStdout, { position.X + GAME_WIDTH / 2, position.Y }))  //Right
		SetConsoleCursorPosition(hStdout, { static_cast<SHORT>(nCols - 1), position.Y });
	if (!SetConsoleCursorPosition(hStdout, { position.X - GAME_WIDTH / 2, position.Y }))  //Left
		SetConsoleCursorPosition(hStdout, { 0, position.Y });
	if (!SetConsoleCursorPosition(hStdout, { position.X, position.Y + GAME_HEIGHT / 2 })) //Down
		SetConsoleCursorPosition(hStdout, { position.X, static_cast<SHORT>(nRows - 1) });
	if (!SetConsoleCursorPosition(hStdout, { position.X, position.Y - GAME_HEIGHT / 2 })) //Up
		SetConsoleCursorPosition(hStdout, { position.X, 0 });
}