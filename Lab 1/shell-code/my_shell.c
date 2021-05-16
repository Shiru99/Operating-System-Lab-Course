#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <signal.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int numOfBackground = 0;
int numOfParallel = 0;
pid_t parallelPID[64];
pid_t backgroundPID[64];


/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

void sighandler()
{
	// killAll(parallelPID, numOfParallel);
}

void backgroundHandler()
{
	if (numOfBackground != 0)
	{
		int wstat;
		pid_t pid;
		pid = wait3(&wstat, WNOHANG, (struct rusage *)NULL);

		for (size_t i = 0; i < numOfBackground; i++)
		{
			if (backgroundPID[i] == pid)
			{
				numOfBackground--;
				printf("Shell: Background process finished\n");
				//swapping the last background process with removed arr
				backgroundPID[i] = backgroundPID[numOfBackground];
				break;
			}
		}
	}
}

int exec(bool isBackground, char **commandArgs, int commandLength)
{
	pid_t pid;

	char *new_arg[commandLength + 1];
	for (int i = 0; i < commandLength; i++)
	{
		new_arg[i] = commandArgs[i];
	}
	new_arg[commandLength] = NULL;

	pid = fork();
	if (pid == 0) // Child
	{
		// execvp starts its execution, the original program in the caller's address space is gone and is replaced by the new program
		if (execvp(commandArgs[0], new_arg) < 0)
		{
			printf("Shell: Incorrect command  \n");
			exit(EXIT_FAILURE);
		}
		else // never runs
		{
			printf("Shell: Completed successfully");
		}
	}
	else if (pid)
	{
		if (isBackground)
		{ // if background
			backgroundPID[numOfBackground++] = pid;
		}
		else
		{ // if blocking, reap it before going ahead
			int status;
			pid_t wpid = waitpid(pid, &status, 0);
		}
	}

	return commandLength;
}

int main(int argc, char *argv[])
{

	signal(SIGINT, sighandler);			// Issued if the user sends an interrupt signal (Ctrl + C)
	signal(SIGCHLD, backgroundHandler); // When a child process stops or terminates, SIGCHLD is sent to the parent process. The default response to the signal is to ignore it.

	char line[MAX_INPUT_SIZE]; // Input from terminal
	char **tokens;
	int i;

	FILE *fp;
	if (argc == 2)
	{
		fp = fopen(argv[1], "r");
		if (fp < 0)
		{
			printf("File doesn't exists.");
			return -1;
		}
	}

	while (1)
	{
		/* BEGIN: TAKING INPUT */
		// pid_t wpid;
		// while ((wpid = wait(NULL)) > 0);
		// printf("Inside while\n");
		bzero(line, sizeof(line));

		/*
		 	The bzero() function erases the data in the n bytes of the memory  starting at the location pointed to by s, by writing zeros (bytes containing '\0') to that area.
		*/

		if (argc == 2) // batch mode
		{
			if (fgets(line, sizeof(line), fp) == NULL) // file reading finished & terminates at the '\n'
			{
				break;
			}
			line[strlen(line) - 1] = '\0';
		}
		else // interactive mode
		{
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);

		//do whatever you want with the commands, here we just print them
		// for (i = 0; tokens[i] != NULL; i++)
		// {
		// printf("found token %s (remove this debug output later)\n", tokens[i]);
		// }

		// -------------------- Added Here --------------------

		char *command[MAX_TOKEN_SIZE];
		int commandLength = 0;
		bool CDcommand = false;

		bool isBackground = false;
		bool isParallel = false;

		numOfParallel = 0;
		numOfBackground = 0;
		int index[65];
		index[0] = 0;
		for (i = 0; tokens[i] != NULL; i++)
		{

			if (strcmp(tokens[i], "&") == 0) // background
			{
				isBackground = true;
				exec(isBackground, command, commandLength);
				commandLength = 0;
			}
			else if (strcmp(tokens[i], "&&") == 0) // foreground Serial
			{
				if (CDcommand != true)
				{
					exec(isBackground, command, commandLength);
				}
				CDcommand = false;
				commandLength = 0;
			}
			else if (strcmp(tokens[i], "&&&") == 0) // foreground parallel
			{
				isParallel = true;
				numOfParallel++;
				index[numOfParallel] = i;
				commandLength = 0;
			}
			else if (strcmp(tokens[i], "cd") == 0) // ls
			{
				if (chdir(tokens[++i]) < 0)
				{
					printf("Shell: Incorrect command  \n");
				}
				CDcommand = true;
				commandLength = 0;
			}
			else // For ls,cat,echo,sleep
			{
				command[commandLength++] = tokens[i];
			}
		}
		index[numOfParallel + 1] = i;

		if (isParallel)
		{
			int j = 0;
			for (int k = 1; k <= numOfParallel + 1; k++)
			{
				char *command1[MAX_TOKEN_SIZE];

				if (k == 1)
					commandLength = index[k] - index[k - 1];
				else
					commandLength = index[k] - index[k - 1] - 1;

				for (; j < index[k]; j++)
				{
					if (k == 1)
						command1[j - index[k - 1]] = tokens[j];
					else
						command1[j - index[k - 1] - 1] = tokens[j];
				}
				j++;

				char *new_arg[commandLength];
				for (int l = 0; l < commandLength; l++)
				{
					new_arg[l] = command1[l];
				}
				new_arg[commandLength] = NULL;

				if (strcmp(command1[0], "cd") == 0)
				{
					continue;
				}
				else
				{
					pid_t pid;
					pid = fork();
					if (pid == 0) // Child
					{
						if (execvp(command1[0], new_arg) < 0)
						{
							printf("Shell: Incorrect command  \n");
							exit(EXIT_FAILURE);
						}
					}
				}
			}
			pid_t wpid;
			while ((wpid = wait(NULL)) > 0);
		}
		else if (CDcommand != true && isBackground != true)
		{
			exec(isBackground, command, commandLength);
		}

		// -------------------- Added Here --------------------

		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++)
		{
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}