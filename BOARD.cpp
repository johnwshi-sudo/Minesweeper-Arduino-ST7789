#include "BOARD.h"
#include <EEPROM.h>
static const int DIALOG_X = 45;
static const int DIALOG_Y = 55;
static const int DIALOG_W = 150;   // spans x: 45–195
static const int DIALOG_H = 100;   // spans y: 55–155 (title + clock/trophy only)

static const int BTN_X = 55;
static const int BTN_W = 130;
static const int BTN_H = 28;
static const int BTN_GAP = 8;
static const int PLAY_AGAIN_Y = 165;              // starts right after the panel
static const int EXIT_Y = PLAY_AGAIN_Y + BTN_H + BTN_GAP;   // = 201

Board::Board(int rows, int cols, int mines, int eepromAddress, Adafruit_ST7789 *display){   //constructor for revealed and hidden boards

  win = false;
  lose = false;
  this->rows = rows;
  this->cols = cols;
  this->mines = mines;
  tft = display;
  tilesRevealed = 0;
  minesRemaining = mines;

  int availableWidth  = 240 - (2 * SIDEBAR_WIDTH);
  int availableHeight = 240 - STATUS_BAR_HEIGHT;

  int tileW = availableWidth / cols;
  int tileH = availableHeight / rows;
  tilePixelSize = (tileW < tileH) ? tileW : tileH;  // pick the smaller size so board fits both dimensions

  cursorRow = (rows / 2) - 1;   //middle starting position will be top left of the middle 4 tiles
  cursorCol = (cols / 2) - 1;

  this->eepromAddress = eepromAddress;
  EEPROM.get(eepromAddress, fastestTime);      //initialize fastest time being a default high number
  if (fastestTime == 0xFFFFFFFF) {   // empty EEPROM reads as all 1s
    fastestTime = 99999999;
  }

  initializeBoards();
}

bool Board::tileInBounds(int row, int col){   //checks if a tile is within the board


      if (row >= 0 && col >= 0 && row < rows && col < cols){    //if within confines of board...
          return true;
      }
      return false;
}

void Board::initializeBoards(){   //sets all boards to hidden tiles

  for (int r = 0; r<rows; r++){
    for (int c = 0; c < cols; c++){

      hidden_board[r][c] = HIDDEN;
      revealed_board[r][c] = HIDDEN;
      }
    } 
}

void Board::firstMove(int row, int col){    // ensures the first move will be a 0, and will give a solid start
  int chosenRow = row - 1;
  int chosenCol = col - 1;
  hidden_board[chosenRow][chosenCol] = 0;   //first move is 0
  placeMines(chosenRow, chosenCol);   //generate board based off first move
  generateTileNumbers();
  revealTile(row, col);  //revealtile is 1-based

}

void Board::placeMines(int row, int col){   //randomly places mines
  int placedMines = 0;

  while (placedMines < mines){   //while not enough mines are placed...

  int random_c = random(cols);    //generate random tile
  int random_r = random(rows);
  if (abs(random_c - col) <= 1 && abs(random_r - row) <= 1){    //ensure tile is not within 3x3 of first move (gives player solid start)
    continue;
  }

  if (hidden_board[random_r][random_c] != MINE){    //ensures no repeat mines
    hidden_board[random_r][random_c] = MINE;   //places mine
    placedMines++;   
  }
  }
}

void Board::generateTileNumbers() {  //based off mine placements, generates tile numbers

  for (int r = 0; r < rows; r++){   //loop thru entire board
    for (int c = 0; c < cols; c++){

  if (hidden_board[r][c] == MINE){    //skip if tile is mine 
    continue;
  }
  int tileNumber = 0;

  for (int col = -1; col < 2; col++){   //loop thru 3x3 for each tile
    for (int row = -1; row < 2; row++){

      if (col == 0 && row == 0){  //skips tile its on, cant be a mine.
        continue;
      }
      int board_col = col + c;    //defines variables for tile position on board
      int board_row = row + r;

      if (tileInBounds(board_row,board_col) && hidden_board[board_row][board_col] == MINE){    //if surrounding tile is inside board and is a mine
        tileNumber++;
      }

   }

  }
  hidden_board[r][c] = tileNumber;   //sets tile number 
   }
  }
}

void Board::revealTile(int row, int col){   //reveals tile + flood fill
  revealTileRecursive( row - 1, col - 1);    //fixes indices

}

