#ifndef BOARD_H
#define BOARD_H

#include <Adafruit_ST7789.h>

#define TILE1_COLOR 0x001F
#define TILE2_COLOR 0x0400
#define TILE3_COLOR 0xF800
#define TILE4_COLOR 0x0010
#define TILE5_COLOR 0x8000
#define TILE6_COLOR 0x0410
#define TILE7_COLOR 0x0000
#define TILE8_COLOR 0x8410
#define MINE_COLOR 0x0000
#define FLAG_COLOR 0xF800
#define HIDDEN_TILE_COLOR1 0xA72F
#define HIDDEN_TILE_COLOR2 0x96EE
#define REVEALED_TILE_COLOR 0xF796
#define REVEALED_NUMBER_TILE_COLOR 0xE72D
#define BORDER_COLOR 0x0320
#define MEDIUM_TILE_COLOR1 0xFD68
#define MEDIUM_TILE_COLOR2 0xEC46
#define HARD_TILE_COLOR1 0xF808
#define HARD_TILE_COLOR2 0xD805
#define WHITE_COLOR 0xFFFF
#define BRIGHT_GREEN 0x07E0
#define DARK_RED 0x8800
#define GRAY 0x8410
#define GOLD_COLOR 0xFFE0
#define LIGHT_BLUE_COLOR 0x5D1F   
#define MAX_SIZE 20
#define EEPROM_ADDRESS_EASY   0
#define EEPROM_ADDRESS_MEDIUM 4
#define EEPROM_ADDRESS_HARD   8


 enum Direction { UP, DOWN, LEFT, RIGHT };    //sets directions for cursor movement

 enum GameOverOption{PLAY_AGAIN, EXIT};      //tracks game over options
 
class Board {

public:

 Board (int rows, int cols, int mines, int eepromAddres, Adafruit_ST7789 *display);      //constructor

 void initializeBoards();  //sets both hidden and revealed boards to all 0's

 void firstMove(int row, int col);     //ensures first move is safe and has playable starting position

 void placeMines(int chosenRow, int chosenCol);       //RNG mine placement

 void generateTileNumbers();         //places number tiles based on random mine placements

 void revealTile(int row, int col);    //fixes indices 

 void revealTileRecursive(int row, int col);    //reveals tile on 2d arrays + floodfill

 void flag(int row, int col);       //flag and unflag with graphics

 bool chord(int row, int col);         //chord reveal

 int countAdjacentFlags(int row, int col);         //helper for chording

 void revealAllMines();       //game end screen, reveal all minnes

 void drawTile(int row, int col);         //draws a specific tile

 void drawGrid();       //graws the gameboard

 void drawPerimeterAndStatusBar();     //draws perimeter and status bar of game

 void drawTimer(unsigned long elapsedTime);     //updates timer in status bar

 void drawFlagCount();        //updates flag count in status bar

 void drawBoard();         //draws entire screen

 void drawCursor();        //draws cursor outline on tile 

 void moveCursor(Direction dir);       //moves outline of cursor

 void drawGameOverBox(GameOverOption o);        //draws main game over BOX

 void drawGameOverSelectionOutline(GameOverOption selected);         //draws cursor outline during game over

 void drawEndGameScreen(unsigned long elapsedTime);         //draws the game over screen

 bool tileInBounds(int row, int col);     //checker to see if a tile is within bounds

 bool getGameStatus();     //getter for if the player loses or wins

 int getMinesRemaining();     //getter for mines remaining

 int getCursorRow();       //getter for cursors row

 int getCursorCol();       //getter for cursors column

private:

   Adafruit_ST7789 *tft;
   unsigned long fastestTime;
   int tilesRevealed;
   int cols;
   int mines;
   int rows;
   int minesRemaining;
   int tilePixelSize;      //tile size in pixels
   int cursorCol;          //tracks cursor position
   int cursorRow;
   int eepromAddress;     //tracks fastest time - specific to difficulty
   bool win;
   bool lose;
   int8_t revealed_board[MAX_SIZE][MAX_SIZE];
   int8_t hidden_board[MAX_SIZE][MAX_SIZE];
   const static int MINE = -1;
   const static int HIDDEN = -2;
   const static int FLAG = -3;
   const static int CLICKED_MINE = -4;
   const static int REVEALED_MINE = -5;
   const static int STATUS_BAR_HEIGHT = 30;
   const static int SIDEBAR_WIDTH = 10;

};

#endif
