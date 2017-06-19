#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>


#define RL_BUFF_SIZE 1024
#define TK_BUFF_SIZE 64
#define TOK_DELIM " \t\r\n\a"


//ANSI Color codes
#define RED   		"\033[0;31m"
#define YELLOW 		"\033[0;33m"
#define CYAN 		"\033[0;36m"
#define GREEN 		"\033[0;32m"
#define BLUE 		"\033[0;34m"
#define RESET  		"\e[0m" 
#define BOLD		"\e[1m"
#define ITALICS		"\e[3m"



/*****************************
 * function declarations *
******************************/

void printtokens(char **);
void get_dir(char *);
void signalHandler();
char **split_line(char *);
char *read_line();
int dash_cd(char **);
int dash_echo(char **);
int dash_ls(char **);
int dash_exit(char **);
int dash_pwd(char **);		
int dash_tail(char **);			
int dash_help(char **);
int dash_head(char **);
int dash_cat(char **);
int dash_touch(char **);
int dash_help(char **);
int dash_grep(char **);
int dash_file(char **);			//file name and file size
int dash_launch(char **);
int dash_execute(char **);
int history_line_count();
int dash_history();

/* array of builtin function pointers */
int (*builtin_funcs[])(char **) = { &dash_cd, &dash_help, &dash_exit, &dash_history };

/* string array of builtin commands for strcmp() before invoking execvp() */
char *builtin_str[] = { "cd", "help", "exit" , "history" };

/* return the size of the builtin array */
int builtin_funcs_count()
{
	return sizeof(builtin_str) / sizeof(char *);
}



/*-----------------------------
 * function definitions *
-------------------------------*/

char *get_hist_file_path()
{
	//char *home_dir = getenv("HOME");
	//char *fname = "/.dash_history";
	//char *file_path = malloc(strlen(home_dir)+1);
	static char file_path[128];
	strcat(strcpy(file_path, getenv("HOME")), "/.dash_history");
//	file_path = realloc(file_path, strlen(fname));
//	strcpy(file_path, home_dir);
//	strcpy(file_path, fname);

	return file_path;
}

int dash_history()
{
	FILE *fp = fopen(get_hist_file_path(), "r");
	int ch, c, i=0, line_num = 1;
	char line[128];
	if(!fp)
		fprintf(stderr, RED "dash: file not found" RESET "\n");
	else
	{
		while((c = getc(fp)) != EOF)
		{
			putchar(c);
		}
	}
	scanf("%d", &ch);
	fseek(fp, 0, SEEK_SET);
	if (ch == 'q' || ch == 'Q')
		return 1;
	else
	{
		
	   	while((fgets(line, 128, fp)) != NULL)
	   	{
			//printf("%d %d\n", ch ,line_num);
			if(line_num == ch)
			{
				//printf("inside if\n");
				puts(line);
				return 1;	
			}
			else
				line_num++;
				
	   	}
	}
	printf("outside else\n");
	fclose(fp);
	return 1;
}
			

int history_line_count()
{
//	char file_path[128];
//	strcat(strcpy(file_path, getenv("HOME")), "/.dash_history");
	FILE *fp = fopen(get_hist_file_path(), "r");
	int c;
	int numOfLines = 1;
	do
	{	
		c = getc(fp);
		if(c == '\n')
		{
			numOfLines++;	
		}
	}while(c != EOF);
	return numOfLines;
}





/*******************************************************************
 * This func allows the shell to catch Ctrl-c and not exit it until
 * the exit command is entered explicitly
 *******************************************************************/
void signalHandler()
{
	signal(SIGINT, signalHandler);
	getchar();
}


/***************************************************************************** 
 * Executes the system call execvp with the tokenized user input as argument
 * the execvp() call takes place in a child process
 * the parent waits until the child has finished processing
 *****************************************************************************/
int dash_execute(char **args)
{
	pid_t cpid;//, ppid;
	int status;
	cpid = fork();

	if(cpid == 0)
	{	
		if(execvp(args[0], args) == -1)
			perror(RED "dash: " RESET);
		return 0;
	}
	else if(cpid < 0)
		printf(RED "Error forking" RESET "\n");
	else
	{
		if(tcsetpgrp(STDOUT_FILENO, getpid()) < 0) 	//setting the current process in the
			perror("tcgetpgrp() error\n");		//child process the foreground process
		//parent process
		waitpid(cpid, &status, WUNTRACED);
	}
	return 1;

}


