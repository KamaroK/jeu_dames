#include<stdio.h>  
#include <stdlib.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <string.h>  
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/select.h>
#include <regex.h>
#include "login.h"
#include "game.h"
#include "constants.h"


//Authors : Kevis KAMARO & Arnaud NSEGIYUMVA


void confirm_sign_in(int server_sock_fd, char * recv)
{
	char recv_msg[BUFFER_SIZE];
	memset(recv_msg, '\0', BUFFER_SIZE);
	strcpy(recv_msg, recv);
	char * split = ",";
	char * sender = strsep(&recv, split);
	char * receiver = recv;
	char choice[5];
	char input_msg[BUFFER_SIZE];
	printf("%s wanna play with you! Enter 'yes' to agree, enter 'no' to refuse!\n", sender);
	fgets(choice, 5, stdin);
	if(strcmp(choice, "yes\n") == 0)
	{
		memset(input_msg, '\0', BUFFER_SIZE);
		strcpy(input_msg, define_msg(input_msg,"3:", "true"));
		strcpy(input_msg, define_msg(input_msg, ",", sender));
		strcpy(input_msg, define_msg(input_msg, ",", receiver));
		if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
		{
			printf("client send error!\n");
		}
	}
	else if(strcmp(choice, "no\n") == 0)
	{
		memset(input_msg, '\0', BUFFER_SIZE);
		strcpy(input_msg, define_msg(input_msg, "3:", "false"));
		strcpy(input_msg, define_msg(input_msg, ",", sender));
		strcpy(input_msg, define_msg(input_msg, ",", receiver));
		if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
		{
			printf("client send error!\n");
		}
	}
	else
	{
		printf("Illegal input!\n");
		confirm_sign_in(server_sock_fd, recv_msg);
	}

}


