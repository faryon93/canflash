#include <stdio.h>

#define PROGRESS_BAR_SIZE	30

void print_progress(double max, double value, char* title, char* action)
{
	const double d = PROGRESS_BAR_SIZE / max;

	printf("\r%s: [", title);
	for (int i = 0; i < PROGRESS_BAR_SIZE; i++)
	{
		if (i < (value * d))
			printf("=");
		else
			printf(" ");
	}
	printf("]: %s                ", action);
}