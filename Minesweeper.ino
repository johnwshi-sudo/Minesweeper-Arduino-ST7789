#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "board.h"
#include <EEPROM.h>
#define JOYSTICK_Y A5
#define JOYSTICK_X A4
#define JOYSTICK_BUTTON 3
#define TFT_CS  10
#define TFT_RST 8
#define TFT_DC  9
#define BUTTON  12

  Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

    Board*board = nullptr;    //no board can be created until difficulty selected

    unsigned static long gameStartTime;    //tracks start time
    unsigned long lastStatusUpdate = 0;   //tracks time to update by SECOND, not millisecond (used for game timer)
    unsigned long elapsedTime;     //tracks game time

    enum GameState{TITLE_SCREEN, DIFFICULTY_SELECTION, PLAYING, GAME_OVER};   //tracks different game states
    GameState state = TITLE_SCREEN;   //state object to keep track of state

    enum GameDifficulty{EASY, MEDIUM, HARD};   //tracks game difficulty
    GameDifficulty difficulty;

    GameOverOption gameOverOption = PLAY_AGAIN;       //default to play again

    bool boardCreated = false;    //ensures board will only be created once

  //basic joystick movement + button pressing
    bool joystickButtonWasPressed = false;    //safeguard for when player holds down joystick button
    unsigned long lastJoystickButtonPressTime = 0;
    const unsigned long JOYSTICK_DEBOUNCE_DELAY = 50;    //prevents joystick button bounce (undoing of a press in small time)
    bool joystickLeftWasTilted = false; bool joystickRightWasTilted = false; bool joystickUpWasTilted = false; bool joystickDownWasTilted = false; //safeguards for when player holds down joystick

    bool buttonWasPressed = false;    //safeguard for player holding button
    unsigned long lastButtonPressTime = 0;
    const unsigned long DEBOUNCE_DELAY = 100;    //prevents button bounce (undoing of a press in small time)


  //holding joystick dduring playing will move cursor faster automatically (key repeat)
    unsigned long directionHeldSince = 0;   //tracks time since user held joystick
    unsigned long lastAutoMove = 0; //tracks time since last automatic move occured
    const unsigned long HOLD_DELAY = 500;    // ms before auto-repeat starts
    const unsigned long REPEAT_INTERVAL = 150;  // ms between auto-repeat moves
    Direction lastHeldDirection = UP;        // placeholder, only meaningful while a direction is held
    bool directionIsHeld = false;

  //game safeguards
    bool titleScreenDrawn = false;    // title screen safeguard
    bool difficultyScreenDrawn = false;   // difficulty slection screen safeguard
    bool gameStarted = false;   //safeguard for game start
    bool firstTimeOnDifficultyScreen = true;    //helps to make the user default-hover over easy mode
    bool firstTimeOnPlayingScreen = true;   //helps track first time on playing
    bool firstTimeOnGameOverScreen = true;    //safeguard for game over screen
    bool firstMoveMade = false;      //tracks the first reveal tile

  //tracking cursor position during difficulty selection
    int difficultyPosition = 0;     //tracks which difficulty user is at (currently at easy)
    int previousDifficultyPosition = 0;   //tracks the previous difficulty the user WAS at beforehand.




