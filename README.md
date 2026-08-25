# Minesweeper-Arduino-ST7789

A fully playable Minesweeper adapted from a Java prototype, built from scratch in C++ for Arduino, with a 240x240 pixel ST7789, a joystick for revealing, and a button for flagging.

![Demo](https://github.com/user-attachments/assets/c7aec5dd-d49c-4946-8f03-675d48a28dec)

## Features
- Three difficulty levels (Easy 8x8, Medium 12x12, Hard 18x18)
- Joystick-based cursor movement with key-repeat on hold
- Flag / unflag tiles, chording on revealed numbers
- Safe first-click guarantee (no mine within 3x3 of first move)
- Recursive flood-fill reveal for empty areas
- All-time record win-time tracking per difficulty via EEPROM
- Custom-rendered UI: title screen, difficulty select, in-game HUD (timer, flag count), win/loss screen

## Hardware

| Components            | Notes                                        |
| --------------------- | -------------------------------------------- |
| Arduino UNO (1)       |                                              |
| ST7789 (TFT) (240x240)| SPI pins: RES=8, DC=9, CS=10, SDA=11, SCL=13 |
| Joystick (1)          | X=A5, Y=A4, Button=3                         |
| Push Button (1)       | Pin 12                                       |
| 220Ω resistors (5)    | Current-limiting for 5V Arduino → 3.3V logic |

### Wiring Diagram
![Wiring](https://github.com/user-attachments/assets/94c6070d-d948-4437-83c2-b1f9d3a2e467)

## Screenshots & Gameplay
* [Hard Mode Completion Video](https://drive.google.com/file/d/1Q4zkYb6TBOvhrEZJSIVUQOV5J79SxS5L/view?usp=drive_link) — (18x18 board, 55 mines)
* [Easy Speedrun Video](https://github.com/user-attachments/assets/db2dd8ee-5442-4bc7-8f10-e168078552f7) — (8x8 board, 10 mines)

| Mid-Game Screen | Win Screen | Loss Screen |
| :---: | :---: | :---: |
| ![Mid-game screen](https://github.com/user-attachments/assets/357c1b3d-4bbc-40f9-807f-ff042e155613) | ![Win screen](https://github.com/user-attachments/assets/20689a98-8787-461d-860d-31de5a95fc98) | ![Loss screen](https://github.com/user-attachments/assets/faba0bb6-5c75-4d81-8d2d-1c3b3855163a) |

## How It Works
The main game loop relies on a switch-case block containing the different stages of the game (title screen → difficulty selection → playing → game over). Each case calls a method to draw the visual screen, as well as many variables keeping track of player movement (joystick movement and button movement) with respective safeguards to ensure holding down doesn't count as multiple presses. Additionally, key repeat was implemented for joystick movement, and button bounce safeguards were also included.

The game begins by asking the user to select a difficulty (board size), and once they do, two 2D-array boards are created: one fully completed hidden one, and the board the user sees. Both are initially blank... Until the user makes their first move, where a special `firstMove()` method is called which forces the 3x3 area of the first move to be 0's (to be safe), randomly place mines, and generates respective number tiles for the entire board. From here, the `drawTile()` method reads info from the revealed board 2D-array and draws the specific tile for its position, denoting a specific background and icon for the tile. Lastly, the perimeter and top status bar of the game are drawn and keep track of time + flags remaining.

While playing, the player's joystick cursor position is constantly tracked, outlined, and erased. A click on the joystick button reveals a tile, calls `revealTile()`, which then calls `revealTileRecursive()`. This method safely updates the revealed 2D array (and calls `drawTile`), as well as tracking a loss (hits mine) or win (tiles revealed equals to non-mine tiles). It also, if valid, implements a flood-fill algorithm by searching the 3x3 area within the player's move, and calling itself for each of those tiles. The player can choose to flag by pressing the other, non-joystick button. That'll call the `flag()` method to update the 2D array, draw the flag, and reduce the flag counter. Additionally, the player can chord, which is to reveal a 3x3 area surrounding a chosen tile if the number of flags around it is equal to the number of the tile. This is done by determining if `countAdjacentFlags()` is equal to the tile number, and if so, `revealTile()` for all surrounding tiles.

A special screen is played depending on the player's win or lose, which calls `revealAllMines()` and colors their backgrounds either green or red using `drawTile()`. Additionally, it displays both game time and all-time fastest WIN time. The general game time was done by calling multiple `millis()`, as well as a `lastStatusUpdate` variable, which only updates the game timer every 0.2 seconds, rather than every call to `millis()`. Multiple EEPROM memory addresses had to be declared to track all-time time for each difficulty. All visuals had both delicate and tedious math to compute and scale-to-fit all board sizes, while also updating all "Draw" methods. Overall, this minesweeper deals with heavy indices logic from numerous traversals, simple arrays, recursion, method calls from other files, and other methods.

## Origin
Before building the hardware version, I prototyped the core game logic in Java as a text-based version. See `main.java` and `board.java` — same underlying logic (recursive flood-fill, chording, flag-toggle).

## What I Learned / Challenges
- Fitting two 20x20 boards in limited SRAM + RAM tracking showed me how RAM affects the upload and game speed, as well as to constantly think of memory allocation and efficiency for a program.
- Debouncing a mechanical joystick + state change detection for a button forced me to think of the constant loop block instead of one instantaneous input. These functions originated from bugs where holding would flag/unflag a tile many times a second (causing visual glitches) or a flag would unflag itself due to button bouncing.
- The ST7789 was my first look onto GUI. I learned to cleanly declare variables for visuals, update for different difficulties (board sizes), and the TFT's Adafruit library.
- Following game flow with a switch-case showed me how to use numerous safeguards and how to work-around file scope issues. I now know how to communicate between files with shared scope, variables, and method calls within other methods.
- Indices were a very big issue, as methods and calls were inconsistent with either 1 or 0-based indices, leading to an incredible amount of errors, and therefore had to all be 0-based.
- Deeper traversal logic into arrays, especially with the recursive flood fill method, is where I fully understood the chain of recursion and its effect on RAM as opposed to an iterative flood fill. These topics caused me to create a hard `tileInBounds()` safeguard for when tiles drew outside the board.
- Live user input required me to track both internal game status with 2D arrays, as well as visually with constant tile drawing, constant cursor movement and erasing, and live timer + flag counters.
- Learned C++ syntax/file structure, how to format code to include comments for public viewing, and debug searching.

## Build It Yourself
1. Wire according to the "Hardware" section
2. Install libraries: `Adafruit_GFX`, `Adafruit_ST7789`
3. Flash `Minesweeper.ino` to your board

## License
MIT
