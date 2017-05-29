void loop()
{
	char *line;
	char **args;
	int status;

	do{
		printf("->");
		line = read_line();
		args = split_line(line);
		status = execute();

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
