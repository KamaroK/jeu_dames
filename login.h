#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "constants.h"

//Authors : Kevis KAMARO & Arnaud NSEGIYUMVA

char *strsep();

char *define_msg(char input_msg[], char protocole[], char data[])
{
	strcat(input_msg, protocole);
	strcat(input_msg,data);
	return input_msg;
}


char *define_account(char input_msg[], char protocole[], char username[], char password[])
{
	memset(input_msg, '\0', BUFFER_SIZE);
	strcpy(input_msg, protocole);
	strcat(input_msg, username);
	strcat(input_msg, ",");
	strcat(input_msg, password);
	return input_msg;
}

char *verify_account(FILE*fd, char * account)
{
	static char username[LEN];
	memset(username, '\0', LEN);
	if(fd == NULL)
	{
		perror("Open fail");
		exit(1);
	}

	char string[50];
	fseek(fd, 0, SEEK_SET);

	while(fgets(string, sizeof(string), fd))
	{
		if(string[strlen(string)-1] == '\n')
		{
			string[strlen(string)-1] = '\0';
		}
		char * split=",";		
		if(strcmp(string, account) == 0)
		{
			strcpy(username, strsep(&account, split));
			break;
		}
	}
	fclose(fd);
	return username;
}


char *register_account(FILE*fd, char * account)
{
	static char username[LEN];
	memset(username, '\0', LEN);
	char string[50];
	char str_info[50];
	char str_nom[LEN];

	strcpy(str_info,"\n");
	strcat(str_info,account);
	char * split=",";		
	strcpy(username, strsep(&account, split));

	while(fgets(string, sizeof(string), fd))
	{
		if(string[strlen(string)-1] == '\n')
		{
			string[strlen(string)-1] = '\0';
		}

		sscanf(string, "%[^,]", str_nom);
	
		if(strcmp(str_nom, username) == 0)
		{
			memset(username, '\0', LEN);
			break;
		}
	}
	if(strlen(username) != 0)
	{
		fputs(str_info, fd);
	}
	fclose(fd);
	return username;
}


void login(int server_sock_fd)
{
	char data[100];
	memset(data, '\0', 100);
	char input_msg[BUFFER_SIZE];
	memset(input_msg, '\0', BUFFER_SIZE);

	printf("a.Log in\n");
	printf("b.Sign up\n");
	printf("c.Quit\n");

	char username[LEN];
	char password[LEN];
	char choix[5];
	int statut = -1;

	do{
		printf("Please enter your choice:");
		fgets(choix, 5, stdin);
		if(strcmp(choix, "a\n") == 0)
		{
			printf("username:");
			fgets(username, LEN, stdin);
			if(username[strlen(username)-1] == '\n')
			{
				username[strlen(username)-1] = '\0';
			}

			printf("password:");
			fgets(password, LEN, stdin);
			if(password[strlen(password)-1] == '\n')
			{
				password[strlen(password)-1] = '\0';
			}
			statut = 0;

			strcpy(input_msg, define_account(input_msg, "0:", username, password));
			if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
			{  
				perror("message send error!\n");  
			}
		}
		else if(strcmp(choix, "b\n") == 0)
		{
			printf("username:");
			fgets(username, LEN, stdin);
			if(username[strlen(username)-1] == '\n')
			{
				username[strlen(username)-1] = '\0';
			}
			while(strlen(username) > 15)
			{
				printf("Maximum 15 characters!");
				printf("username:");
				fgets(username, LEN, stdin);
				if(username[strlen(username)-1] == '\n')
				{
					username[strlen(username)-1] = '\0';
				}
			}

			printf("password:");
			fgets(password, LEN, stdin);
			if(password[strlen(password)-1] == '\n')
			{
				password[strlen(password)-1] = '\0';
			}
			while(strlen(password) > 15)
			{
				printf("Maximum 15 characters!");
				printf("password:");
				fgets(password, LEN, stdin);
				if(password[strlen(password)-1] == '\n')
				{
					password[strlen(password)-1] = '\0';
				}
			}
			statut = 0;
			strcpy(input_msg, define_account(input_msg, "1:", username, password));
			if(send(server_sock_fd, input_msg, BUFFER_SIZE, 0) == -1)
			{  
				perror("message send error!\n");  
			}
		}
		else if(strcmp(choix, "c\n") == 0)
		{
			statut = 0;
			exit(0);
		}
		else
		{
			printf("Illegal input!\n");
		}
	}while(statut == -1);


}


