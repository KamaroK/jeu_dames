#include <string.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/select.h>
#include <errno.h> 
#include <time.h> 
#include "login.h"
#include "game.h"
#include "constants.h"


//Authors : Kevis KAMARO & Arnaud NSEGIYUMVA

  
struct client client_fds[CONCURRENT_MAX];


void refresh_waiting_list(struct client client_fds[], int size)
{
	char * client_liste = calloc(BUFFER_SIZE, sizeof(char));
    strcpy(client_liste, "10:Online users: "); 

	for(int i=0; i < size; i++)
	{
		if(client_fds[i].client_sock_fd != 0)
		{
			strcat(client_liste, client_fds[i].username);
			strcat(client_liste, "  ");
		}
	}
	for(int i=0; i < size; i++)
	{
		if(client_fds[i].client_sock_fd != 0)
		{
			if(send(client_fds[i].client_sock_fd,client_liste,BUFFER_SIZE,0) == -1)
			{
				printf("server send error!\n");
			}
		}
	}
	free(client_liste);
}


int acquire_client_sock(struct client client_fds[], char * username, int size_client_fds)
{
	int resultat = -1;
	char input_msg[BUFFER_SIZE];  
	memset(input_msg, '\0', BUFFER_SIZE);
	for(int i=0; i < size_client_fds; i++)
	{
		if(strcmp(client_fds[i].username, username) == 0)
		{
			resultat = client_fds[i].client_sock_fd;
			break;
		}
	}
	return resultat;
}

void define_client_colour(struct client client_fds[], int client_sock_fd, int size_client_fds, enum colour col)
{
	for(int i=0; i < size_client_fds; i++)
	{
		if(client_fds[i].client_sock_fd == client_sock_fd)
		{
			client_fds[i].col = col;
			break;
		}
	}
}


