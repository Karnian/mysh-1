#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "commands.h"
#include "built_in.h"

int bpid;
char **in;

char *path[6] = {
	"/usr/local/bin/",
	"/usr/bin/",
	"/bin/",
	"/usr/sbin/",
	"/sbin/"
};

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  if (n_commands > 0) {
    struct single_command* com = (*commands);

    assert(com->argc != 0);

    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
        if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
          fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } else if (strcmp(com->argv[0], "") == 0) {
      return 0;
    } else if (strcmp(com->argv[0], "exit") == 0) {
      return 1;
    } else {
	    int result = access(com->argv[0], 0);
	    if(result == 0)
	    {
//		    printf("Y\n");
		    in = com->argv;
		    pid_t pid = fork();
		    int status;
		    //not &, child
		    if(pid == 0 && strcmp(com->argv[(com->argc)-1], "&") != 0)
		    {
//			    printf("argv[1] : %s\n", com->argv[1]);
			    execv(com->argv[0], com->argv);
		    }
		    //&, child
		    else if(pid == 0 && strcmp(com->argv[(com->argc)-1],"&")==0)
		    {
			    int bbpid = fork();
			    in[(com->argc)-1] = NULL;
			    
			    if(bbpid == 0)
			    {
			//	    execv(in[0], in);

			    for(int i = 0; i < 20; i++)
			    {
				    printf("%d\n", i);
				    sleep(1);
			    }
			    exit(1);
			    }
			    else
			    {
				    printf("%d start\n", bbpid);
				    bpid = bbpid;
				    wait(&status);
				    
				    printf("%d DONE", bbpid);
				    for(int i = 0; i < com->argc - 1; i++)
					    printf(" %s", in[i]);
				    printf("\n");
				    exit(1);
			    }
		    }
		    //&, parent
		    else if(pid != 0 && strcmp(com->argv[(com->argc)-1],"&")==0)
		    {
			    return 0;
		    }
		    else if(pid != 0 && strcmp(com->argv[1], "&") != 0)
		    {
			    wait(&status);
		    }
		    else
			    return -1;
	    }
	    else
	    {
		    for(int i = 0; i < 6; i++)
		    {
			    char od[100];
			    strcat(od, path[i]);
			    strcat(od, com->argv[0]);
//			    printf("od : %s\n", od);
			    if(access(od, 0) == 0)
			    {
				    strcpy(com->argv[0], od);
				    int pid = fork();
				    int status;
				    if(pid == 0)
				    {
					    execv(com->argv[0], com->argv);
				    }
				    else
				    {
					    wait(&status);
					    return 0;
				    }
			    }
			    else
				    memset(od, 0, 100);
		    }
		    fprintf(stderr, "%s: command not found\n", com->argv[0]);
	    }
      return -1;
    }
  }

  return 0;
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
