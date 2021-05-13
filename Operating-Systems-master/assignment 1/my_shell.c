#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
int numberOfParallel = 0;
pid_t pidarr[64];
// int numberOfForeground = 0;
// pid_t foregroundArr[64];
int numberOfBackground = 0;
pid_t backgroundArr[64];

// Helper functions
unsigned int stoi(char *token)
{
	unsigned int unsInt = 0;
	for (int i = 0; i < strlen(token); i++)
	{
		unsInt = unsInt * 10 + (token[i] - '0');
	}
	return unsInt;
}

/* Splits the string by space and returns the array of tokens */
char **tokenize(char *line, int *parallel, int *background)
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
				if (strcmp(token, "&&&") == 0)
				{
					*parallel = 1;
				}
				if (strcmp(token, "&") == 0)
				{
					*background = 1;
				}
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

void killAll(pid_t[], int);

// char *processecho(char **, int, int *);
void pwd();
int exec(int, int, char *, char **, int);

void my_sleep(int);
void sighandler()
{
	killAll(pidarr, numberOfParallel);
}

void backgroundHandler()
{
	if (numberOfBackground != 0)
	{
		int wstat;
		pid_t pid;
		pid = wait3(&wstat, WNOHANG, (struct rusage *)NULL);

		for (size_t i = 0; i < numberOfBackground; i++)
		{
			if (backgroundArr[i] == pid)
			{
				numberOfBackground--;
				printf("Shell: Background process finished \n");
				//swapping the last background process with removed arr
				backgroundArr[i] = backgroundArr[numberOfBackground];
				break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sighandler);
	signal(SIGCHLD, backgroundHandler);

	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i;

	FILE *fp;
	if (argc >= 2)
	{
		fp = fopen(argv[1], "r");
		if (fp < 0)
		{
			printf("File doesn't exists.");
			return -1;
		}
	}
	int ex = 0;
	while (!ex)
	{
		int parallel = 0;
		int background = 0;
		numberOfParallel = 0;

		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if (argc == 2)
		{ // batch mode
			if (fgets(line, sizeof(line), fp) == NULL)
			{ // file reading finished
				break;
			}
			line[strlen(line) - 1] = '\0';
		}
		else
		{ // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line, &parallel, &background);
		char *comm[64];
		int commLen = 0;
		for (i = 0; tokens[i] != NULL; i++)
		{
			if (strcmp(tokens[i], "&") == 0)
			{
				background = 1;
				exec(parallel, background, comm[0], comm, commLen);
				commLen = 0;
			}
			else if (strcmp(tokens[i], "&&") == 0)
			{
				exec(parallel, background, comm[0], comm, commLen);
				commLen = 0;
			}
			else if (strcmp(tokens[i], "&&&") == 0)
			{
				parallel = 1;
				exec(parallel, background, comm[0], comm, commLen);
				commLen = 0;
			}

			else if (strcmp(tokens[i], "cd") == 0)
			{ // done
				chdir(tokens[++i]);
			}

			else if (strcmp(tokens[i], "exit") == 0)
			{ //done
				killAll(pidarr, numberOfParallel);
				killAll(backgroundArr, numberOfBackground);
				ex = 1;
				printf("Shell: Goodbye. \n");
				exit(EXIT_SUCCESS);
				break;
			}

			else
			{
				comm[commLen] = tokens[i];
				commLen++;
			}
		}
		exec(parallel, background, comm[0], comm, commLen);

		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++)
		{
			free(tokens[i]);
		}
		free(tokens);

		for (size_t i = 0; i < numberOfParallel; i++)
		{
			pid_t wpid = wait(0);
			numberOfParallel--;
			pidarr[i] = 0;
		}
	}
	return 0;
}

int exec(int parallel, int background, char *command, char **args, int size)
{
	pid_t pid, wpid;
	int status;
	int i;
	char *new_arg[size + 1];

	// new_arg[0] = command;
	for (i = 0; i < size; i++)
	{
		new_arg[i] = args[i];
	}

	new_arg[size] = NULL;

	pid = fork();
	if (pid == 0)
	{
		if (execvp(command, new_arg) < 0)
		{
			printf("Shell: Incorrect command \n");
			exit(EXIT_FAILURE);
		};
	}
	else if (pid)
	{
		if (parallel == 1)
		{ // if parallel
			pidarr[numberOfParallel] = pid;
			numberOfParallel++;
		}
		else if (background == 1)
		{ // if background
			backgroundArr[numberOfBackground] = pid;
			numberOfBackground++;
		}
		if (parallel != 1 && background != 1)
		{ // if blocking, reap it before going ahead
			wpid = waitpid(pid, &status, 0);
		}
	}
	return size;
}

void killAll(pid_t processes[], int number)
{
	for (int i = 0; i < number; i++)
	{
		kill(processes[i], SIGKILL);
		wait(0);
	}
}