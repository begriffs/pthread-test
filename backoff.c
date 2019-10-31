#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ITERATIONS 10

pthread_mutex_t mutex[3] = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER
};

/* backoff or deadlock */
bool backoff = true;
enum yield_flag
{
	SLEEP = -1, NO_YIELD = 0, YIELD = 1
} yf = NO_YIELD;

void *lock_forward(void *arg)
{
	(void)arg;

	for (int iterate = 0; iterate < ITERATIONS; iterate++)
	{
		int backoffs = 0;
		for (int i = 0; i < 3; i++)
		{
			if (i == 0 || !backoff)
				pthread_mutex_lock(&mutex[i]);
			else
			{
				if (pthread_mutex_trylock(&mutex[i]) == EBUSY)
				{
					backoffs++;
					while (--i >= 0)
						pthread_mutex_unlock(&mutex[i]);
				}

				switch (yf)
				{
					case YIELD:
						sched_yield();
						break;
					case SLEEP:
						sleep(1);
						break;
					default:
						break;
				}
			}
		}

		printf("F: %d\n", backoffs);
		pthread_mutex_unlock(&mutex[2]);
		pthread_mutex_unlock(&mutex[1]);
		pthread_mutex_unlock(&mutex[0]);
		sched_yield();
	}
	return NULL;
}

void *lock_backward(void *arg)
{
	(void)arg;

	for (int iterate = 0; iterate < ITERATIONS; iterate++)
	{
		int backoffs = 0;
		for (int i = 2; i >= 0 ; i--)
		{
			if (i == 2 || !backoff)
				pthread_mutex_lock(&mutex[i]);
			else
			{
				if (pthread_mutex_trylock(&mutex[i]) == EBUSY)
				{
					backoffs++;
					while (++i < 3)
						pthread_mutex_unlock(&mutex[i]);
				}

				switch (yf)
				{
					case YIELD:
						sched_yield();
						break;
					case SLEEP:
						sleep(1);
						break;
					default:
						break;
				}
			}
		}

		printf("B: %d\n", backoffs);
		pthread_mutex_unlock(&mutex[0]);
		pthread_mutex_unlock(&mutex[1]);
		pthread_mutex_unlock(&mutex[2]);
		sched_yield();
	}
	return NULL;
}

int main(int argc, const char **argv)
{
	pthread_t forward, backward;

	if (argc > 1)
		backoff = atoi(argv[1]);
	if (argc > 2)
		yf = atoi(argv[2]);

	pthread_create(&forward, NULL, lock_forward, NULL);
	pthread_create(&backward, NULL, lock_backward, NULL);

	pthread_exit(NULL);
}