int main(int argc, const char * argv[])  
{  

	char input_msg[BUFFER_SIZE];  
	char recv_msg[BUFFER_SIZE]; 

	struct sockaddr_in server_addr;  
	server_addr.sin_family = AF_INET;  
	server_addr.sin_port = htons(PORT);  
	server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");  
	memset(&(server_addr.sin_zero), '\0', 8);  
	//creation de socket  
	int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
	if(server_sock_fd == -1)
	{  
		perror("socket error");  
		return 1;  
	}  
 
	int bind_result = bind(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));  
	if(bind_result == -1)
	{  
		perror("bind error");  
		return 1;  
	}  
	//listen  
	if(listen(server_sock_fd, BACKLOG) == -1) 
	{  
        	perror("listen error");  
        	return 1;  
	}  
	  
	fd_set server_fd_set;  
	int max_fd = -1;  

	while(1)
	{  
		FD_ZERO(&server_fd_set);  
		FD_SET(STDIN_FILENO, &server_fd_set);  
		FD_SET(server_sock_fd, &server_fd_set);  

		if(max_fd < server_sock_fd)
		{  
			max_fd = server_sock_fd;  
		}  
		
		for(int i =0; i < CONCURRENT_MAX; i++)
		{  
			if(client_fds[i].client_sock_fd != 0)
			{  
				FD_SET(client_fds[i].client_sock_fd, &server_fd_set);  
				if(max_fd < client_fds[i].client_sock_fd)
				{  
					max_fd = client_fds[i].client_sock_fd;  
				}  
			}  
		} 

		int ret = select(max_fd + 1, &server_fd_set, NULL, NULL, NULL);  
		if(ret < 0)  
		{  
			perror("select error\n");  
			continue;  
		}  
		else if(ret > 0)
		{  

			if(FD_ISSET(server_sock_fd, &server_fd_set))
			{    
				struct sockaddr_in client_address;  
				socklen_t address_len = sizeof(struct sockaddr);  
				int client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_address, &address_len);  
				printf("new connection client_sock_fd = %d\n", client_sock_fd);  

				if(client_sock_fd == -1)
				{
					printf("%s\n",strerror(errno));
				}
				if(client_sock_fd > 0)
				{  
					int index = -1;  
					for(int i = 0; i < CONCURRENT_MAX; i++)
					{  
						if(client_fds[i].client_sock_fd == 0)
						{  
							index = i;  
							client_fds[i].client_sock_fd = client_sock_fd; 
							break;  
						}  
					}  
					if(index >= 0)
					{  
						printf("new client(%d) is connecting... ... %s:%d\n", index+1, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));  
					}  
					else
					{  
						memset(input_msg, '\0', BUFFER_SIZE); 
						strcpy(input_msg, "Warning:too many clients!\n");  
						send(client_sock_fd, input_msg, BUFFER_SIZE, 0);  
					}  
				}  
			} 
			if(FD_ISSET(STDIN_FILENO, &server_fd_set))
			{  
				memset(input_msg, '\0', BUFFER_SIZE);  
				fgets(recv_msg, BUFFER_SIZE, stdin);  
				if(strcmp(recv_msg,"quit") == 0)
				{
					exit(0);
				}
			}

	 
			for(int i =0; i < CONCURRENT_MAX; i++)
			{  
				if(client_fds[i].client_sock_fd !=0)
				{  
					if(FD_ISSET(client_fds[i].client_sock_fd, &server_fd_set))
					{   
						memset(input_msg, '\0', BUFFER_SIZE);
						long byte_num = recv(client_fds[i].client_sock_fd, recv_msg, BUFFER_SIZE, 0);

						if (byte_num > 0)
						{
							if (byte_num > BUFFER_SIZE)
							{
								byte_num = BUFFER_SIZE;
							}

							char *recv;
							char *split = ":";
							recv = recv_msg;
							int type = atoi(strsep(&recv, split));
							char nom[20];
							memset(nom, '\0', 20);

							switch (type)
							{
								case LOGIN:
									strcpy(nom, verify_account(fopen("infoLogin.txt", "rt"), recv));
									if (strlen(nom) == 0)
									{
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, define_msg(input_msg, "5:", "false"));
										send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0);
									}
									else
									{
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, define_msg(input_msg, "5:", "true"));
										strcpy(client_fds[i].username, nom);

										if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
										refresh_waiting_list(client_fds, CONCURRENT_MAX);
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, "9:Choose a name as your adversary or enter 'a' to wait for others:");
										if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
									}
									break;

								case SIGN_UP:
									strcpy(nom, register_account(fopen("infoLogin.txt", "a+"), recv));
									if (strlen(nom) == 0)
									{
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, define_msg(input_msg, "6:", "false"));
										send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0);
									}
									else
									{
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, define_msg(input_msg, "6:", "true"));
										strcpy(client_fds[i].username, nom);
										if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}

										refresh_waiting_list(client_fds, CONCURRENT_MAX);
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, "9:Choose a name as your adversary or enter 'a' to wait for others:");
										if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
									}
									break;

								case ASK_CONNECT_TO_SERVER:;
									int index = i;
									int statut = -1;

									for (int i = 0; i < CONCURRENT_MAX; i++)
									{
										if (strcmp(recv, client_fds[i].username) == 0)
										{
											if (index != i)
											{
												memset(input_msg, '\0', BUFFER_SIZE);
												strcpy(input_msg, define_msg(input_msg, "7:", client_fds[index].username));
												strcpy(input_msg, define_msg(input_msg, ",", client_fds[i].username));
												if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
												{
													printf("server send error!\n");
												}
												statut = 1;
											}
											else
											{
												memset(input_msg, '\0', BUFFER_SIZE);
												strcpy(input_msg, "10:You can not start a game with yourself!");
												if (send(client_fds[index].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
												{
													printf("server send error!\n");
												}
												memset(input_msg, '\0', BUFFER_SIZE);
												strcpy(input_msg, "9:Choose a name as your adversary or enter 'a' to wait for others:");
												if (send(client_fds[index].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
												{
													printf("server send error!\n");
												}
												statut = 1;
											}
											break;
										}
									}
									if (statut == -1)
									{
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, "9:Choose a name as your adversary or enter 'a' to wait for others:");
										if (send(client_fds[index].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
									}
									break;

								case REPONSE_CONNECT_TO_SERVER:
									split = ",";
									char reponse[LEN];
									memset(reponse, '\0', LEN);
									strcpy(reponse, strsep(&recv, split));
									char sender[LEN];
									memset(sender, '\0', LEN);
									strcpy(sender, strsep(&recv, split));
									int client_sock_fd_sender = acquire_client_sock(client_fds, sender, BUFFER_SIZE);
									int client_sock_fd_receiver = acquire_client_sock(client_fds, recv, BUFFER_SIZE);

									if (strcmp(reponse, "true") == 0)
									{
										client_fds[i].adversary_sock_fd = client_sock_fd_sender;
										for (int i = 0; i < CONCURRENT_MAX; i++)
										{
											if (client_fds[i].client_sock_fd == client_sock_fd_sender)
											{
												client_fds[i].adversary_sock_fd = client_sock_fd_receiver;
												break;
											}
										}

										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, define_msg(input_msg, "10:", "3 2 1 FIGHT!!!!!"));
										if (send(client_sock_fd_sender, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
										if (send(client_sock_fd_receiver, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}

										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, define_msg(input_msg, "11:", "true"));
										if (send(client_sock_fd_sender, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
										if (send(client_sock_fd_receiver, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}

										srand((unsigned)time(0));
										int n = rand() % 2 + 1;

										if (n == 1)
										{
											define_client_colour(client_fds, client_sock_fd_sender, CONCURRENT_MAX, P_WHITE);
											memset(input_msg, '\0', BUFFER_SIZE);
											for (int i = 0; i < CONCURRENT_MAX; i++)
											{
												if (client_fds[i].client_sock_fd == client_sock_fd_sender)
												{
													printf("%s 's color is --> %d\n", client_fds[i].username, client_fds[i].col);
													client_fds[i].turn = "true";
													strcpy(input_msg, define_msg(input_msg, "4:", client_fds[i].turn));
													strcpy(input_msg, define_msg(input_msg, ",", "white"));
													if (send(client_sock_fd_sender, input_msg, BUFFER_SIZE, 0) == -1)
													{
														printf("server send error!\n");
													}
													break;
												}
											}

											define_client_colour(client_fds, client_sock_fd_receiver, CONCURRENT_MAX, P_RED);
											printf("%s 's color is --> %d\n", client_fds[i].username, client_fds[i].col);
											memset(input_msg, '\0', BUFFER_SIZE);
											client_fds[i].turn = "false";
											strcpy(input_msg, define_msg(input_msg, "4:", client_fds[i].turn));
											strcpy(input_msg, define_msg(input_msg, ",", "red"));
											if (send(client_sock_fd_receiver, input_msg, BUFFER_SIZE, 0) == -1)
											{
												printf("server send error!\n");
											}
										}

										else
										{
											define_client_colour(client_fds, client_sock_fd_receiver, CONCURRENT_MAX, P_WHITE);
											memset(input_msg, '\0', BUFFER_SIZE);
											client_fds[i].turn = "true";
											strcpy(input_msg, define_msg(input_msg, "4:", client_fds[i].turn));
											strcpy(input_msg, define_msg(input_msg, ",", "white"));
											if (send(client_sock_fd_receiver, input_msg, BUFFER_SIZE, 0) == -1)
											{
												printf("server send error!\n");
											}

											define_client_colour(client_fds, client_sock_fd_sender, CONCURRENT_MAX, P_RED);
											memset(input_msg, '\0', BUFFER_SIZE);
											for (int i = 0; i < CONCURRENT_MAX; i++)
											{
												if (client_fds[i].client_sock_fd == client_sock_fd_sender)
												{
													client_fds[i].turn = "false";
													strcpy(input_msg, define_msg(input_msg, "4:", client_fds[i].turn));
													strcpy(input_msg, define_msg(input_msg, ",", "white"));
													if (send(client_sock_fd_sender, input_msg, BUFFER_SIZE, 0) == -1)
													{
														printf("server send error!\n");
													}
													break;
												}
											}
										}
									}

									else
									{
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, "10:Your request has been refused! Try again!");
										if (send(client_sock_fd_sender, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
										refresh_waiting_list(client_fds, CONCURRENT_MAX);
										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, "9:Choose a name as your adversary or enter 'a' to wait for others:");
										if (send(client_sock_fd_sender, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}

										memset(input_msg, '\0', BUFFER_SIZE);
										strcpy(input_msg, "9:Choose a name as your adversary or enter 'a' to wait for others:");
										if (send(client_sock_fd_receiver, input_msg, BUFFER_SIZE, 0) == -1)
										{
											printf("server send error!\n");
										}
									}
									break;

								case MOVE:;
									index = i;
									player_turn(client_fds, client_fds[i], recv);

									client_fds[index].turn = "false";
									for (int i = 0; i < 8; i++)
									{
										if (client_fds[i].adversary_sock_fd == client_fds[index].client_sock_fd)
										{
											client_fds[i].turn = "true";
											break;
										}
									}
									memset(input_msg, '\0', BUFFER_SIZE);
									strcpy(input_msg, define_msg(input_msg, "4:", client_fds[i].turn));
									printf("4:'turn' -->  %s\n", input_msg);
									if (client_fds[i].col == P_RED)
									{
										strcpy(input_msg, define_msg(input_msg, ",", "red"));
									}
									else
									{
										strcpy(input_msg, define_msg(input_msg, ",", "white"));
									}
									printf("4:'turn','col' -->  %s\n", input_msg);
									if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
									{
										printf("server send error!\n");
									}

									memset(input_msg, '\0', BUFFER_SIZE);
									for (int i = 0; i < CONCURRENT_MAX; i++)
									{
										if (client_fds[i].adversary_sock_fd == client_fds[index].client_sock_fd)
										{
											strcpy(input_msg, define_msg(input_msg, "4:", client_fds[i].turn));
											if (client_fds[i].col == P_RED)
											{
												strcpy(input_msg, define_msg(input_msg, ",", "red"));
											}
											else
											{
												strcpy(input_msg, define_msg(input_msg, ",", "white"));
											}
											if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
											{
												printf("server send error!\n");
											}
											break;
										}
									}
									break;

								case END:
									memset(input_msg, '\0', BUFFER_SIZE);
									strcpy(input_msg, define_msg(input_msg, "10:", "You LOSE!"));
									if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
									{
										printf("client send error!\n");
									}

									memset(input_msg, '\0', BUFFER_SIZE);
									index = i;
									for (int i = 0; i < CONCURRENT_MAX; i++)
									{
										if (client_fds[i].adversary_sock_fd == client_fds[index].client_sock_fd)
										{
											strcpy(input_msg, define_msg(input_msg, "10:", "You WIN!"));
											if (send(client_fds[i].client_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
											{
												printf("server send error!\n");
											}
											break;
										}
									}

								default:
									send(client_fds[i].client_sock_fd, "illegal", BUFFER_SIZE, 0);
							}
						}
						else if(byte_num < 0)
						{  
							printf("receive from %s error!\n", client_fds[i].username);  
						}  
						else
						{  
							FD_CLR(client_fds[i].client_sock_fd, &server_fd_set);  
							client_fds[i].client_sock_fd = 0;  
							memset(client_fds[i].username, '\0', 20);
							printf("%s\n", client_fds[i].username);
						}  
					}  
				}  
			}  
		}  
	}
	close(server_sock_fd);
	return 0;  
} 
