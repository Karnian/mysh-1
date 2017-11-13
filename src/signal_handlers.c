#include "signal_handlers.h"
#include <stdio.h>
#include <commands.h>
#include <signal.h>

void catch_sigint(int signalNo)
{
  // TODO: File this!
	printf("ctrl^c\n");
	signal(SIGINT, SIG_IGN);
	return;
}

void catch_sigtstp(int signalNo)
{
  // TODO: File this!
	printf("ctrl^z\n");
	signal(SIGTSTP, SIG_IGN);
	return;
}
