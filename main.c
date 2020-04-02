#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define SPLIT_SYMBOLS " \n\t\a\r"

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[0;34m"

char* command[] = {
	"exit",    // done
	"cd",      // done
	"ls",      // done
	"copy",    // done
	"mkdir",   // done
	"rmdir",   // done
	"mkfile",  // done
	"rmfile",  // done
	"clear",   // done
	"help"     // done
};

#define COMMAND_COUNT sizeof(command) / sizeof(char*)

char* read_line()
{
	int bufsize = BUFFER_SIZE;
	int pos = 0;
	char* buffer = malloc(sizeof(char*) * bufsize);
	int c;

	if (!buffer)
	{
		fprintf(stderr, "Error: Can't allocate memory to read line.\n");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		c = getchar();

		if (c == EOF || c == '\n')
		{
			buffer[pos] = '\0';
			return buffer;
		}
		else
			buffer[pos] = c;

		pos++;

		if (pos >= bufsize)
		{
			bufsize += BUFFER_SIZE;
			buffer = realloc(buffer, bufsize);

			if (!buffer)
			{
				fprintf(stderr, "Error: Can't allocate memory to read line.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char** split_line(char* line)
{
	int bufsize = BUFFER_SIZE;
	int pos = 0;

	char** tokens = malloc(sizeof(char*) * bufsize);
	if (!tokens)
	{
		printf("Error: Can't allocate memory to split line.\n");
		exit(EXIT_FAILURE);	
	}

	char* token = strtok(line, SPLIT_SYMBOLS);

	while (token != NULL)
	{
		tokens[pos] = token;
		pos++;

		if (pos >= bufsize)
		{
			bufsize += BUFFER_SIZE;
			tokens = realloc(tokens, bufsize);

			if (!tokens)
			{
				fprintf(stderr, "Error: Can't allocate memory to split line.\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, SPLIT_SYMBOLS);
	}

	tokens[pos] = NULL;

	return tokens;
}

void scan_directory()
{
	struct dirent** namelist;
	int n;

	n = scandir(".", &namelist, NULL, alphasort);
	if (n == -1) 
	{
 		perror("scandir");
 		return;
	}

	struct stat buffer;

	while (n--) 
	{
		if (n <= 1)
			break;

		if (stat(namelist[n]->d_name, &buffer) == 0 && S_ISDIR(buffer.st_mode))
			printf("%s%s%s ", BLUE, namelist[n]->d_name, RESET);
		else
			if (stat(namelist[n]->d_name, &buffer) == 0 && S_ISREG(buffer.st_mode))
				printf("%s ",namelist[n]->d_name);
		free(namelist[n]);
	}

	printf("\n");
	free(namelist);
}

void make_directory(char* dir)
{
	if (dir == NULL) 
	{
		fprintf(stderr, "Error: Excepted argument to \"mkdir\".\n");
		return;
	}

	struct stat buffer;

	if (stat(dir, &buffer) == 0 && S_ISDIR(buffer.st_mode))
	{
		fprintf(stderr, "Error: Can't create directory: already exist.\n");
		return;
	}
	if (mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
	{
		fprintf(stderr, "Error: Can't create directory.\n");
		return;
	}
}

void remove_directory(char* dir)
{
	if (dir == NULL) 
	{
		fprintf(stderr, "Error: Excepted argument to \"rmdir\".\n");
		return;
	}	

	if (rmdir(dir) != 0)
	{
		fprintf(stderr, "Error: Can't remove directory: no such directory.\n");
		return;
	}
}

void change_directory(char* dir)
{
	if (dir == NULL) 
	{
		fprintf(stderr, "Error: Excepted argument to \"cd\".\n");
		return;
	}

	struct stat buffer;

	if (stat(dir, &buffer) == 0 && S_ISDIR(buffer.st_mode))
	{
		if (chdir(dir) != 0)
		{
			fprintf(stderr, "Error: Can't change directory.\n");
			return;			
		}		
	} 
	else 
	{
		fprintf(stderr, "Error: No such file or directory.\n");
		return;	
	}
}

void make_file(char* filename)
{
	if (filename == NULL)
	{
		fprintf(stderr, "Error: Excepted argument to \"mkfile\".\n");
		return;
	}

	int fd;

	fd = open(filename, O_WRONLY);
	if (fd != -1)
	{
		fprintf(stderr, "Error: file %s already exist.\n", filename);
		if (close(fd) != 0)	
			fprintf(stderr, "Error: Existing file %s was not closed succussfully.\n", filename);
		return;		
	}

	fd = creat(filename, O_CREAT | O_TRUNC | S_IRWXU | S_IRWXG);
	if (fd == -1)
	{
		fprintf(stderr, "Error: Couldn't create file %s.\n", filename);
		return;
	}
}

void remove_file(char* filename)
{
	if (filename == NULL)
	{
		fprintf(stderr, "Error: Excepted argument to \"rmfile\".\n");
		return;
	}

	if (unlink(filename) == -1)
	{
		fprintf(stderr, "Error: Couldn't remove file %s\n", filename);
		return;
	}
}

void copy_file(char* input_file, char* output_file)
{
	if (input_file == NULL || output_file == NULL)
	{
		fprintf(stderr, "Error: Excepted 2 arguments to \"copy\".\n");
		return;
	}

	if (fork() == 0)
		execl("./copy", input_file, output_file, NULL);

	return;
}

void help()
{
	printf("This shell is created by jazzerX.\n");
	printf("The list of commands:\n");

	for (int i = 0; i < COMMAND_COUNT; ++i)
	{
		printf("> %s\n", command[i]);
	}

	return;
}

int execute(char** arg)
{
	for (int i = 0; i < COMMAND_COUNT; i++)
		if (strcmp(arg[0], command[i]) == 0)
		{
			if (command[i] == "exit")
				return 0;
			else 
				if (command[i] == "cd") 
				{
					change_directory(arg[1]);
					return 1;
				}
			else 
				if (command[i] == "ls") 
				{
					scan_directory(arg[1]);
					return 1;			
				}
			else
				if (command[i] == "mkdir") 
				{
					make_directory(arg[1]);
					return 1;			
				}
			else
				if (command[i] == "rmdir") 
				{
					remove_directory(arg[1]);
					return 1;
				}
			else
				if (command[i] == "mkfile")
				{
					make_file(arg[1]);
					return 1;
				}
			else
				if (command[i] == "rmfile")
				{
					remove_file(arg[1]);
					return 1;	
				}
			else
				if (command[i] == "copy")
				{
					copy_file(arg[1], arg[2]);
					return 1;
				}
			else
				if (command[i] == "clear")
				{
					system("clear");
					return 1;
				}
			else
				if (command[i] = "help")
				{
					help();
					return 1;
				}
		}

	printf("Command '%s' not found.\n", arg[0]);
	return -1;
}

int main()
{    
	system("clear");

	char hostname[_SC_HOST_NAME_MAX + 1];
	gethostname(hostname, _SC_HOST_NAME_MAX + 1);

	chdir(getenv("HOME"));
	char pathname[PATH_MAX + 1];
	getcwd(pathname, PATH_MAX + 1);

  	printf("%s%s%s:%s%s%s$ ", YELLOW, hostname, RESET, RED, pathname, RESET);

	char* line = read_line();
	char** arg = split_line(line);
	int status = execute(arg);

	while (status != 0)
	{
		getcwd(pathname, PATH_MAX + 1);
		printf("%s%s%s:%s%s%s$ ", YELLOW, hostname, RESET, RED, pathname, RESET);
		line = read_line();
		arg = split_line(line);
		status = execute(arg);
		free(line);
	}

	return 0;
}