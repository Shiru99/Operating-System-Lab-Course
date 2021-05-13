pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0)
	{exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		perror("error forking");
	}
	else
	{
		do
		{
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}