#include <pthread.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	pthread_t weirdo;
	/* join with an uninitialized value */
	int status = pthread_join(weirdo, NULL);

	if (status != 0)
		fprintf(stderr, "Error %d: %s\n", status, strerror(status));
	else
		fprintf(stderr, "Weird, no error\n");
	return 0;
}