/****************************************************************
 * Checks if the command entered by user is a builtin or not
 * and if it is, then invoke the required builtin functions
 * and if it not, then invoke the execute() method which in turn
 * will execute the systemcall execvp(args)
 ****************************************************************/

int dash_launch(char **args)
{
	FILE *history_file = NULL;
	int i = 0, j = 0;
	if(args[0] == NULL)
		return 1;
	else
	{
//		char file_path[128];
//		strcat(strcpy(file_path, getenv("HOME")), "/.dash_history");
		printf("inside launch else\n");
		history_file = fopen(get_hist_file_path(), "a+");
		printf("file opened\n");
		j = 0;
//		printf("%s\n", file_path);
		fprintf(history_file, "%d. ", history_line_count());
		while(args[j] != NULL)
		{
			if(j > 0)
				fputs(" ", history_file);
			fputs(args[j], history_file);
			j++;
		}
		fputs("\n", history_file);
		fclose(history_file);
	}
	for(i = 0; i<builtin_funcs_count(); i++)
	{
		if(strcmp(args[0], builtin_str[i]) == 0)
		{
			printf("inside stcmp(builtin)\n");
			return (*builtin_funcs[i])(args);	
		}
	}
	printf("end of launch\n");
	return dash_execute(args);

}


int dash_file(char **args)
{
	int fp=0;
	struct stat st;
	if(args[1] != NULL && strcmp(args[0], "file") == 0)
	{	
		fp = open(args[1], O_RDONLY);
		if(fstat(fp, &st) > -1)
		{
			printf("Name:\t%s\nSize:\t%ld bytes\n", args[1], st.st_size);
		}	
		else
			printf( RED BOLD "dash: unable to access file" RESET "\n");
		close(fp);
	}
	return 1;
}


int dash_grep(char **args)
{
	FILE *fp = NULL;
	int flag = 0;
	char temp[512];
	if(args[0] != NULL && strcmp(args[0], "grep") == 0)
	{
		if(args[1] != NULL && args[2] != NULL)
		{
			fp = fopen(args[2], "r");
			while((fgets(temp, 512, fp)) != NULL)
			{
				if(strstr(temp, args[1]))
				{
					printf("%s", temp);
					flag = 1;
				}
			}
			fclose(fp);
		}
		else
		{
			fprintf(stderr, RED "dash: grep requires two params, " ITALICS "PATTERN" RESET RED " and " RED ITALICS "FILE" RESET "\n");
		}
	}
	if(flag == 0)
		printf(ITALICS "No matches were found" RESET "\n");
	return 1;
}


int dash_help(char **args)
{
	if(args[0] != NULL && strcmp(args[0], "help") == 0)
	{
		fprintf(stderr,"\n------\n" 
				BOLD "\ndash " RESET "is a basic unix terminal shell written purely in C developed by Danish Prakash\n"
				"\nSupported Commands:\n1. " ITALICS "cd" RESET "\n2. " ITALICS "exit" RESET "\n3. " ITALICS "help" RESET "\n4. " ITALICS "touch" RESET "\n5. " ITALICS "cat" RESET 
				"\n\n------\n\n");
	}
	return 1;
}

int dash_touch(char **args)
{
	FILE *fp;
	if(args[0] != NULL && strcmp(args[0], "touch") == 0)
	{
		if(args[1] == NULL)
		{
			printf(RED "dash: 'touch' requires an argument" RESET "\n");

			//return 1;
		}
		else
		{
			fp = fopen(args[1], "w");
			fclose(fp);
			//return 1;
		}
	}
	return 1;
}

int dash_cat(char **args)
{
	FILE *fp = NULL;
	int c;
	if(args[0] != NULL && args[1] != NULL && strcmp(args[0], "cat") == 0)
	{
		fp = fopen(args[1], "r");
		if(!fp)
		{
			fprintf(stderr, "%sdash: File not found%s\n", RED, RESET);
			return 1;
		}
		else
		{
			while((c = getc(fp)) != EOF)
			{
				putchar(c);
			}
		}
		fclose(fp);
	}
	return 1;
}

