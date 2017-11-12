#include "signal_handlers.h"
#include <stdio.h>

void catch_sigint(int signalNo)
{
  // TODO: File this!
	printf("ctrl^c\n");
}

void catch_sigtstp(int signalNo)
{
  // TODO: File this!
	printf("ctrl^z\n");
}
