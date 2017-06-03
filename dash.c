#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define RL_BUFF_SIZE 1024
#define TK_BUFF_SIZE 64
#define TOK_DELIM " \t\r\n\a"

//function declarations
void printtokens(char **);
void get_dir();
int dash_cd(char **);
char **split_line(char *);
char *read_line();
int dash_echo(char **);
int dash_ls(char **);
int dash_exit(char **);


int dash_exit(char **args)
{
	if(args[0] != NULL && strcmp(args[0], "exit") == 0)
		return 0;
	else
	{
		printf("dash: '%s' not a valid command\n", args[0]);
		return 1;
	}
}

int dash_ls(char **args)
{
	struct dirent *dirPointer;	//directory pointer
	DIR *dirReader = opendir(".");

	if(!dirReader)
		printf("dash: unable to list directory\n");

	while((dirPointer = readdir(dirReader)) != NULL)
		printf("%s\n", dirPointer->d_name);

	closedir(dirReader);
	return EXIT_SUCCESS;
}
	
int dash_echo(char **args)
{
	if(args[1] == NULL)
	{
		fprintf(stderr, "dash: Please enter an argument to echo\n");
	}
	else if(strcmp(args[0], "echo") == 0)	//move these checks  to another func later on
	{
		int i = 1;
		while(args[i] != NULL)
		{
			printf("%s ", args[i]);
			i++;
		}
		printf("\n");
	}
	return 1;
}

void get_dir()
{
	char cwd[1024];
	if(getcwd(cwd, sizeof(cwd))  != NULL)
	{
		printf("[%s] ", cwd);
	}
	else
		printf("getcwd() error");
}


int dash_cd(char **args)
{
	//get_dir();
	if(args[1] == NULL)
	{
		fprintf(stderr, "dash: Please enter a path to cd\n");
	}
	else if(strcmp(args[0], "cd") == 0)	//move this check to another func later on
	{
		//printf("%s", args[1]);
		if(chdir(args[1]) > 0)
		{
			perror("dash");
		}
	}
	return 1;
}

char **split_line(char *line)
{
	int buffsize = TK_BUFF_SIZE, position = 0;
	char **tokens = malloc(buffsize*sizeof(char*));
	char **token;

	if(!tokens)
	{
		fprintf(stderr, "allocation erro\n");	
		exit(EXIT_FAILURE);
	}
	token = strtok(line, TOK_DELIM);
	while(token != NULL)
	{
		tokens[position] = token;
		position++;

		if(position>=buffsize)
		{
			buffsize += TK_BUFF_SIZE;
			tokens = realloc(tokens, buffsize*sizeof(char*));

			if(!tokens)
			{
				fprintf(stderr, "allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, TOK_DELIM);
	}

	tokens[position] = NULL;
	//printtokens(tokens);
	return tokens;
}

void printtokens(char **tokens)
{
	int i = 0;
	while(tokens[i] != NULL)
	{
		printf("%s\n", tokens[i]);
		i++;
	}
}


char *read_line()
{
	int buffsize = RL_BUFF_SIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * buffsize);
	int c;

	if(!buffer)
	{
		fprintf(stderr, "allocation error");
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		c  = getchar();
		if (c == EOF || c == '\n')
		{
			//printf("\n"); 
			buffer[position] = '\0';
			return buffer;
		}
		else
		{
			buffer[position] = c;
		}
		position++;

		if (position >= buffsize)
		{
			printf("Overflow buffer..alloacating more memory\n"); //test
			buffsize += RL_BUFF_SIZE;
			buffer = realloc(buffer, buffsize);

			if(!buffer)
			{
				fprintf(stderr, "allocation error");
				exit(EXIT_FAILURE);
			}
		}
	}
}


void loop()
{
	char *line;
	char **args;
	int status=1;

	do{
		get_dir();
		printf("> ");
		line = read_line();
		args = split_line(line);
		//status = execute();
		status = dash_exit(args); 

		//free(line);
		//free(args);
	}while(status);
}


int main(int argc, char **argv)
{
	//config file for custom prompt by the user

	loop();

	return EXIT_SUCCESS;
}
