#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct alarm
{
	struct alarm *link;
	time_t time;
	int seconds;
	char message[64];
};

pthread_mutex_t  g_alarm_mutex;
struct alarm    *g_alarm_list;

#define MAX(a,b) ((a)>=(b) ? (a) : (b))

void *alarm_thread(void *_arg)
{
	struct alarm *a;
	int sleep_time;
	time_t now;

	(void)_arg; /* unused */

	while (1)
	{
		pthread_mutex_lock(&g_alarm_mutex);
		a = g_alarm_list;

		if (!a)
			sleep_time = 1;
		else
		{
			g_alarm_list = a->link;
			now = time(NULL);
			sleep_time = MAX(0, a->time - now);
		}
		pthread_mutex_unlock(&g_alarm_mutex);

		if (sleep_time > 0)
		{
			/* this kind of sucks because it will miss out on any
			 * alarms created during the sleep which expire sooner */
			sleep(sleep_time);
		}
		else
			sched_yield(); /* give main() a chance */

		if (a)
		{
			printf("(%d) %s\n", a->seconds, a->message);
			free(a);
		}
	}

	return NULL;
}


int main(void)
{
	int status;
	char line[128];
	struct alarm *a, *next, **last;
	pthread_t thread;

	status = pthread_create(&thread, NULL, alarm_thread, NULL);
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
			pthread_mutex_lock(&g_alarm_mutex);
			a->time = time(NULL) + a->seconds;

			/* insert alarm into ordered list */
			last = &g_alarm_list;
			next = *last;
			while (next)
			{
				if (next->time >= a->time)
				{
					a->link = next;
					*last = a;
					break;
				}
				last = &next->link;
				next = next->link;
			}
			/* if hit end of list */
			if (!next)
			{
				*last = a;
				a->link = NULL;
			}
			pthread_mutex_unlock(&g_alarm_mutex);
		}
	}
}