void setup() {

  Serial.begin(9600);   //setup joystick
  pinMode(JOYSTICK_BUTTON,INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
  tft.init(240, 240);    //initialize screen
  tft.setRotation(2);   //rotate due to orientation
  randomSeed(analogRead(A0));      //ensures actual random board generation, not pseudoRNG

}

void loop() {


    int joystickX = analogRead(JOYSTICK_X);   //reads joystick input
    int joystickY = analogRead(JOYSTICK_Y);
    bool joystickUp = (joystickY < 300); bool joystickDown = (joystickY > 700); bool joystickLeft = (joystickX > 700); bool joystickRight = (joystickX < 300);    //helps ensure joystick holding wont count as multiple passes. one hold = one click. Note that joystick r/l and u/d are different due to wiring setup
    bool joystickUpJustTilted = joystickUp && !joystickUpWasTilted; bool joystickDownJustTilted = joystickDown && !joystickDownWasTilted; bool joystickLeftJustTilted = joystickLeft && !joystickLeftWasTilted; bool joystickRightJustTilted = joystickRight && !joystickRightWasTilted;
    joystickUpWasTilted = joystickUp; joystickDownWasTilted = joystickDown; joystickLeftWasTilted = joystickLeft; joystickRightWasTilted = joystickRight;

    bool buttonPressed = !digitalRead(BUTTON);
    bool buttonJustPressed = false;
    if (buttonPressed && !buttonWasPressed && (millis() - lastButtonPressTime >= DEBOUNCE_DELAY)){      //accounts for button held-down (registers as one press, not holding down) + bouncing
      buttonJustPressed = true;
      lastButtonPressTime = millis();
    };
    buttonWasPressed = buttonPressed;

    bool joystickButtonPressed = !digitalRead(JOYSTICK_BUTTON);   //pullup, therefore used !. true = pressed, false = not pressed
    bool joystickButtonJustPressed = false;
    if (joystickButtonPressed && !joystickButtonWasPressed && (millis() - lastJoystickButtonPressTime >= JOYSTICK_DEBOUNCE_DELAY)){     //similar logic to button above
      joystickButtonJustPressed = true;
      lastJoystickButtonPressTime = millis();

    }
    joystickButtonWasPressed = joystickButtonPressed;

switch(state){    //game pace
    case TITLE_SCREEN:
      {
      if (!titleScreenDrawn){   //draw title screen, safeguard.
        drawTitleScreen();
        titleScreenDrawn = true;    
      }

      if (joystickButtonJustPressed){   //press button to get to difficulty selection screen
        state = DIFFICULTY_SELECTION;
      }

      break;
      }

    case DIFFICULTY_SELECTION:
      {
      if (!difficultyScreenDrawn){    //safeguard
      drawDifficultyScreen();
      difficultyScreenDrawn = true;
      }

      static GameDifficulty Difficulties[3] = {EASY, MEDIUM, HARD};    //array so that user can go from hard directly to easy & vv.

      if (firstTimeOnDifficultyScreen){     //makes it so that the user hovers over easy on first time entering difficulty screen
          drawDifficultySelectionOutline(Difficulties[difficultyPosition], EASY);   
          firstTimeOnDifficultyScreen = false;
      }

      if (joystickRightJustTilted){   //tilting right will hover over the difficutly to the right
        previousDifficultyPosition = difficultyPosition;
        difficultyPosition++;
        if (difficultyPosition > 2){
          difficultyPosition = 0;
        }
        drawDifficultySelectionOutline(Difficulties[difficultyPosition], Difficulties[previousDifficultyPosition]);   //draw outline box + erase potential past box

      }

      else if (joystickLeftJustTilted){   //tilting left will hover over the difficutly to the left
        previousDifficultyPosition = difficultyPosition;
        difficultyPosition--;
        if (difficultyPosition < 0){
          difficultyPosition = 2;
        }
        drawDifficultySelectionOutline(Difficulties[difficultyPosition], Difficulties[previousDifficultyPosition]);   //draw outline box + erase potential past box

      }

      if (joystickButtonJustPressed){   //select difficulty and create board

        difficulty = Difficulties[difficultyPosition];
        if (difficulty == HARD){
          board = new Board(18,18,55,EEPROM_ADDRESS_HARD, &tft);
        }
        else if (difficulty == MEDIUM){
          board = new Board(12,12,25,EEPROM_ADDRESS_MEDIUM, &tft);
        }
        else{
          board = new Board(8,8,10,EEPROM_ADDRESS_EASY, &tft);
        }
        boardCreated = true;
        state = PLAYING;
      }

        break;
      }
    case PLAYING:
      {
      if (!gameStarted){
        gameStartTime = millis();   //time since arduino began running, not time since called. Only called once, therefore denotes time the game started.
        board->drawBoard();
        gameStarted = true;
      }

      elapsedTime = millis() - gameStartTime;

      if (elapsedTime - lastStatusUpdate >= 200){    //if 200ms has passed since last timer, update timer
          lastStatusUpdate = elapsedTime;
          board->drawTimer(elapsedTime);
      }

      if (firstTimeOnPlayingScreen){
          board->drawCursor();   //draws initial approx. middle tile for cursor to hover
          firstTimeOnPlayingScreen = false;
      }

      bool anyDirectionHeld = (joystickUp || joystickLeft || joystickRight || joystickDown);    //if held, anydirectionheld is true

      if (!anyDirectionHeld){
        directionIsHeld = false;      //reset holding if not held
      }

      if (joystickRightJustTilted){       //hovers the cursor to player's desired position, updates holding key repeat variables
          board->moveCursor(RIGHT);
          directionIsHeld = true;
          lastHeldDirection = RIGHT;
          directionHeldSince = millis();

      }

      else if (joystickLeftJustTilted){
          board->moveCursor(LEFT);
          directionIsHeld = true;
          lastHeldDirection = LEFT;
          directionHeldSince = millis();

      }

      else if (joystickUpJustTilted){
          board->moveCursor(UP);
          directionIsHeld = true;
          lastHeldDirection = UP;
          directionHeldSince = millis();
      }

      else if (joystickDownJustTilted){
          board->moveCursor(DOWN);
          directionIsHeld = true;
          lastHeldDirection = DOWN;
          directionHeldSince = millis();
      }

        else if (directionIsHeld && anyDirectionHeld){      //direction still being held
            unsigned long now = millis();
            if (now - directionHeldSince >= HOLD_DELAY &&     //if its been 500ms since ORIGINAL hold
                now - lastAutoMove >= REPEAT_INTERVAL){       //and its been 150ms since LAST MOVE
                  board->moveCursor(lastHeldDirection);     //key repeat
                  lastAutoMove = now;     //updates
                }

      }

      if (joystickButtonJustPressed){     //when player reveals
        
        if (!firstMoveMade){     //ensures first move is safe
          board->firstMove(board->getCursorRow() + 1, board->getCursorCol() + 1);
          firstMoveMade = true;
        }

        else{     //regular move
            board->chord(board->getCursorRow() + 1, board->getCursorCol() + 1);   //makes 0-based. Must be before revealTile.

            board->revealTile(board->getCursorRow() + 1, board->getCursorCol() + 1);    //makes 0-based
          
          if (board->getGameStatus()){
          state = GAME_OVER;    //checks if game over. This is the only way to change state since game only win/lose after revealing a tile

          break;
          }
        }
      }

      if (buttonJustPressed){     //when player flags
        board->flag(board->getCursorRow() + 1, board->getCursorCol() + 1);    //makes 0-based. Accounts for unflagging.
      }

      break;
      }

    case GAME_OVER:
       {

        if (firstTimeOnGameOverScreen) {      //first time conditions

          board->revealAllMines();      //accounts for win/lose visuals
          delay(2000);
          board->drawEndGameScreen(elapsedTime);
          board->drawGameOverSelectionOutline(PLAY_AGAIN);     //defaults to play again
            firstTimeOnGameOverScreen = false;
        }

        if (joystickDownJustTilted && gameOverOption == PLAY_AGAIN ){     //user wants to move to EXIT
            board->drawGameOverBox(PLAY_AGAIN);     //erases past
            board->drawGameOverSelectionOutline(EXIT);  
            gameOverOption = EXIT;
        }
        
        else if (joystickUpJustTilted && gameOverOption == EXIT){     //user wants to move to PLAY AGAIN
            board->drawGameOverBox(EXIT);     //erases past
            board->drawGameOverSelectionOutline(PLAY_AGAIN);
            gameOverOption = PLAY_AGAIN;
        }
        
        if (joystickButtonJustPressed){       //user decides

            if (gameOverOption == PLAY_AGAIN){      //if want to play again
              if (board != nullptr){      //gets rid of old boards
                  delete board;
                board = nullptr;
                  }
              difficultyScreenDrawn = false;      //resets safeguards
              firstTimeOnPlayingScreen = true;
              firstTimeOnDifficultyScreen = true;
              firstMoveMade = false;
              gameStarted = false;
              firstTimeOnGameOverScreen = true;
              lastStatusUpdate = 0;
              state = DIFFICULTY_SELECTION;     //set back to difficulty selection screen to replay
            }

            else{     //if want to exit
              tft.fillScreen(MINE_COLOR);   //black screen
              tft.writeCommand(0x28); // Turn off the visuals (ST77XX_DISPOFF) (supposedly, but maybe my st7789 is cheap and bad)
              tft.writeCommand(0x10);  // Put the ST7789 into sleep mode (ST77XX_SLPIN)
              while (true){     //put arduino into infinity loop (exit playing session)

              }
            }

        }   


        break;

    }
  }
}

void drawTitleScreen(){   //draws title screen


  for (int r = 0; r < 12; r++){   //creates a 12x12 background that alternates between dark and light green (same colors as hidden tiles)
    for (int c = 0; c < 12; c++){
      bool isDark = (((r + c) % 2) == 0);  //tracks alternation
      int x = 20*c;
      int y = 20*r;
      if (isDark){    //draws background grid
        tft.fillRect(x, y, 20, 20, HIDDEN_TILE_COLOR1);   
      }
      else{
        tft.fillRect(x, y, 20, 20, HIDDEN_TILE_COLOR2);
      }

    }
  }

  tft.setTextSize(3);   //creates the title
  tft.setTextColor(MINE_COLOR);
  tft.setCursor(21,90);   //middle of screen
  tft.print(F("MINESWEEPER"));

  tft.fillRect(80, 140, 93, 25, MINE_COLOR);   //creates the play button. Centers properly
  tft.setTextSize(1);
  tft.setTextColor(REVEALED_TILE_COLOR);
  tft.setCursor(87, 150);
  tft.print(F("PRESS TO PLAY"));
  
}

void drawDifficultyScreen(){    //draws the difficulty selection screen

  for (int r = 0; r < 9; r++){    //draws EASY background to match with easy difficulty (first 1/3 of screen)
    for (int c = 0; c < 3; c++){
      bool isDark = (((r + c) % 2) == 0);  //tracks alternation
      int x = c * 27;   //each tile is 27x27 pixels
      int y = r * 27;
      if (isDark){
        tft.fillRect(x, y, 27, 27, HIDDEN_TILE_COLOR1);   //greenish color for easy
      }
      else{
        tft.fillRect(x, y, 27, 27, HIDDEN_TILE_COLOR2);
      }
  }
  }
    drawDifficultyBox(EASY);    //draws easy box


  for (int r = 0; r < 16; r++){   //draws MEDIUM background to match w medium difficulty (second third of screen)
      for (int c = 0; c < 6; c++){
      bool isDark = (((r + c) % 2) == 0);  //tracks alternation
      int x = 80 + (c * 15);   //each tile is 15x15 pixels. adds 80 to account for first third of screen being for easy mode
      int y = (r * 15);
      if (isDark){
        tft.fillRect(x, y, 15, 15, MEDIUM_TILE_COLOR1);   //orangeish color for medium
      }
      else{
        tft.fillRect(x, y, 15, 15, MEDIUM_TILE_COLOR2);
      }
  }
  }
        drawDifficultyBox(MEDIUM);    //draws medium box

  for (int r = 0; r < 24; r++){   //draws HARD background to match w hard difficulty (last third of screen)
    for (int c = 0; c < 8; c++){
      bool isDark = (((r + c) % 2) == 0);  //tracks alternation
      int x = 160 + (c * 10);   //each tile is 10x10 pixels. adds 160 to account for past two thirds of screen being for easy & medium mode
      int y = (r * 10);
      if (isDark){
        tft.fillRect(x, y, 10, 10, HARD_TILE_COLOR1);   //redish color for hard
      }
      else{
        tft.fillRect(x, y, 10, 10, HARD_TILE_COLOR2);
      }
  }
  }
        drawDifficultyBox(HARD);    //draws hard box

}

void drawDifficultyBox(GameDifficulty d){   //draws the difficulty box for one difficulty

  int difficultyBoxY = 120;   //sizes for the box surrounding the difficulty word
  int difficultyBoxX = 10;
  int difficultyBoxW = 60;
  int difficultyBoxH = 15;
  int difficultyX = 28;   //sizes for the actual difficulty word
  int difficultyY = 123;

  if (d == EASY){   //draws easy box

    tft.fillRect(difficultyBoxX, difficultyBoxY, difficultyBoxW , difficultyBoxH , WHITE_COLOR);   //draws the "EASY" button
    tft.setCursor(difficultyX,difficultyY); tft.setTextColor(HIDDEN_TILE_COLOR1); tft.setTextSize(1); tft.print(F("EASY"));    //prints text "EASY"

  }

  else if (d == MEDIUM){    //draws medium box

    tft.fillRect(difficultyBoxX + 80, difficultyBoxY, difficultyBoxW , difficultyBoxH , WHITE_COLOR);    //draws the "MEDIUM" button
    tft.setCursor(difficultyX + 74 ,difficultyY); tft.setTextColor(MEDIUM_TILE_COLOR1); tft.setTextSize(1); tft.print(F("MEDIUM"));    //prints text "MEDIUM"

  }

  else{   //draws hard box

    tft.fillRect(difficultyBoxX + 160, difficultyBoxY, difficultyBoxW , difficultyBoxH , WHITE_COLOR);   //draws the "HARD" button
    tft.setCursor(difficultyX + 160,difficultyY); tft.setTextColor(HARD_TILE_COLOR1); tft.setTextSize(1); tft.print(F("HARD"));

  }


}

void drawDifficultySelectionOutline(GameDifficulty selected, GameDifficulty previous ){    //outlines the rectangle the user is hovering on during difficulty selection
  int difficultyBoxY = 120;   //sizes for the box surrounding the difficulty word
  int difficultyBoxX = 10;
  int difficultyBoxW = 60;
  int difficultyBoxH = 15;

    drawDifficultyBox(previous);    //erases past outline

  if (selected == EASY){
    tft.drawRect(difficultyBoxX, difficultyBoxY, difficultyBoxW , difficultyBoxH , MINE_COLOR);    //outlines easy

  }
  else if (selected == MEDIUM){
    tft.drawRect(difficultyBoxX + 80, difficultyBoxY, difficultyBoxW , difficultyBoxH , MINE_COLOR);    //outlines medium

  }
  else{
    tft.drawRect(difficultyBoxX + 160, difficultyBoxY, difficultyBoxW , difficultyBoxH , MINE_COLOR);   //outlines hard
 }

}