void Board::revealTileRecursive( int row, int col){   //flood fill and main revealing method. ensures that during floodfill indices dont keep losing -1

  if (!tileInBounds(row,col)){      //ensures inbound
    return;
  }

  if (lose || win || revealed_board[row][col] != HIDDEN) {    //abort the game if already ended (so if the game ends, other recursive calls stop), or if tile is already revealed/flagged
      return;
  }

  revealed_board[row][col] = hidden_board[row][col];    //reveal tile
  tilesRevealed++;
  drawTile(row, col);
  drawCursor();      //without this, the outline gets erased by drawTile()'s fillRect

  if (revealed_board[row][col] == MINE){    //if tile is mine, end game (lose condition)
    revealed_board[row][col] = CLICKED_MINE;
    drawTile(row,col);
    lose = true;
    return;
  }

  if (tilesRevealed == ((rows * cols) - mines)){      //checks win condition
    win = true;
    return;
  }

  if (revealed_board[row][col] > 0){    //if tile already revealed, stop (prevents normal numbers from going into floodfill)
  return;
  }

  for (int r = -1; r < 2; r++){     //revealed tile is a zero --> flood fill
    for (int c = -1; c < 2; c++){

      if (r==0 && c==0){    //skips the middle tile which is already a revealed 0
        continue;
      }

      if (tileInBounds(r+row,c + col)){   //If tile is within board, reveal.
        revealTileRecursive(r + row,c + col);   //wont reveal mines as 0 tiles have no mines surrounding, guarenteed.
      }

  }
  }

}

void Board::flag(int row, int col){   //flag or unflag

    int chosenRow = row - 1;
    int chosenCol = col - 1;

    if (revealed_board[chosenRow][chosenCol] == FLAG){    //if already flagged, unflag
    revealed_board[chosenRow][chosenCol] = HIDDEN;
    drawTile(chosenRow,chosenCol);
    drawCursor();      //without this, the outline gets erased by drawTile()'s fillRect
    minesRemaining++;
    drawFlagCount();      //updates flag counter
    }
    
    else if (revealed_board[chosenRow][chosenCol] == HIDDEN){    //if not flagged (hidden to user), flag
      revealed_board[chosenRow][chosenCol] = FLAG;
      drawTile(chosenRow,chosenCol);
      drawCursor();      //without this, the outline gets erased by drawTile()'s fillRect
      minesRemaining--;
      drawFlagCount();      //updates flag counter
    }

}

int Board::countAdjacentFlags(int row, int col){    //counts flags in 3x3, used for chord method
  int flags = 0; 
  for (int c = -1; c < 2; c++){   //3x3 area of chosen tile
    for (int r = -1; r < 2; r++){
        int board_row = r + row;
        int board_col = c + col;
        if (c == 0 && r == 0){    //skips chosen tile, cannot be flag
          continue;
        }
        if (tileInBounds(board_row,board_col) && revealed_board[board_row][board_col] == FLAG){   //if tile is within board and is flag
          flags++;
        }
    }
  }
  return flags;

}

bool Board::chord(int row, int col){    //chords (chooses revealed number tile, reveals all in 3x3 if flags within the 3x3 equal to revealed tile number)

  int chosenRow = row - 1;   //fixes indices to be 0 based
  int chosenCol = col - 1;

  if (revealed_board[chosenRow][chosenCol] <= 0){   //if tile is not revealed, end
   return false;
  }

  if (revealed_board[chosenRow][chosenCol] == countAdjacentFlags(chosenRow, chosenCol)){    //if surrounding flags = tile number (chording is available)

  for (int r = -1; r < 2; r++){   //checks 3x3 area
   for (int c = -1; c < 2; c++){
       int boardRow = r + chosenRow;
       int boardCol = c + chosenCol;

       if ((r==0 && c==0) || !tileInBounds(boardRow, boardCol)){   //skips if tile is out of bounds or is center tile
          continue;
        }
       else{   //reveals tile
         revealTile(boardRow + 1, boardCol + 1);   //+1 is to account for revealTile() recursive indice change
       }
   }
  }
  return true;
  }
  return false;
}

void Board::revealAllMines(){   //reveal all mines for when game ends.
    for (int r = 0; r < rows; r++){
        for (int c = 0; c < cols; c++){
          if (hidden_board[r][c] == MINE && revealed_board[r][c] != CLICKED_MINE){      //prevents overwriting of CLICKED_MINE in revealTileRecursive
                revealed_board[r][c] = hidden_board[r][c];
                drawTile(r,c);
            }
        }
    }
}

bool Board::getGameStatus(){    //getter to check if player wins or loses
  return (win || lose);
}

int Board::getTilesRevealed(){    //getter for total tiles revealed (used to check if game is won)
  return tilesRevealed;
}

