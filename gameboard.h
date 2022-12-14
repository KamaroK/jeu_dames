#define BOARDWIDTH 8
#define BOARDHEIGHT  8
#define WHITE_DISPLAY "\x1b[39m" "o"
#define WHITE_KING_DISPLAY "\x1b[39m" "O"
#define RED_DISPLAY "\x1b[31m" "o"
#define RED_KING_DISPLAY "\x1b[31m" "O"
#define WHITE_RESET "\x1b[39m"

//Authors : Kevis KAMARO & Arnaud NSEGIYUMVA

enum cell_contents
{
    EMPTY, RED, WHITE, K_RED, K_WHITE
};


static const enum cell_contents master_board[BOARDHEIGHT][BOARDWIDTH] =
{
    {RED, EMPTY, RED, EMPTY, RED, EMPTY, RED, EMPTY},
    {EMPTY, RED, EMPTY, RED, EMPTY, RED, EMPTY, RED},
    {RED, EMPTY, RED, EMPTY, RED, EMPTY, RED, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, WHITE, EMPTY, WHITE, EMPTY, WHITE, EMPTY, WHITE},
    {WHITE, EMPTY, WHITE, EMPTY, WHITE, EMPTY, WHITE, EMPTY},
    {EMPTY, WHITE, EMPTY, WHITE, EMPTY, WHITE, EMPTY, WHITE}
};


void init_gameboard(enum cell_contents board[][BOARDWIDTH])
{
  int i,j;
  for(i = 0;i<BOARDHEIGHT;i++)
  {
    for (j = 0; j < BOARDHEIGHT; j++ ) 
    {
      board[i][j] = master_board[i][j];
    }
  }
}


void display_gameboard(enum cell_contents board[][BOARDWIDTH])
{
  int i,j;

  for(i = 0;i <= BOARDHEIGHT;i++)
  {
    for (j = 0; j <= BOARDWIDTH; j++ ) 
    {
      if(i == 0 && j == 0)
      {
        printf("     |");
      } 
      else if(i == 0)
      {
        printf("  %d  |", j - 1);
      }
      else if(j == 0)
      {
        printf("  %d  |", i - 1);
      }
      else 
      {
        printf("  ");
        switch (board[i-1][j-1]) 
        {
          case EMPTY:
            printf(" ");
            break;
          case RED:
            printf(RED_DISPLAY);
            break;
          case WHITE:
            printf(WHITE_DISPLAY);
            break;
          case K_RED:
            printf(RED_KING_DISPLAY);
            break;
          case K_WHITE:
            printf(WHITE_KING_DISPLAY);
            break;
          default:
            break;
        }
        printf(WHITE_RESET);
        printf("  |");
      }
    }
    printf("\n------------------------------------------------------\n");
  }

}


