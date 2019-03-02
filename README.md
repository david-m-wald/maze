# Maze

#### Maze game written in C++

<p align="center"><img src="https://imgur.com/v2c9jy6.gif"  width=700></p>

## Language

- C++

## Features

- Dynamic and fully randomized 2D maze generation supporting any combination of 8-100 traversable path rows and columns
- Maze traversal by pressing arrow keys with an active position marker and marking of traversed path
- Auto-sized and centered console window to display smaller mazes in full and larger mazes with scrolling enabled
- Auto-scrolling of larger mazes during traversal with up to 50% window width/height visibility in all directions
- Auto-solver
- Key commands to generate a new maze, restart an existing maze, auto-solve a maze, or auto-scroll larger mazes to focus on the end or current positions

## Installation

Run **Maze.exe** file included with the latest release

## Usage / Gameplay Instructions

When starting a new gameplay session, press any key to begin.

When creating a new maze, the player will be prompted to type in a desired number of traversable path rows and columns. The game supports any combination of 8-100 rows and 8-100 columns. The player will be notified if any input is invalid and then re-prompted for input.

A new maze will be randomly generated with traversable paths as black spaces, walls as gray spaces, the start position as a green space with an 'S', and the end position as a red space with an 'E'. For smaller mazes, the console window will be resized to show the full maze. For larger mazes, the console window will be sized no greater than a preset maximum with scrollbars, as needed. Upon generation, the window will be auto-scrolled, as necessary, to ensure that the start position is visible on screen and that a portion of the maze in each direction, if present, is also visible. Specifically, this initial maze visibility in each direction will be up to 50% of the window's width/height or up to the maze's edge.

Use the arrow keys to traverse the maze, beginning from the start position. Once the first move is made, the active position will be marked with a cyan-colored 'X'. Spaces on the traversed path will be marked with a yellow 'o'. In larger mazes with window scrollbars, the window will auto-scroll relative to the current position, as needed, to ensure that a portion of the maze in each direction, if present, is also visible. Like before, the maze visibility in each direction will be up to 50% of the window's width/height or up to the maze's edge. 

Upon reaching the end of the maze, the full correct path through the maze will be highlighted with cyan 'o's and the player will be prompted with a dialog box to replay or quit. To view the maze and solution path in depth, simply cancel out of the dialog box. To replay or quit after canceling out of the dialog box, press the 'n' or 'q' keys, respectively.

In addition to using the arrow keys for maze traversal, the following keys can be pressed while playing to execute special commands:

- n = create new maze
- r = restart current maze
- c = refocus on current position (notably useful when manually engaging with scrollbars for larger mazes)
- e = focus on end position
- s = auto-solve maze
- q = quit

## Potential Future Work

- Scoring system
- Preset maze sizes
- 3D mazes

## Version History

#### v1.0.0 -- March 2, 2019

- Initial release
- Dynamic 2D maze generation, traversal, and auto-solver
- Special key commands