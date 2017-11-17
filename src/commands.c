#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "commands.h"
#include "built_in.h"

#define SOCK_PATH "tpf_unix_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
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

void *cl_cr(struct single_command (*cc)[512])
{
	int client_sock, rc, len;
	struct sockaddr_un server_sockaddr; 
	struct sockaddr_un client_sockaddr; 

	client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (client_sock == -1) 
	{
		printf("SOCKET ERROR");
		exit(1);
	}

	client_sockaddr.sun_family = AF_UNIX;
	strcpy(client_sockaddr.sun_path, CLIENT_PATH);
	len = sizeof(client_sockaddr);

	unlink(CLIENT_PATH);
	rc = bind(client_sock, (struct sockaddr *) &client_sockaddr, len);
	if (rc == -1)
	{
		printf("BIND ERROR");
		close(client_sock);
		exit(1);
	}

	server_sockaddr.sun_family = AF_UNIX;
	strcpy(server_sockaddr.sun_path, SERVER_PATH);
	rc = connect(client_sock, (struct sockaddr *) &server_sockaddr, len);
	if(rc == -1)
	{
		printf("CONNECT ERROR");
		close(client_sock);
		exit(1);
	}

	int output = dup(STDOUT_FILENO);
	dup2(client_sock, STDOUT_FILENO);
	close(client_sock);

	evaluate_command(1, cc);

	close(STDOUT_FILENO);
	dup2(output, STDOUT_FILENO);
	close(output);

	close(client_sock);

	pthread_exit(NULL);
}
/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  if (n_commands > 0) {
	  if(n_commands == 2)
	  {
		  struct single_command *com1 = (*commands);
		  struct single_command *com2 = &((*commands)[1]);
		  struct sockaddr_un server_sockaddr, client_sockaddr;
		  int server_sock, client_sock, len, rc;
		  int bytes_rec = 0;

		  struct single_command secom[512];
		  memcpy(secom, com2, sizeof(struct single_command));

		  char buff[512];
		  int backlog = 10;
		  void *status;

		  memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
		  memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
		  memset(buff, 0, 512);

		  //server creation
		  server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
		  if(server_sock == -1)
		  {
			  printf("SERVER CREATION ERR\n");
			  exit(1);
		  }

		  //setting
		  server_sockaddr.sun_family = AF_UNIX;
		  strcpy(server_sockaddr.sun_path, SOCK_PATH);
		  len = sizeof(server_sockaddr);

		  unlink(SOCK_PATH);

		  //listening
		  rc = bind(server_sock,(struct sockaddr*)&server_sockaddr,len);
		  if(rc == -1)
		  {
			  printf("L ERR\n");
			  close(server_sock);
			  exit(1);
		  }
		  printf("Listening\n");

		  pthread_t chthread;
		  int tid = pthread_create(&chthread,NULL,(void*)cl_cr,&com1);
                  if(tid)
		  {
				  printf("TH ERR\n");
				  exit(1);
		  }

		  //accept incoming connection
		  client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &len);
		  if (client_sock == -1)
		  {
		        printf("ACCEPT ERROR\n");
			close(server_sock);
			close(client_sock);
			exit(1);
		  }

		  //etc
		  pthread_join(chthread, &status);

		  int input = dup(STDIN_FILENO);
		  dup2(client_sock, STDIN_FILENO);
		  close(client_sock);

		  evaluate_command(1, &secom);

		  close(STDIN_FILENO);
		  dup2(input, STDIN_FILENO);
		  close(input);
		  close(server_sock);
		  return 0;
	  }
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
		    for(int i = 0; i < 5; i++)
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
	    else
		    fprintf(stderr, "%s: command not found\n", com->argv[0]);
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
