import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        
//Asks for the board size
    Scanner scanner = new Scanner(System.in);
    int size;

    long startTime = System.currentTimeMillis();
        
    while (true){
        System.out.println("How big of a board do you want (3-15)?: ");
        size = scanner.nextInt();
        
        if (size >= 3 && size <= 15){
            break;
        }
        
        System.out.println("Size of the board must be between 3 and 15.");
    }

//Create board, keep track of game status (win/lose), error mitigation
        int mineCount = (int) Math.round(size*size*0.15);
        int totalTiles = (size*size) - mineCount;
        int chosenRow = 0;
        int chosenCol = 0;
        final int FLAG = -3;
        Board board = new Board(size, size, mineCount);
        boolean decision;
        boolean gameOver = false;
        boolean firstMove = true;

//FIRST MOVE
        
        System.out.println();
        board.printRevealedBoard();
        System.out.println();
        
    //Asks for a chosen row and column to reveal
        
        chosenRow = getValidRow(scanner,size);
        chosenCol = getValidCol(scanner,size);
        board.firstMove(chosenRow,chosenCol);
        
//MAIN GAME LOOP
        while (!gameOver){
            
            System.out.println();
            board.printRevealedBoard();
            System.out.println();

//Asks if the user wants to place a flag or reveal a tile
            
                decision = getFlagOrRevealDecision(scanner);
                chosenRow = getValidRow(scanner,size);
                chosenCol = getValidCol(scanner,size);
        
            if (decision){        //if user wnats to reveal...
                int tile = board.getRevealedBoardTile(chosenRow,chosenCol);
                if (tile == FLAG) {
                    System.out.println("Tile is a flag.");
                    System.out.println();
                    continue;
                }
                else if (tile >= 0 && tile <= 8){
                    boolean chorded = board.chord(chosenRow, chosenCol);
                    if (!chorded){
                        System.out.println("Not enough adjacent flags to chord.");
                        System.out.println();
                    }
                    
                    continue;
                    
                }
                
                //tile must be hidden now, so reveal
                board.revealTile(chosenRow,chosenCol);
                
            }

                
            else{
                board.flag(chosenRow,chosenCol);
            }
            
        //If total tiles is equal to tiles revealed, player wins
        if (board.getTilesRevealed() == totalTiles){
            break;
        }
        //update game status every loop to check if player hits mine
            gameOver = board.getGameOver();
        
        }

        
if (gameOver){
    board.revealAllMines();
    board.loseHighlightMine(chosenRow - 1,chosenCol - 1);
    System.out.println();
    board.printRevealedBoard();
    System.out.println();
    System.out.println("Game Over! You hit a mine!");
}
    
else{
    board.winHighlightMines();
    board.printRevealedBoard();
    System.out.println("YOU WON 😎");
}

long endTime = System.currentTimeMillis();
long gameTime = endTime - startTime;
System.out.println("Time: " + (gameTime/1000.0) + " seconds.");

    }

public static int getValidRow(Scanner scanner, int size){
    
    while (true){
        System.out.println("What row?: ");
        int chosenRow = scanner.nextInt();
        
        if (chosenRow >= 1 && chosenRow <= size){
            return chosenRow;
        }
            System.out.println("Row must be between 1 and the size of the board.");
            System.out.println();
    }    
}

public static int getValidCol(Scanner scanner, int size){

    while (true){
        System.out.println("What column?: ");
        int chosenCol = scanner.nextInt();

        if (chosenCol >= 1 && chosenCol <= size){
            return chosenCol;
        }
            System.out.println("Column must be between 1 and the size of the board.");
            System.out.println();
    }   
    
}

public static boolean getFlagOrRevealDecision(Scanner scanner){

    while (true){
        System.out.println("(r)eveal/chord or (f)lag/unflag?");
        String decision = scanner.next();
        if (decision.toUpperCase().equals("R")){
            return true;
        }
        else if (decision.toUpperCase().equals("F")){
            return false;
        }
            System.out.println("Please type either R or F.");
            System.out.println();
        }
    

    }
    
}
