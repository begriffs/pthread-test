#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct shared_int
{
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	int             value;
} g_data = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	0
};

/* sleep time */
int g_hibernation = 1;

/* sets main's predicate and signals condition var */
void *wait_thread(void *arg)
{
	(void)arg;
	sleep(g_hibernation);

	pthread_mutex_lock(&g_data.mutex);
	g_data.value = 1; /* main's predicate */
	pthread_cond_signal(&g_data.cond);
	pthread_mutex_unlock(&g_data.mutex);

	return NULL;
}

int main(int argc, const char **argv)
{
	pthread_t wait_thread_id;
	struct timespec timeout = {0};

	if (argc > 1)
		g_hibernation = atoi(argv[1]);

	pthread_create(&wait_thread_id, NULL, wait_thread, NULL);

	/* if g_hibernation > 2 this will time out */
	timeout.tv_sec = time(NULL)+2;

	pthread_mutex_lock(&g_data.mutex);
	while (g_data.value == 0)
	{
		int status = pthread_cond_timedwait(
			&g_data.cond, &g_data.mutex, &timeout);
		if (status == ETIMEDOUT)
		{
			puts("Condition var timeout");
			break;
		}
	}
	if (g_data.value != 0)
		puts("Condition var got signaled");
	pthread_mutex_unlock(&g_data.mutex);
	return 0;
}
