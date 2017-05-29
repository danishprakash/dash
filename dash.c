#include <stdlib.h>
#include <stdio.h>

#define RL_BUFF_SIZE 1024

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
		printf("->");
		line = read_line();
		//args = split_line(line);
		//status = execute();

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
