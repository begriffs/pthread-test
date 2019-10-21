#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

int main(void)
{
	int seconds;
	char line[128], message[64];
	pid_t pid;

	while (printf("Alarm> ") &&
			fgets(line, sizeof(line), stdin))
	{
		if (strlen(line) <= 1)
			continue;
		if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2)
			fputs("Bad command\n", stderr);
		else
		{
			if ((pid = fork()) == 0)
			{
				sleep(seconds);
				printf("(%d) %s\n", seconds, message);
				exit(EXIT_SUCCESS);
			}
			else if (pid > 0)
			{
				do
				{
					/* collect kids who have finished */
					pid = waitpid(-1, NULL, WNOHANG);
				} while (pid > 0);
			}
			else
				fputs("Failed fork()\n", stderr);
		}
	}
}
