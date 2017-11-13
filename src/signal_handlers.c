#include "signal_handlers.h"
#include <stdio.h>
#include <commands.h>

void catch_sigint(int signalNo)
{
  // TODO: File this!
	printf("ctrl^c\n");
	return;
}

void catch_sigtstp(int signalNo)
{
  // TODO: File this!
	printf("ctrl^z\n");
	return;
}
