#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct alarm
{
	int seconds;
	char message[64];
};

void *alarm_thread(struct alarm *a)
{
	int status = pthread_detach(pthread_self());
	if (status != 0)
	{
		fprintf(stderr, "pthread_detach: %s\n", strerror(status));
		exit(EXIT_FAILURE);
	}

	sleep(a->seconds);
	printf("(%d) %s\n", a->seconds, a->message);
	free(a);

	return NULL;
}


int main(void)
{
	int status;
	char line[128];
	struct alarm *a;
	pthread_t thread;

	while (printf("Alarm> ") && fgets(line, sizeof(line), stdin))
	{
		if (strlen(line) <= 1)
			continue;

		if (!(a = malloc(sizeof(*a))))
		{
			fputs("Failed to allocate alarm\n", stderr);
			return EXIT_FAILURE;
		}
		if (sscanf(line, "%d %64[^\n]", &a->seconds, a->message) < 2)
		{
			fputs("Bad command\n", stderr);
			free(a);
		}
		else
		{
			status = pthread_create(&thread, NULL, (void *(*)(void*))alarm_thread, a);
			if (status != 0)
			{
				fprintf(stderr, "pthread_create: %s\n", strerror(status));
				return EXIT_FAILURE;
			}
		}
	}
}
