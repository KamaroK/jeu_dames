#include "gameboard.h"

//Authors : Kevis KAMARO & Arnaud NSEGIYUMVA

enum colour
{
    P_RED, P_WHITE
};

struct client 
{
	int client_sock_fd;
	char username[20];
	enum colour col;
	int adversary_sock_fd;
	char *turn;
};

enum move_type
{
    ATTACK, NORMAL, INVALID
};

int send_move(char *shift, enum colour col_client, enum cell_contents board[][BOARDWIDTH])
{
  char * split = ",";
  int ystart = atoi(strsep(&shift, split));
  int xstart = atoi(strsep(&shift, split));
  int yend = atoi(strsep(&shift, split));
  int xend = atoi(shift);

  if( xstart>=0 && ystart>=0 && xend>=0 && yend>=0 && xstart<=BOARDWIDTH-1 && ystart<=BOARDWIDTH-1 && xend<=BOARDWIDTH-1 && yend<=BOARDWIDTH-1)
  {
	if( (board[ystart][xstart]==RED && col_client == P_RED && board[yend][xend]==EMPTY)
        ||  (board[ystart][xstart]==WHITE && col_client == P_WHITE && board[yend][xend]==EMPTY)
        ||  (board[ystart][xstart]==K_WHITE && col_client == P_WHITE && board[yend][xend]==EMPTY)
        ||  (board[ystart][xstart]==K_RED && col_client == P_RED && board[yend][xend]==EMPTY) )
		{
      		return 1;
    	}
    else if( (board[ystart][xstart]==RED && col_client == P_RED && board[yend][xend]==EMPTY && ( board[(ystart)-1][(xstart)+1]!=RED || board[(ystart)-1][(xstart)-1]!=RED ))
               ||  (board[ystart][xstart]==WHITE && col_client == P_WHITE && board[yend][xend]==EMPTY && ( board[(ystart)+1][(xstart)+1]!=WHITE || board[(ystart)+1][(xstart)-1]!=WHITE))
               ||  (board[ystart][xstart]==K_WHITE && col_client == P_WHITE && board[yend][xend]==EMPTY && ( board[(ystart)+1][(xstart)+1]!=RED || board[(ystart)+1][(xstart)-1]!=RED ))
               ||  (board[ystart][xstart]==K_RED && col_client == P_RED && board[yend][xend]==EMPTY && ( board[(ystart)-1][(xstart)+1]!=RED || board[(ystart)-1][(xstart)-1]!=RED )))
			   {
      				return 1;
    			}
  }
  printf("Not allowed! \n");
  return -1;
}



enum move_type is_valid_move(char *shift)
{
    char *split = ",";
    int ystart = atoi(strsep(&shift, split));
    int xstart = atoi(strsep(&shift, split));
    int yend = atoi(strsep(&shift, split));
    int xend = atoi(shift);

    if( (abs(xend-xstart)==1 && abs(yend-ystart)==1) )
	{
		return NORMAL;
    }
    else if( (abs(xend-xstart)==2 && abs(yend-ystart)==2) )
	{
		return ATTACK;
    }
    else
	{
  		return INVALID;
    }
}


