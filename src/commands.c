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
int len;

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
	    if(result == 0 && n_commands == 1)
	    {
//		    printf("Y\n");
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
			    printf("%d\n", getpid());

			    com->argv[(com->argc)-1] = NULL;
			    
//			    execv(com->argv[0], com->argv);

			    //test function
			    for(int i = 0; i < 5; i++)
			    {
				    printf("%d\n", i);
				    sleep(1);
			    }
			    exit(1);
		    }
		    //&, parent
		    else if(pid != 0 && strcmp(com->argv[(com->argc)-1],"&")==0)
		    {
//			    printf("bp start\n");

			    //need malloc
			    len = com->argc - 1;
//			    printf("len = %d\n", len);
//			    for(int i = 0; i < len; i++)
//				    printf("%s\n", com->argv[i]);
			    in = (char**)malloc(com->argc*sizeof(char*));
			    for(int i=0;i<len;i++)
			    {
				    int size = strlen(com->argv[i]);
				    in[i] = (char*)malloc(sizeof(char)*(size+1));
				    strcpy(in[i],com->argv[i]);
			    }
//			    printf("malloc done\n");
//			    strcpy(in[len], "\0");
			    bpid = pid;

			    return 0;
		    }
		    else if(pid != 0 && strcmp(com->argv[(com->argc)-1],"&")!=0)
		    {
//			    printf("p is wating\n");
			    wait(&status);
		    }
		    else
			    return -1;
	    }
	    else if(result != 0 && n_commands == 1)
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
	    else if(n_commands == 2)
	    {

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
