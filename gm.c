#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/types.h>
#include <errno.h>
#ifndef MAX_BUF
#define MAX_BUF 200
#endif
#define GM_RL_BUFSIZE 1024
#define GM_TOK_BUFSIZE 64
#define GM_TOK_DELIM " \t\r\n\a"



int gm_cd(char **args);
int gm_help(char **args);
int gm_exit(char **args);

char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&gm_cd,
	&gm_help,
	&gm_exit
};

int gm_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

int gm_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "gm: expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0){
			perror("gm");
		}
	}
	return 1;
}

int gm_help(char **args)
{
	int i;
	printf("Built in:\n");

	for (i = 0; i < gm_num_builtins(); i++) {
		printf(" %s\n", builtin_str[i]);
	}
	return 1;
}

int gm_exit(char **args)
{
	return 0;
}


			
	

int gm_launch(char **args){
	pid_t pid, wpid;
	int status;

	pid = fork();

	if (pid == 0) {
		//Child process
		if (execvp(args[0], args) == -1){
			perror("gm");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {
		// error forking
		perror("gm");
	} else {
		//parent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while  (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int gm_execute(char **args)
{
	int i;

	if (args[0] == NULL) {
		return 1;
	}

	for (i = 0; i < gm_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}
	
	return gm_launch(args);
}


char **gm_split_line(char *line)
{
	int bufsize = GM_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "gm: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, GM_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position ++;

		if (position >= bufsize) {
			bufsize += GM_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "gm: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	token = strtok(NULL, GM_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

	


char *gm_read_line(void)
{
	int bufsize = GM_RL_BUFSIZE;
	int position= 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;
	
	if (!buffer){
		fprintf(stderr, "gm: allocation error\n");
		exit(EXIT_FAILURE);
	}
			
	
	while (1) {
		// Read a character
		c = getchar();
		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		}
		else {
			buffer[position] = c;
		}
		position++;

		if (position >= bufsize){
			bufsize += GM_RL_BUFSIZE;
			buffer = realloc(buffer,bufsize);
			if (!buffer) {
				fprintf(stderr, "gm: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}


void gm_loop(void)
{
	char *line;
	char **args;
	int status;
	do {
		char path[MAX_BUF];
		getcwd(path, MAX_BUF);
		char *uid = getenv("USER"); //Get username
		printf("[%s %s]$ ", uid,path);	
		line = gm_read_line();
		args = gm_split_line(line);
		status = gm_execute(args);

		free(line);
		free(args);
	}
	while (status);
}




int main(int argc, char **argv)
{
	gm_loop();

	return EXIT_SUCCESS;

}


