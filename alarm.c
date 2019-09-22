#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	int seconds;
	char line[128], message[64];

	while (printf("Alarm> ") && fgets(line, sizeof(line), stdin))
	{
		if (strlen(line) <= 1)
			continue;
		if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2)
			fputs("Bad command\n", stderr);
		else
		{
			sleep(seconds);
			printf("(%d) %s\n", seconds, message);
		}
	}
}
