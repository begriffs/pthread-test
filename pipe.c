#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct stage
{
	pthread_mutex_t mutex;
	pthread_cond_t  avail;
	pthread_cond_t  ready;
	bool            data_ready;
	long            data;
	pthread_t       thread;
	struct stage    *next;
};

struct pipe
{
	pthread_mutex_t mutex;
	struct stage    *head;
	struct stage    *tail;
	int             stages;
	int             active;
};

/* pass "message" to specified pipe stage */
void pipe_send(struct stage *s, long data)
{
	pthread_mutex_lock(&s->mutex);
	{
		while (s->data_ready)
			pthread_cond_wait(&s->ready, &s->mutex);
		s->data = data;
		s->data_ready = true;
		pthread_cond_signal(&s->avail);
	}
	pthread_mutex_unlock(&s->mutex);
}

void *pipe_stage(void *arg)
{
	struct stage *s = arg;
	struct stage *next = s->next;

	pthread_mutex_lock(&s->mutex);
	{
		while (1)
		{
			while (!s->data_ready)
				pthread_cond_wait(&s->avail, &s->mutex);
			/* the operation we perform is incrementing data */
			pipe_send(next, s->data + 1);
			s->data_ready = false;
			pthread_cond_signal(&s->ready);
		}
	}
	pthread_mutex_unlock(&s->mutex);
}

void pipe_create(struct pipe *p, int stages)
{
	struct stage **link = &p->head, *new_s, *s;

	pthread_mutex_init(&p->mutex, NULL);
	p->stages = stages;
	p->active = false;

	while (stages-- > 0)
	{
		new_s = malloc(sizeof *new_s);
		pthread_mutex_init(&new_s->mutex, NULL);
		pthread_cond_init(&new_s->avail, NULL);
		pthread_cond_init(&new_s->ready, NULL);
		new_s->data_ready = false;
		*link = new_s;
		link = &new_s->next;
	}
	
	/* end list, record tail */
	*link = NULL;
	p->tail = new_s;

	for (s = p->head; s->next; s = s->next)
		pthread_create(&s->thread, NULL, pipe_stage, s);
}

void pipe_start(struct pipe *p, long value)
{
	pthread_mutex_lock(&p->mutex);
	{
		p->active++;
	}
	pthread_mutex_unlock(&p->mutex);

	pipe_send(p->head, value);
}

bool pipe_result(struct pipe *p, long *result)
{
	struct stage *tail = p->tail;
	bool empty = false;

	pthread_mutex_lock(&p->mutex);
	{
		if (p->active <= 0)
			empty = true;
		else
			p->active--;
	}
	pthread_mutex_unlock(&p->mutex);

	if (empty)
		return false;

	pthread_mutex_lock(&tail->mutex);
	{
		while (!tail->data_ready)
			pthread_cond_wait(&tail->avail, &tail->mutex);
		*result          = tail->data;
		tail->data_ready = false;
		pthread_cond_signal(&tail->ready);
	}
	pthread_mutex_unlock(&tail->mutex);
	return true;
}

int main(void)
{
	struct pipe p;
	long value, result;
	char line[128];

	pipe_create(&p, 10);
	puts("Enter integer values, or '=' for next result");

	while (1)
	{
		printf("Data> ");
		if (fgets(line, sizeof line, stdin) == NULL)
			return 0;
		if (strlen(line) <= 1)
			continue;
		if (strlen(line) == 2 && *line == '=')
		{
			if (pipe_result(&p, &result))
				printf("Result is %ld\n", result);
			else
				puts("Pipe is empty");
		}
		else
		{
			if (sscanf(line, "%ld", &value) < 1)
				fprintf(stderr, "Enter an inter value\n");
			else
				pipe_start(&p, value);
		}
	}
}
