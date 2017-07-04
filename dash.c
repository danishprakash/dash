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
#include <ctype.h>

#define RL_BUFF_SIZE 1024
#define TK_BUFF_SIZE 64
#define TOK_DELIM " \t\r\n\a"


char *clr[2] = {"clear", NULL};

//ANSI Color codes
#define RED   		"\033[0;31m"
#define YELLOW 		"\033[0;33m"
#define CYAN 		"\033[0;36m"
#define GREEN 		"\033[0;32m"
#define BLUE 		"\033[0;34m"
#define INVERT		"\033[0;7m"
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
int dash_pipe(char **);
int args_length(char **);
char *trimws(char *);			//trim leading and trailing whitespace
char **split_pipes(char *);
	

/* array of builtin function pointers */
int (*builtin_funcs[])(char **) = { &dash_cd, &dash_help, &dash_exit, &dash_history, &dash_grep, &args_length };

/* string array of builtin commands for strcmp() before invoking execvp() */
char *builtin_str[] = { "cd", "help", "exit" , "history", "grep", "sizeof" };

/* return the size of the builtin array */
int builtin_funcs_count()
{
	return sizeof(builtin_str) / sizeof(char *);
}



/*-----------------------------
 * function definitions *
-------------------------------*/


char *trimws(char *str)
{
	char *end;
	while(isspace((unsigned char) *str)) str++;
	if(*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char) *end)) end--;
	*(end+1) = 0;
	return str;
}


char **split_pipes(char *input)
{
	//char input[] = "cat dash.c | less | cat | grep | which";
	char *p = strtok(input, "|");
	char **s = malloc(1024*sizeof(char *));
	int i = 0;
	while(p != NULL)
	{
		
		s[i] = trimws(p);
		i++;
		//printf("%s_\n", p);
		p = strtok(NULL, "| ");
	}
	s[++i] = NULL;
	i=0;
	while(s[i] != NULL)
	{
		printf("%s\n", s[i]);
		i++;
	}
	return s;
}

int args_length(char **args)
{
	int i = 0;

	while(args[i] != NULL)
	{
		i++;
	}
	printf("%d\n", i);
	return i;
}

int dash_pipe(char **args)
{
	/*saving current stdin and stdout for restoring*/
	int tempin=dup(0);			
	int tempout=dup(1);			
	int j=0, i=0, flag=0;
	int fdin, fdout;
	pid_t cpid;
	while(args[j] != NULL)
	{
		if(strcmp(args[j], "<") == 0)
		{
			//args[j] = NULL;
			fdin=open(args[j+1], O_RDONLY);
			flag += 2;
			//break;
		}
		else if(strcmp(args[j], ">") == 0)
		{
			fdout=open(args[j+1], O_WRONLY);
			flag += 2;
			//break;
			//args[j] = args[j+1] = NULL;		//enable after initial testing
			//break;
		}
		j++;
	}
	if(!fdin)
		fdin=dup(tempin);
	if(!fdout)
		fdout=dup(tempout);
	int k=0;						/*have to use split_pipe instead of split_line for args*/
//	while(args[k] != NULL)
//	{
//		printf("%s\n", args[i]);
//		i++;
//	}
	for(i=0; i<args_length(args)-flag; i++)
	{
		dup2(fdin, 0);
		close(fdin);
	//	if((strcmp(args[i], "|")) == 0)
	//		continue;

		//if(i == args_length(args))
		
		int fd[2];
		pipe(fd);
		
		fdin = fd[0];
		fdout = fd[1];
		
		dup2(fdout, 1);
		//printf("above pipe(fd)\n");
		close(fdout);
		cpid = fork();
		if(cpid == 0)
		{
			//printf("inside child\n");
			execvp(args[i], args);
			perror("error forking\n");
			exit(EXIT_FAILURE);
		}
	}

	dup2(tempin, 0);
	dup2(tempout, 1);
	close(tempin);
	close(tempout);

	return 1;
}


//int dash_pipe(char **arg1, char **arg2)
//{
//	int fd[2], cpid;
//	pipe(fd);
//
//	int _stdin = dup(STDIN_FILENO);
//
//	if((cpid = fork()) == 0)
//	{
//		close(STDOUT_FILENO);
//		dup(fd[1]);
//		close(fd[0]);
//		dash_execute(arg1);
//		exit(EXIT_FAILURE);
//	}
//	else if(cpid > 0)
//	{
//		close(STDIN_FILENO);
//		dup(fd[0]);
//		close(fd[1]);
//		dash_execute(arg2);
//		dup2(_stdin, STDIN_FILENO);
//		return 1;
//	}
//	return 1;
//}


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



/* shows user it's recent history of entered commands and waits for user input
 * 
 * q - exit
 * line number - executes that particular command again from history */
