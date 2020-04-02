#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

void file_error(const char* msg, const char* filename)
{
	fprintf(stderr, "Error: Could not %s file %s.\n", msg, filename);
	exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{

	int input_fd, output_fd;
	ssize_t num_read;
	char buf[BUFF_SIZE];

	input_fd = open(argv[0], O_RDONLY | S_IRUSR | S_IRGRP | S_IROTH);
	if (input_fd == -1)
		file_error("open", argv[0]);

	output_fd = open(argv[1], O_WRONLY | S_IRUSR | S_IRGRP | S_IROTH);
	if (output_fd == -1)
		output_fd = open(argv[1], O_WRONLY | O_CREAT | O_EXCL | S_IRUSR | S_IRGRP | S_IROTH);

	while ((num_read = read(input_fd, buf, BUFF_SIZE)) > 0)
		if (write(output_fd, buf, num_read) != num_read)
			file_error("write in", argv[1]);

	if (num_read == -1)
		file_error("read in", argv[0]);

	if (close(input_fd) == -1)
		file_error("close", argv[0]);

	if (close(output_fd) == -1)
		file_error("close", argv[1]);

	exit(EXIT_SUCCESS);
}