int Board::getRevealedBoardTile(int row, int col){    //getter for specific tile.
  return revealed_board[row - 1][col - 1];
}

int Board::getMinesRemaining(){    //getter for mines remaining
  return minesRemaining;
}

int Board::getCursorCol(){    //getter for cursors column
  return cursorCol;
}

int Board::getCursorRow(){    //getter for cursor's row
  return cursorRow;
}

bool Board::getLose(){    //getter if player loses
  return lose;
}

void Board::drawTile(int row, int col){   //draws one tile

  int tile = revealed_board[row][col];    //variables to keep track of tile color and position on board. method will return 0-based indexes
  int color;
  bool isDark = (((row + col) % 2) == 0);

  int boardPixelWidth = cols * tilePixelSize;     //calculate perimeterSize (border width)
  int leftoverSpace = 240 - boardPixelWidth;
  int perimeterSize = leftoverSpace / 2;   // naturally >= SIDEBAR_WIDTH since tilePixelSize was capped using availableWidth

  int pixelX = perimeterSize + (col * tilePixelSize);
  int pixelY = STATUS_BAR_HEIGHT + (row * tilePixelSize);  

  if ((tile == HIDDEN || tile == MINE || tile == FLAG || tile == REVEALED_MINE) && isDark){    //gives color of BACKGROUND of the tile (revealed or hidden)
    color = HIDDEN_TILE_COLOR1;
  }
  else if ((tile == HIDDEN || tile == MINE || tile == FLAG || tile == REVEALED_MINE) && !isDark){
    color = HIDDEN_TILE_COLOR2;
  }
  else if(tile == 0){
    color = REVEALED_TILE_COLOR;
  }
  else if (tile == CLICKED_MINE){
    color = FLAG_COLOR;   //bright red
  }
  else {   // tile is a number tile
    color = REVEALED_NUMBER_TILE_COLOR;
  }

  if (lose && (tile == REVEALED_MINE || tile == MINE)){     //helps w/ reveal all mines method. slightly darker red than actual clicked mine
    color = DARK_RED;
  }

  else if (win && (tile == MINE || tile == REVEALED_MINE)){       //helps w/ reveal all mines method. all mines will be background bright green
    color = BRIGHT_GREEN;
  }

  tft->fillRect(pixelX,pixelY,tilePixelSize,tilePixelSize,color);   //fills background color of tile

  if (tile == MINE || tile == CLICKED_MINE || tile == REVEALED_MINE){      //draws MINE
    int centerX = pixelX + (tilePixelSize/2);
    int centerY = pixelY + (tilePixelSize/2);
    int r = tilePixelSize/4;    //radius
    int armLength = r + 3;              // spikes always poke ~3px past the circle edge
    int diag = armLength * 0.7;         // diagonal spikes proportionally shorter (true diagonal length)

    tft->fillCircle(centerX, centerY, r, MINE_COLOR);
    tft->drawLine(centerX, centerY - armLength, centerX, centerY + armLength, MINE_COLOR);        // vertical
    tft->drawLine(centerX - armLength, centerY, centerX + armLength, centerY, MINE_COLOR);        // horizontal
    tft->drawLine(centerX - diag, centerY - diag, centerX + diag, centerY + diag, MINE_COLOR);   // diagonal (\)
    tft->drawLine(centerX - diag, centerY + diag, centerX + diag, centerY - diag, MINE_COLOR);   // diagonal (/)

    }

  else if (tile == FLAG){     //draws FLAG
    double scale = (double) tilePixelSize / 27;   //scale for different difficulties
    int poleX = pixelX + 8 * scale;
    int poleBottom = pixelY + 21 * scale;
    int poleTop = pixelY + 5 * scale;
    int flagH = 6 * scale;
    int flagW = 10 * scale; 
    tft->drawLine(poleX, poleTop, poleX, poleBottom, MINE_COLOR);     //black flagpole
    tft->fillTriangle(poleX, poleTop, poleX, poleTop + (flagH * 1.5), poleX + flagW, poleTop + flagH/2, FLAG_COLOR);    //red flag triangle

  }

  else if (tile == HIDDEN || tile == 0){}   //leaves tile blank if hidden or 0

  else{   //writes NUMBER
  double scale = (double)tilePixelSize / 27;    //scale for different difficulties
    tft->setCursor(pixelX + 8 * scale, pixelY + 6 * scale);   //sets cursor to correct position
    tft->setTextSize((int)tilePixelSize / 9);
    static uint16_t numberColors[9] = {0, TILE1_COLOR, TILE2_COLOR, TILE3_COLOR,      //array to get access to tile colors easier/efficient. 0 index never gets used. static prevents constant reusing
    TILE4_COLOR, TILE5_COLOR, TILE6_COLOR, TILE7_COLOR, TILE8_COLOR};
    tft->setTextColor(numberColors[tile]);        //draws number of tile with correct color
    tft->print(tile);

  }
}

