#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define SPIN 10000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long counter;
time_t end_time;

void *counter_thread(void *arg)
{
	int spin;

	(void)arg;

	while (time(NULL) < end_time)
	{
		pthread_mutex_lock(&mutex);
		for (spin = 0; spin < SPIN; spin++)
			counter++;
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	return NULL;
}

void *monitor_thread(void *arg)
{
	int status;

	(void)arg;

	while (time(NULL) < end_time)
	{
		sleep(3);
		status = pthread_mutex_trylock(&mutex);
		if (status != EBUSY)
		{
			printf("Counter is %ld\n", counter/SPIN);
			pthread_mutex_unlock(&mutex);
		}
		else
			puts("(missed)");
	}
	return NULL;
}

int main(void)
{
	pthread_t counter_thread_id, monitor_thread_id;

	end_time = time(NULL) + 60;
	pthread_create(&counter_thread_id, NULL, counter_thread, NULL);
	pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);

	pthread_join(counter_thread_id, NULL);
	pthread_join(monitor_thread_id, NULL);
}
