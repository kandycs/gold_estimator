
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "list.h"
#include "common.h"

struct _buf_list {
	struct list_head list;
	double *data;
};

LIST_HEAD(buf_list);

struct _hist_buffer history;

int ge_buffer_init(int window_size, int dim)
{
	history.steps = window_size;
	history.curr = 0;
	history.dim = dim;
#if DEBUG_BUFFER
	printf("hist.steps: %d ", history.steps);
	printf("hist.curr: %d ", history.curr);
	printf("hist.dim: %d\n", history.dim);
#endif

	history.data = (double *)malloc(sizeof(double) * window_size * dim);
	if (history.data == NULL) {
		perror("initializing error when alloc memory");
		exit(EXIT_FAILURE);
	}
}

bool ge_buffer_status()
{
	if (history.curr == history.steps)
		return true;
	else
		return false;
}

int ge_buffer_steps()
{
	return history.steps;
}

int ge_buffer_dim()
{
	return history.dim;
}

double *ge_buffer_data()
{
	return history.data;
}

int ge_buffer_append(double *buf, int size)
{
	struct list_head *pos;
	struct _buf_list *entry;
	int i = 0;
#ifdef DEBUG_BUFFER
	int j;
#endif

	/* check whether the array size matching the expected size */
	if (history.dim != size)
		fprintf(stderr, "array size doesn't match, \
                init value: %d, curr value: %d\n", history.dim, size);

	/* allocate a _buf_list to store the data */
	struct _buf_list *tmp =
	    (struct _buf_list *)malloc(sizeof(struct _buf_list));
	if (tmp == NULL) {
		perror("failed to allocate the memory at append");
		ge_buffer_clean();
		exit(EXIT_FAILURE);
	}

	tmp->data = (double *)malloc(size * sizeof(double));
	if (tmp->data == NULL) {
		perror("failed to allocate the memory at append");
		ge_buffer_clean();
		exit(EXIT_FAILURE);
	}

	memcpy(tmp->data, buf, size * sizeof(double));

#ifdef DEBUG_BUFFER
	printf("append:\n");
	for (j = 0; j < size; j++)
		printf("%f ", tmp->data[j]);
	printf("\n");
#endif

	/* if history buf is not full, append to the end */
	if (history.curr < history.steps) {
		list_add_tail(&tmp->list, &buf_list);
		history.curr++;
	} else {
		// get and delete oldest timestep
		entry = list_entry(buf_list.next, struct _buf_list, list);
		list_del(&entry->list);
		free(entry->data);
		free(entry);
		// append the list to the tail
		list_add_tail(&tmp->list, &buf_list);
	}

	if (history.curr == history.steps) {
		i = 0;
		list_for_each(pos, &buf_list) {
			entry = list_entry(pos, struct _buf_list, list);
			memcpy(&history.data[i * history.dim],
			       entry->data, history.dim * sizeof(double));

			i++;
		}
#ifdef DEBUG_BUFFER
		printf("data in history:\n");
		for (i = 0; i < history.steps; i++) {
			printf("\n");
			for (j = 0; j < history.dim; j++)
				printf("%f ",
				       history.data[i * history.dim + j]);
		}
		printf("\n");
#endif
	}

}

int ge_buffer_clean()
{
	struct list_head *pos;
	struct _buf_list *entry;
	for (pos = buf_list.next; pos != &buf_list;) {
		entry = list_entry(pos, struct _buf_list, list);
		list_del(pos);
		pos = pos->next;
		free(entry->data);
		free(entry);
	}
}