int dash_head(char **args)
{
	FILE *fp = NULL;
	int c, i=0;
	if(args[0] != NULL && strcmp(args[0], "head") == 0)
	{
		if(args[1] == NULL)
		{
			printf("%sdash: 'head' requires an argument%s\n", RED, RESET);
			return 1;
		}
		else
			fp = fopen(args[1], "r");
		if(!fp)
		{
			printf("%sdash: File not found%s\n", RED,RESET);		
			return 1;
		}
		else
		{
			 //fseek(fp, SEEK_SET, 50);
	 		 while((c = getc(fp)) != EOF || i < 10)
			 {
				 putchar(c);
				 if(c == '\n')
					 i++;	
			 }		 
		}
		fclose(fp);
	}
	return 1;
}

int dash_tail(char **args)
{
	FILE *fp = NULL;
	int c, i=0;
	if(args[0] != NULL && args[1] != NULL && strcmp(args[0], "tail") == 0)
	{	
		fp = fopen(args[1], "r");
		if(!fp)
		{	
			printf("%sdash: File not found%s\n", RED, RESET);
		}
		else
		{
			fseek(fp, -150, SEEK_END);
			//pos = ftell(fp);
			//printf("%d\n", pos);
			//fseek(fp, SEEK_END-10, SEEK_END);
			//printf("\n");
			while((c = getc(fp)) != EOF && i < 10)
			{
				putchar(c);
				if(c == '\n')
					i++;
			}
		}
		fclose(fp);
	}
	return 1;
}	


int dash_pwd(char **args)
{
	if(args[0] != NULL && strcmp(args[0], "pwd") == 0)
		get_dir("pwd");	
	else
		printf("dash: '%s' not a valid command\n", args[0]);
	return 1;
}
		

int dash_exit(char **args)
{

	return 0;
//	if(args[0] != NULL && strcmp(args[0], "exit") == 0)
//		return 0;
//	else
//	{
//		printf("dash: '%s' not a valid command\n", args[0]);
//		return 1;
//	}
}

int dash_ls(char **args)
{
	struct dirent *dirPointer;	//directory pointer
	DIR *dirReader = opendir(".");

	if(!dirReader)
		printf("%sdash: unable to list directory\n", RED);

	while((dirPointer = readdir(dirReader)) != NULL)
		printf("%s\n", dirPointer->d_name);

	closedir(dirReader);
	return 1;
}
	
int dash_echo(char **args)
{
	if(args[1] == NULL)
	{
		fprintf(stderr, "%sdash: Please enter an argument to echo%s\n", YELLOW, RESET);
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

void get_dir(char *state)
{
	char cwd[1024];
	if(getcwd(cwd, sizeof(cwd)) != NULL)
	{
		if(strcmp(state, "loop") == 0)
			printf(RED "[ " RESET CYAN "%s" RESET RED " ] " RESET, cwd);
			//printf("%s[ %s%s %s]%s ", RED, CYAN, cwd, RED, RESET); 	//change colors back to def
		else if(strcmp(state, "pwd") == 0)
			printf("%s\n", cwd);
	}
	else
	{
		printf("%sgetcwd() error%s", RED, RESET);
	}
}


int dash_cd(char **args)
{
	printf(YELLOW "OWN" RESET "\n");
	//get_dir();
	if(args[1] == NULL)
	{
		fprintf(stderr, "%sdash: Please enter a path to cd%s\n", YELLOW, RESET);
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
		fprintf(stderr, "%sdash: Allocation error%s\n", RED, RESET);	
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
				fprintf(stderr, "%sdash: Allocation error%s\n", RED, RESET);
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
		fprintf(stderr, "%sdash: Allocation error%s\n", RED, RESET);
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
				fprintf(stderr, "%sdash: Allocation error%s\n", RED, RESET);
				exit(EXIT_FAILURE);
			}
		}
	}
}

/******************* 
 * driving function
********************/
void loop()
{
	char *line;
	char **args;
	int status=1;
	
	//signal(SIGINT, signalHandler);

	do{
		get_dir("loop");
		printf("%s>%s ", CYAN, RESET);
		//printf("> ");
		line = read_line();
		args = split_line(line);
		//status = execute();
		status = dash_launch(args); 

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
