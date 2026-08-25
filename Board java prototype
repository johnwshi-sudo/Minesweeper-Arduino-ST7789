import java.util.Random;

public class Board {

  private boolean gameOver;
  private int tilesRevealed;
  private int cols;
  private int mines;
  private int rows;
  private int[][] revealed_board;
  private int[][] hidden_board;
  private final static int MINE = -1;
  private final static int HIDDEN = -2;
  private final static int FLAG = -3;
  private final static int CLICKED_MINE = -4;
  private final static int REVEALED_MINE = -5;
//board constructor
 public Board (int rows, int cols, int mines) {

   gameOver = false;
   this.rows = rows;
   this.cols = cols;
   this.mines = mines;
   tilesRevealed = 0;
     
  hidden_board = new int [rows][cols];
  revealed_board = new int [rows][cols];
  initializeBoards();
 }

//generates both revealed and hidden boards.
  public void initializeBoards(){
    
  //generate revealed board
for (int r = 0; r < rows; r++){
  for (int c = 0; c < cols; c++){
  revealed_board[r][c] = HIDDEN;
      //sets hidden board as all empty. Mines & numbers get added later
  hidden_board[r][c] = HIDDEN;
  }
}

  }

//ensures safe first move and generates the hidden board
  public void firstMove(int row, int col){
        int chosenRow = row - 1;
        int chosenCol = col - 1;
        hidden_board[chosenRow][chosenCol] = 0;

  //generates hidden board & reveal tile
        placeMines(chosenRow,chosenCol);
        generateTileNumbers();
        revealTile(chosenRow + 1,chosenCol + 1);

      }
  
//generates mines for the hidden board
  public void placeMines(int chosenRow, int chosenCol){
        //randomizes mine location
    int placed = 0;
    while (placed < mines){
        int random_r = (int)(Math.random()*(rows));
        int random_c = (int)(Math.random()*(cols));
        //Skips the 3x3 area around the first click
      if (Math.abs(random_r - chosenRow) <= 1 && Math.abs(random_c - chosenCol) <= 1){
        continue;
      }
        //Places mines, no duplicates
      if (hidden_board[random_r][random_c] != MINE){

        hidden_board[random_r][random_c] = MINE;
        placed++;
      }
      }
  }

//generates tile numbers for the hidden board based on mine placements
  public void generateTileNumbers(){
    //Counts surrounding tiles for mines, gives tiles a number
      for (int r = 0; r < rows; r++){
        for (int c = 0; c < cols; c++){
        int counter = 0;
          //skip counting if tile is a mine
      if (hidden_board[r][c] == MINE){
        continue;
      }
      //counts mines in 3x3 of the chosen tile
         for (int row2 = -1; row2 < 2; row2++){
           for (int col2 = -1; col2 < 2; col2++){
          //skips tile its currently on (can't be a mine, so skip)
          if (col2 == 0 && row2 == 0){
        continue;
          }
          //track position on actual board
        int board_col = col2+c;
        int board_row = row2+r;
          // if position is valid on board & if mine present, add 1 to tile
          if (board_col >= 0 && board_row >= 0 && board_col < cols && board_row < rows && hidden_board[board_row][board_col] == MINE){
            counter++;
              }
           }
         }
          //finally sets the tile on the board to a number
    hidden_board[r][c] = counter;
          }
        }
  }
  
//Player chooses a tile to reveal
  public void revealTile(int row, int col){
    //fixes indices
    revealTileRecursive(row - 1, col - 1);
    } 

//Extension of reveal tile with correct indices
  public void revealTileRecursive(int row, int col){
    
      //Stop if tile is already revealed (avoids infinite recursion)
      if (revealed_board[row][col] != HIDDEN){
        return;
      }
          //Updates player's board
        revealed_board[row][col] = hidden_board[row][col];
        tilesRevealed++;
    
    //If mine, stop game
        if (hidden_board[row][col] == MINE){
            gameOver = true;
            return;
        }
    //If number, stop (already been revealed)
        if (hidden_board[row][col] > 0){
          return;
        }
    //If tile is an empty space (0), flood fill
        //Counts tiles in 3x3 of chosen tile, sees if they are 0
          for (int r = -1; r < 2; r++){
            for (int c = -1; c < 2; c++){
              //Actual position on the board
               int board_col = c + col;
               int board_row = r + row;

              //If position is valid in 3x3
                if (board_col >= 0 && board_row >= 0 && board_col < cols && board_row < rows){
                  
                  revealTileRecursive(board_row,board_col);
            }
          }
        }
    
  }

//Flags or unflags the desired tile
  public void flag(int row, int col){
    int chosenRow = row - 1;
    int chosenCol = col - 1;
    
    //If tile is a flag, then unflag
    if (revealed_board[chosenRow][chosenCol] == FLAG){
      revealed_board[chosenRow][chosenCol] = HIDDEN;
    }
    //If tile is hidden, flag it
    else if (revealed_board[chosenRow][chosenCol] == HIDDEN){
      revealed_board[chosenRow][chosenCol] = FLAG;
    }
    //If the tile is already revealed, don't place the flag
    else{
      System.out.println("Desired tile to flag is already revealed.");
      System.out.println();
    }
  }

//Reveals all tiles if the given number tile has flags equal to it
  public boolean chord(int row, int col){
    int chosenRow = row - 1;
    int chosenCol = col - 1;
    
    //if tile is not a number tile --> return
    if (revealed_board[chosenRow][chosenCol] <= 0){
      return false;
    }
    
      if (countAdjacentFlags(chosenRow,chosenCol) == revealed_board[chosenRow][chosenCol]){
        
        for (int r = -1; r < 2; r++){
          for (int c = -1; c < 2; c++){
            int boardRow = chosenRow + r;
            int boardCol = chosenCol + c;
            if ((r==0 && c==0) || boardRow < 0 || boardRow >= rows || boardCol < 0 || boardCol >= cols) {
              continue;
            }
            else{
              revealTile(boardRow + 1,boardCol + 1);
            }
            
      }
     }
        return true;
  }
    return false;
  }

//Counts flags within 3x3 of tile
  public int countAdjacentFlags(int row, int col){
    int flags = 0;
    for (int r = -1; r < 2; r++){
      for (int c = -1; c < 2; c++){
        int boardRow = row + r;
        int boardCol = col + c;
        if (boardRow < 0 || boardRow >= rows || boardCol < 0 || boardCol >= cols){
          continue;
        }
        else if(revealed_board[boardRow][boardCol] == FLAG){
          flags++;
        }
      }
    }
    return flags;
  }
  
//Highlights the mine that was clicked (loss)
  public void loseHighlightMine(int row, int col){
    revealed_board[row][col] = CLICKED_MINE;
  }

//Highlights all successfully avoided mines (win)
  public void winHighlightMines(){
    for (int row = 0; row < rows; row++){
      for (int col = 0; col < cols; col++){
        if (hidden_board[row][col] == MINE){
          revealed_board[row][col] = REVEALED_MINE;
        }
      }
    }

  }
  
//When the player loses, reveals all mines
  public void revealAllMines(){
    for (int row = 0; row < rows; row++){
      for (int col = 0; col < cols; col++){
          if (hidden_board[row][col] == MINE){
            revealed_board[row][col] = hidden_board[row][col];
          }
      }
    }
  }
  
//Getter for GameOver
  public boolean getGameOver(){
    return gameOver;
  }

//Getter for a specific tile on the revealed board
  public int getRevealedBoardTile(int row, int col){
  return revealed_board[row - 1][col - 1];
}
  
//Getter for tiles revealed to track a game win
  public int getTilesRevealed(){
    return tilesRevealed;
  }

  public void printRevealedBoard() {
    
    // Print column numbers
    System.out.print("  ");
    for (int c = 0; c < cols; c++) {
        System.out.print("\u001B[31m" + (c + 1) + " " + "\u001B[0m");
    }
    System.out.println();
    
    for (int r = 0; r < rows; r++) {
      
      System.out.print("\u001B[31m" + (r + 1) + " " + "\u001B[0m");
      
      for (int c = 0; c < cols; c++) {
        
        if (revealed_board[r][c] == HIDDEN){
          System.out.printf("□ ");
        } 
        else if (revealed_board[r][c] == MINE){
          System.out.print("💣");
        } 
        else if (revealed_board[r][c] == FLAG){
            System.out.print("🚩");
        }
            //Denotes the mine that the player hit
        else if (revealed_board[r][c] == CLICKED_MINE){
            System.out.print("💢");
        }
            //Denotes all mines after a win
        else if (revealed_board[r][c] == REVEALED_MINE){
          System.out.print("🟥");
        }
        else{
          System.out.print(revealed_board[r][c] + " ");
        }
      }
      System.out.println();
    }
  }
}
