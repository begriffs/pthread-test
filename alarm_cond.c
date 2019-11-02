#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct alarm
{
	struct alarm *link;
	time_t time;
	int seconds;
	char message[64];
} *g_alarm_list = NULL;

pthread_mutex_t g_alarm_mutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  g_alarm_cond    = PTHREAD_COND_INITIALIZER;
time_t          g_current_alarm = 0;

/* Locking protocol:
 * This function requires the caller to have
 * locked g_alarm_mutex
 */
void alarm_insert(struct alarm *a)
{
	struct alarm **last, *next;

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

	/* if a new earlier alarm has come in, or if
	 * the alarm thread is idle, wake it up */
	if (g_current_alarm == 0 || a->time < g_current_alarm)
	{
		g_current_alarm = a->time;
		pthread_cond_signal(&g_alarm_cond);
	}
}

void *alarm_thread(void *arg)
{
	struct alarm *a;
	struct timespec cond_time = {0};
	time_t now;
	bool expired;

	(void)arg;
	pthread_mutex_lock(&g_alarm_mutex);
	while (1)
	{
		g_current_alarm = 0;
		while (g_alarm_list == NULL)
			pthread_cond_wait(&g_alarm_cond, &g_alarm_mutex);

		a = g_alarm_list;
		g_alarm_list = a->link;
		now = time(NULL);
		expired = false;
		
		if (a->time > now)
		{
			cond_time.tv_sec = a->time;
			g_current_alarm = a->time;
			while (g_current_alarm == a->time)
			{
				int s = pthread_cond_timedwait(
					&g_alarm_cond, &g_alarm_mutex, &cond_time);
				if (s == ETIMEDOUT)
				{
					expired = true;
					break;
				}
			}
			if (!expired)
				alarm_insert(a);
		}
		else
			expired = true;

		if (expired)
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
	struct alarm *a;
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
			alarm_insert(a);
			pthread_mutex_unlock(&g_alarm_mutex);
		}
	}
}