int dash_history()
{
	printf("inside history\n");
	FILE *fp = fopen(get_hist_file_path(), "r");
	int ch, c, line_num = 1;
	char line[128];
	char prev_comm[128];
	char **args=NULL;
	//int len;
	if(!fp)
		fprintf(stderr, RED "dash: file not found" RESET "\n");
	else
	{
		while((c = getc(fp)) != EOF)
		{
			putchar(c);
		}
	}
	printf( "\n" INVERT " <0>: Quit    <#line>: Execute respective command    <-1>: clear history file " RESET "\n\n: ");
	scanf("%d", &ch);
	getchar();
	fseek(fp, 0, SEEK_SET);
	if(isdigit(ch) != 0)
	{
		printf("please enter a numerical choice\n");	
	}
	else if (ch == 0)
	{	
		fclose(fp);
		return 1;//dash_execute(clr);
	}
	else if(ch == -1)
	{
		fclose(fp);
		fp = fopen(get_hist_file_path(), "w");
		//fprintf(fp, "");
		fclose(fp);
		return dash_execute(clr);
	}

	else
	{
		
	   	while((fgets(line, 128, fp)) != NULL)
	   	{
			//printf("%d %d\n", ch ,line_num);
			if(line_num == ch)
			{

				//printf("inside if\n");
				strcpy(prev_comm, &line[3]);
//				printf("%s\n", prev_comm);
				args = split_line(prev_comm);
				//len = strlen(*args);
				fclose(fp);
				//printf("**len:%d, *args[len]:%s\n", len, *args);
				//*args[len-1] = '\0';	
				return dash_launch(args);	
	
			}
			else
				line_num++;
				
	   	}
	}	
	//printf("end of history\n");
	//fclose(fp);
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
	//printf("inside execute\n");
	pid_t cpid;
	int status;
	cpid = fork();

	if(cpid == 0)
	{	
		//printf("inside child\n");
		if(execvp(args[0], args) < 0)
			printf("dash: command not found: %s\n", args[0]); 
			//perror(RED "dash: " RESET);
		exit(EXIT_FAILURE);
		
	}
	else if(cpid < 0)
		printf(RED "Error forking" RESET "\n");
	else
	{    
		//do {
      			waitpid(cpid, &status, WUNTRACED);
    		//} while (!WIFEXITED(status) && !WIFSIGNALED(status));
//		do
//		{	
//			printf("inside parent\n");
//		}while(wait(NULL)>0);
//		return 1;
		//printf("end of parent\n");
	}
	//printf("end of execute\n");
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
	//printf("inside launch\n");
	FILE *history_file = NULL;
	int i = 0, j = 0;

	if(args[0] == NULL)
	{
		//printf("inside args null\n");
		return 1;
	}
	else if(strcmp(args[0], "history") != 0)		//excluding the history command
	{
		//printf("here with history\n");
//		char file_path[128];
//		strcat(strcpy(file_path, getenv("HOME")), "/.dash_history");
//		printf("inside launch else\n");
		history_file = fopen(get_hist_file_path(), "a+");
//		printf("file opened\n");
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
	int m = 0;
//	while(args[m] != NULL)
//	{
//		if(!strcmp("|", args[m]))
//		{
//			//char **arg2=NULL;
//			//args[m] = NULL;
//			//arg2 = &args[m+1];
//			printf("inside pipe call\n");
//			return dash_pipe(args);
//			break;
//		}
//		m++;
//	}
	for(i = 0; i<builtin_funcs_count(); i++)
	{
		if(strcmp(args[0], builtin_str[i]) == 0)
		{
			//printf("inside stcmp(builtin)\n");
			return (*builtin_funcs[i])(args);	
			//exit(EXIT_FAILURE);
		}
	}
	//printf("end of launch\n");
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
	int line_num = 1;
	if(args[0] != NULL && strcmp(args[0], "grep") == 0)
	{
		if(args[1] != NULL && args[2] != NULL)
		{
			fp = fopen(args[2], "r");
			while((fgets(temp, 512, fp)) != NULL)
			{
				if(strstr(temp, args[1]))
				{
					printf("%d. %s", line_num, temp);
					flag = 1;
				}
				line_num++;
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

/*
 * 
 */


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
 * 
 * status var controlled while loop
 * every iteration first prints the prompt(cwd) 
 * then proceeds to read user input using the func read_line()
 * the input is then split into tokens using the split_line() func
 * the returned stream of tokens are then passed onto the launch func
 * which returns 0 or 1 depending upon the execution,
 * the loop exits accordingly
********************/
void loop()
{
	char *line;
	char **args;
	int status=1, i = 0, flag = 0;
	
	//signal(SIGINT, signalHandler);

	do{

		//printf("status loop\n");
		get_dir("loop");
		printf(CYAN "> " RESET);
		//printf("> ");
		line = read_line();
		flag = 0;
		while(line[i] != '\0')
		{
			if(line[i] == '|')
			{
				printf("inside if\n");
				flag = 1;
				break;
			}
			i++;
		}
		if(flag)
		{
				args = split_pipes(line);
				status = dash_pipe(args);
		}
		else
		{
			args = split_line(line);
			status = dash_launch(args);
		}
		//args = split_line(line);
		//printf("**line:%s, args:%s \n", line, *args);
		//status = execute();
		//status = dash_launch(args); 
		free(line);
		free(args);
	}while(status);
}


int main(int argc, char **argv)
{
	//config file for custom prompt by the user

	loop();

	return EXIT_SUCCESS;
}