void player_turn(struct client *client_fds, struct client current, char *shift)
{
	char input_msg[BUFFER_SIZE];
	char dep_initial[50];
	memset(dep_initial, '\0', 50);
	strcpy(dep_initial, shift);

	enum move_type current_move_type;

	do 
	{
		current_move_type = is_valid_move(shift);
		if(current_move_type == INVALID)
		{
        	memset(input_msg, '\0', BUFFER_SIZE);
			strcpy(input_msg,define_msg(input_msg, "10:", "This movement is not allowed! Please try again!"));
			if(send(current.client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
			{
				printf("client send error!\n");
			}
			memset(input_msg, '\0', BUFFER_SIZE);
			strcpy(input_msg, define_msg(input_msg, "4:", "true"));
			if(current.col == P_RED)
			{
				strcpy(input_msg, define_msg(input_msg, ",", "red"));
			}
			else
			{
				strcpy(input_msg, define_msg(input_msg, ",", "white")); 
			}
			if(send(current.client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
			{
				printf("client send error!\n");
			}
		}	
		else
		{
			if(current_move_type == NORMAL)
			{
				memset(input_msg, '\0', BUFFER_SIZE);
				strcpy(input_msg, define_msg(input_msg, "13:", "normal"));
				strcpy(input_msg, define_msg(input_msg, ":", dep_initial));

				if(current.col == P_RED)
				{
					strcpy(input_msg, define_msg(input_msg, ":", "red"));
				}
				else
				{
					strcpy(input_msg, define_msg(input_msg, ":", "white"));
				}
				printf("normal movement : %s\n", input_msg);

				if(send(current.client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
				{
					printf("client send error!\n");
				}	
				if(send(current.adversary_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
				{
					printf("client send error!\n");
				}
			}
			else
			{ 
				memset(input_msg, '\0', BUFFER_SIZE);
				strcpy(input_msg, define_msg(input_msg, "13:", "attack")); 
				strcpy(input_msg, define_msg(input_msg, ":", dep_initial));
				if(current.col == P_RED)
				{
					strcpy(input_msg, define_msg(input_msg, ":", "red"));
				}
				else
				{
					strcpy(input_msg, define_msg(input_msg, ":", "white"));
				}
				printf("attack shift : %s\n", input_msg);

				if(send(current.client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
				{
					printf("client send error!\n");
				}	
				if(send(current.adversary_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
				{
					printf("client send error!\n");
				}
			}
	
			memset(input_msg, '\0', BUFFER_SIZE);
			strcpy(input_msg, define_msg(input_msg, "10:", "Your movement is valid"));
			if(send(current.client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
			{
				printf("client send error!\n");
			}

			break;
		}
	} while (1);
}

void play_game(enum colour col_client, enum cell_contents board[][BOARDWIDTH], enum move_type movetype, char * shift)
{
	enum cell_contents temp_cell;
	char * split = ",";
	int ystart = atoi(strsep(&shift, split));
	int xstart = atoi(strsep(&shift, split));
	int yend = atoi(strsep(&shift, split));
	int xend = atoi(shift);

	switch (movetype) 
	{
		case NORMAL:
			temp_cell = board[ystart][xstart];
			board[ystart][xstart] = EMPTY;

			if(col_client==P_RED && yend==7)
			{
				board[yend][xend] = K_RED;
			}
			else if(col_client==P_WHITE && yend==0)
			{
				board[yend][xend] = K_WHITE;
			}
			else 
			{
				board[yend][xend] = temp_cell;
			}
			break;

		case ATTACK:
			temp_cell = board[ystart][xstart];
			board[ystart][xstart] = EMPTY;
			board[(ystart + yend) / 2][(xstart + xend) / 2] = EMPTY;

			if(col_client==P_RED && yend==7)
			{
				board[yend][xend] = K_RED;
			}
			else if(col_client==P_WHITE && yend==0)
			{
				board[yend][xend] = K_WHITE;
			}
			else
			{
				board[yend][xend] = temp_cell;
			}
			break;

		default:
			break;
	}
}


int test_for_winner(enum cell_contents board[][BOARDWIDTH], enum colour col)
{
	int i,j,resultat;
	resultat = 1;
	int xstart,ystart,xend,yend;
	char possible_move[50];

	for( i = 0; i <  BOARDHEIGHT; i++ )
	{
		for (j = 0; j < BOARDWIDTH; j++)
		{
			if(board[i][j]==RED && col==P_RED)
			{
				xstart = i;
				ystart = j;
				xend = i + 1;
				yend = j - 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i + 1;
				yend = j + 1;
				memset(possible_move, '\0', 50);;
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else{
					resultat = 0;
					return resultat;
				}

			}
			else if(board[i][j]==K_RED && col==P_RED)
			{
				xstart = i;
				ystart = j;
				xend = i + 1;
				yend = j - 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i + 1;
				yend = j + 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i - 1;
				yend = j - 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i - 1;
				yend = j + 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

			}else if(board[i][j]==WHITE && col==P_WHITE)
			{
				xstart = i;
				ystart = j;
				xend = i - 1;
				yend = j - 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i - 1;
				yend = j + 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

			}
			else if(board[i][j]==K_WHITE && col==P_WHITE)
			{
				xstart = i;
				ystart = j;
				xend = i - 1;
				yend = j - 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i - 1;
				yend = j + 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move,"%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i + 1;
				yend = j - 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}

				xstart = i;
				ystart = j;
				xend = i + 1;
				yend = j + 1;
				memset(possible_move, '\0', 50);
				sprintf(possible_move, "%d,%d,%d,%d", xstart, ystart, xend, yend);
				if (send_move(possible_move, col, board) == 1)
				{
					if(is_valid_move(possible_move) == INVALID)
					{
						resultat = 0;
					}
					return resultat;
				}
				else
				{
					resultat = 0;
					return resultat;
				}
			}
		}
	}
	return resultat;
}