int main(int argc, const char * argv[])
{  
	struct sockaddr_in server_addr;  
	server_addr.sin_family = AF_INET;  
	server_addr.sin_port = htons(PORT);  
	server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");  
	memset(&(server_addr.sin_zero), '\0', 8);  

	int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
	if(server_sock_fd == -1)
	{  
		perror("socket error!");  
		return 1;  
	}  
	char recv_msg[BUFFER_SIZE];  
	char input_msg[BUFFER_SIZE]; 

	if(connect(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == 0)
	{  
		login(server_sock_fd);
		enum cell_contents gameboard[BOARDWIDTH][BOARDHEIGHT];
		init_gameboard(gameboard);

		fd_set client_fd_set;  
		struct timeval tv;  
		while(1)
		{  
			tv.tv_sec = 20;  
			tv.tv_usec = 0;  
			FD_ZERO(&client_fd_set);  
			FD_SET(STDIN_FILENO, &client_fd_set);  
			FD_SET(server_sock_fd, &client_fd_set);  

			select(server_sock_fd + 1, &client_fd_set, NULL, NULL, &tv);  
			if(FD_ISSET(STDIN_FILENO, &client_fd_set))
			{  
				memset(input_msg, '\0', BUFFER_SIZE);  
				fgets(input_msg, BUFFER_SIZE, stdin);  
				if(strcmp(input_msg, "quit") == 0)
				{
					close(server_sock_fd);
					exit(0);
				}
				if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
				{  
					perror("send error!\n");  
				}  
			} 

			if(FD_ISSET(server_sock_fd, &client_fd_set))  
			{  

				memset(recv_msg, '\0', BUFFER_SIZE);
				long byte_num = recv(server_sock_fd, recv_msg, BUFFER_SIZE, 0);  
				if(byte_num > 0)  
				{  
					if(byte_num > BUFFER_SIZE)  
					{  
						byte_num = BUFFER_SIZE;  
					}  
					char * recv;
					char * split=":";
					recv = recv_msg;
					int type = atoi(strsep(&recv,split));

					switch(type)
					{
						case REPONSE_LOGIN :
							if(strcmp(recv, "true") == 0)
							{
								printf("Welcome!\n");
							}
							else{
								printf("Username or password is incorrect! Please try again!\n");
								login(server_sock_fd);
							}
							break;
						case REPONSE_SIGN_UP :
							if(strcmp(recv, "true") == 0)
							{
								printf("Welcome!\n");
							}
							else
							{
								printf("Username has been used!\n");
								login(server_sock_fd);
							}
							break;

						case PROPOSE_CONNECT :
							printf("%s\n", recv);
							char choice_adversary[LEN];
							fgets(choice_adversary, LEN, stdin);
							if(choice_adversary[strlen(choice_adversary)-1] == '\n')
							{
								choice_adversary[strlen(choice_adversary)-1] = '\0';
							}
							if(strcmp(choice_adversary,"a") == 0)
							{
								printf("Please be patient... ...\n");
								break;
							}
							else
							{
								memset(input_msg, '\0', BUFFER_SIZE);
								strcpy(input_msg, define_msg(input_msg, "2:", choice_adversary));
								if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
								{
									printf("client send error!\n");
								}
								else
								{	
									printf("Your request was sent successfully! Please wait patiently... ...\n");
									printf("input_msg:%s\n", input_msg);
								}
								break;
							}

						case ASK_CONNECT_TO_CLIENT :
							confirm_sign_in(server_sock_fd,recv);
							break;

						case DECIDE_TURN :
							split=",";
							char is_turn[10];
							memset(is_turn, '\0', 10);
							strcpy(is_turn, strsep(&recv, split));
							char col[10];
							memset(col, '\0', 10);
							strcpy(col, recv);
							enum colour is_col;
							if(strcmp(col, "white") == 0)
							{
								is_col = P_WHITE;
							}
							else
							{
								is_col = P_RED;
							}
							
							if(strcmp(is_turn,"true")==0)
							{
								printf("Your color is %s\n", col);
								printf("It's your turn!\n"); 
								int x1=-1;
								int x2=-1;
								int y1=-1;
								int y2=-1;
								char shift[50];
								int status = -1;
								do
								{  
									printf("Enter your move:'line 1,column 1- line 2,column 2'\n");
									memset(shift, '\0', 50);
										if(scanf("%d,%d-%d,%d", &x1, &y1, &x2, &y2) == 4)
										{
											sprintf(shift,"%d,%d,%d,%d",x1,y1,x2,y2);
											status = send_move(shift,is_col,gameboard);
										}
										else
										{
											scanf("%s",shift);
											if(strcmp(shift, "quit") == 0)
											{
												close(server_sock_fd);
												exit(0);
											}
										}
										if(status == 1)
										{
											memset(input_msg, '\0', BUFFER_SIZE);
											sprintf(shift, "%d,%d,%d,%d", x1, y1, x2, y2);
											strcpy(input_msg, define_msg(input_msg, "12:", shift));
											if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
											{
												printf("client send error!\n");
											}
											break;
										}
										else
										{
											printf("Illegal action! Please remove!\n");
										}
									}while(status==-1);
								}

								else
								{
									split = ",";
									char col[10];
									memset(col, '\0', 10);
									strcpy(col, recv);
									printf("Your color is %s\n", col);
									printf("Your adversary is moving! Please wait patiently... ...\n");
								}
							break;

						case DISPLAY :
							printf("show:%s\n",recv);
							break;
						case START :{
							display_gameboard(gameboard);
							break;
						}
						case PLAY :{
							if(strcmp("normal", strsep(&recv, split)) == 0)
							{
								char shift[LEN];
								memset(shift, '\0', LEN);
								strcpy(shift, strsep(&recv, split));
								if(strcmp(recv, "red") == 0)
								{
									play_game(P_RED, gameboard, NORMAL, shift);
								}
								else
								{
									play_game(P_WHITE, gameboard, NORMAL, shift);
								}
							}
							else
							{    
								char shift[LEN];
								memset(shift, '\0', LEN);
								strcpy(shift, strsep(&recv, split));
								if(strcmp(recv,"red") == 0)
								{
									play_game(P_RED, gameboard, ATTACK, shift);
								}
								else
								{
									play_game(P_WHITE, gameboard, ATTACK, shift);
								}
							}

							display_gameboard(gameboard);
							break;
						}
						default:
							printf("default:%s\n",recv);
					}

				}  
				else if(byte_num < 0)  
				{  
					printf("reception error!\n");  
				}  
				else  
				{  
					printf("server exit!\n");  
					exit(0);  
				}  
			} 
		}    
	}  

	return 0;  
} 