void Board::drawGrid(){   //draws the grid of the game. Unrevealed tiles alternate green shades, revealed tiles have two shades
  for (int r = 0; r < rows; r++){
    for (int c = 0; c < cols; c++){
      drawTile(r,c);
    }
  }
}

void Board::drawPerimeterAndStatusBar(){   //draws perimeter, the status bar of game, and flags remaining

    int boardWidth = cols * tilePixelSize;   //track coordinates
    int perimeterSideSize = (240 - boardWidth) / 2;

    tft->fillScreen(BORDER_COLOR);    //will get overlapped by board after

    int flagX = 55;   //draw flag
    int flagY = 13;

    tft->drawLine(flagX, flagY - 8, flagX, flagY, MINE_COLOR);    //draws flagpole
    tft->drawLine(flagX - 4, flagY, flagX + 4, flagY, MINE_COLOR);    //draws flag base
    tft->fillTriangle(flagX, flagY - 8, flagX, flagY - 4, flagX + 5, flagY - 6, FLAG_COLOR);    //draws actual flag

    tft->setCursor(flagX + 10, 2);    //denotes flags/mines left
    tft->setTextSize(2);
    tft->setTextColor(WHITE_COLOR);
    tft->print(getMinesRemaining());

    int clockX = 145;
    int clockY = 9;

    tft->fillCircle(clockX, clockY, 6, WHITE_COLOR);    //draws clock circle
    tft->drawLine(clockX, clockY, clockX, clockY - 4, MINE_COLOR);    //draws min hand
    tft->drawLine(clockX, clockY, clockX + 3, clockY, MINE_COLOR);    //draws hour hand
    //actual timer clock gets changed every second in(Board::drawTimer)
}

void Board::drawTimer(unsigned long elapsedTime){    //redraw only the timer once a new second has passed

    unsigned long seconds = elapsedTime / 1000;      //tracks time
    unsigned long tenths = (elapsedTime / 100) % 10;

    tft->fillRect(158, 1, 80, 15, BORDER_COLOR);  //erases old timer
    tft->setCursor(158,1);
    tft->setTextSize(2);
    tft->setTextColor(WHITE_COLOR);

    if (seconds < 10){    //does 01, 02, 03, etc for seconds less than 10.
      tft->print("0");
    }
    tft->print(seconds);
    tft->print(".");
    tft->print(tenths);

}

void Board::drawFlagCount(){      //tracks flags left 
    int flagX = 55;
    tft->fillRect(flagX + 10, 2, 40, 16, BORDER_COLOR);   // erase old number (wide enough for negatives too)
    tft->setCursor(flagX + 10, 2);
    tft->setTextSize(2);
    tft->setTextColor(WHITE_COLOR);
    tft->print(getMinesRemaining());
}

void Board::drawBoard(){   //prints the entire screen
  drawPerimeterAndStatusBar();
  drawGrid();
}

void Board::drawCursor(){   //draws the players cursor in game
  int boardPixelWidth = cols * tilePixelSize;
  int leftoverSpace = 240 - boardPixelWidth;
  int perimeterSize = leftoverSpace / 2;
  int pixelY = STATUS_BAR_HEIGHT + (tilePixelSize * cursorRow);   // now correctly refers to your class constant
  int pixelX = perimeterSize + (tilePixelSize * cursorCol);
  tft->drawRect(pixelX, pixelY, tilePixelSize, tilePixelSize, MINE_COLOR);

}

void Board::moveCursor(Direction dir){    //moves the players cursor in game

  int newRow = cursorRow;   //tracks the moved position
  int newCol = cursorCol;

  switch(dir){

    case LEFT:  newCol--; break;    //updates variables to actually reflect the moved position
    case RIGHT: newCol++; break;
    case UP:    newRow--; break;
    case DOWN:  newRow++; break;

  }

  if (!tileInBounds(newRow,newCol)){    //ignores moves that are out of bounds
    return;
  }

  drawTile(cursorRow,cursorCol);    //erases past cursor outline
  cursorRow = newRow;   //updates for next move
  cursorCol = newCol;
  drawCursor();   //draws new outline for new move

}

void Board::drawGameOverBox(GameOverOption o){      //draws specific playagain/exit box

  int btnY = (o == PLAY_AGAIN) ? PLAY_AGAIN_Y : EXIT_Y;   //changes button position based off which button
  tft->fillRect(BTN_X, btnY, BTN_W, BTN_H, LIGHT_BLUE_COLOR);     //main box
  tft->drawRect(BTN_X, btnY, BTN_W, BTN_H, GRAY);
  tft->setCursor(BTN_X + 30, btnY + 9);     //text
  tft->setTextColor(MINE_COLOR);
  tft->setTextSize(1);
  tft->print(o == PLAY_AGAIN ? "Play again" : "Exit");
}

void Board::drawGameOverSelectionOutline(GameOverOption selected){      //draws cursor outline during game over screen
  int btnY = (selected == PLAY_AGAIN) ? PLAY_AGAIN_Y : EXIT_Y;
  tft->drawRect(BTN_X, btnY, BTN_W, BTN_H, MINE_COLOR);
}

void Board::drawEndGameScreen(unsigned long elapsedTime){       //draws main box giving time, best time, loss/win, etc
  tft->fillRect(DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H, LIGHT_BLUE_COLOR);     //draw big box
  tft->drawRect(DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H, MINE_COLOR);

  // --- Title ---
  tft->setTextSize(2);
  tft->setTextColor(MINE_COLOR);
  tft->setCursor(DIALOG_X + 24, DIALOG_Y + 10);
  tft->print(win ? "You WIN" : "You lost");

  // --- Time clock (left) ---
  int clockX = DIALOG_X + 40, clockY = DIALOG_Y + 60;
  tft->fillCircle(clockX, clockY, 10, WHITE_COLOR);
  tft->drawCircle(clockX, clockY, 10, MINE_COLOR);
  tft->drawLine(clockX, clockY, clockX, clockY - 6, MINE_COLOR);
  tft->drawLine(clockX, clockY, clockX + 4, clockY, MINE_COLOR);

  unsigned long seconds = elapsedTime / 1000;      //tracks game time
  unsigned long tenths  = (elapsedTime / 100) % 10;
  tft->setTextSize(1);
  tft->setCursor(clockX - 12, clockY + 16);
  if (seconds < 10){
    tft->print("0");
  }
  tft->print(seconds); tft->print("."); tft->print(tenths);

  // --- Trophy + best time (right) ---
  int trophyX = DIALOG_X + 120, trophyY = clockY;
  tft->fillRect(trophyX - 4, trophyY + 9, 9, 2, GOLD_COLOR);
  tft->fillRect(trophyX - 2, trophyY + 7, 5, 2, GOLD_COLOR);
  tft->fillCircle(trophyX, trophyY, 8, GOLD_COLOR);
  tft->fillRect(trophyX - 8, trophyY - 8, 17, 8, GOLD_COLOR);
  tft->drawFastHLine(trophyX - 9, trophyY - 8, 19, GOLD_COLOR);
  tft->drawCircle(trophyX - 8, trophyY - 2, 3, GOLD_COLOR);
  tft->drawCircle(trophyX + 8, trophyY - 2, 3, GOLD_COLOR);
  tft->fillCircle(trophyX, trophyY, 6, WHITE_COLOR);
  tft->drawLine(trophyX, trophyY, trophyX, trophyY - 4, MINE_COLOR);
  tft->drawLine(trophyX, trophyY, trophyX + 3, trophyY, MINE_COLOR);

  tft->setCursor(trophyX - 12, trophyY + 16);   //sets cursor for trophy
  if (win && elapsedTime < fastestTime){        //uppdates fastest time
      fastestTime = elapsedTime;
      EEPROM.put(eepromAddress, fastestTime);
      unsigned long seconds = elapsedTime / 1000;
      unsigned long tenths  = (elapsedTime / 100) % 10;
      if (seconds < 10) tft->print("0");
      tft->print(seconds);
      tft->print(".");
      tft->print(tenths);
  }

  else{
        if (fastestTime == 99999999){     //if no record actually recorded yet print -
          tft->print("-");
        }
        else{     //if a recorded time, print the fastest
          unsigned long fastestSeconds = fastestTime / 1000;
          unsigned long fastestTenths  = (fastestTime / 100) % 10;
          if (fastestSeconds < 10) tft->print("0");
          tft->print(fastestSeconds);
          tft->print(".");
          tft->print(fastestTenths);

        }

  }

  // --- Buttons ---
  drawGameOverBox(PLAY_AGAIN);
  drawGameOverBox(EXIT);
}
