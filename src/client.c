#include "../inc/client_socket.h"

int main(int argc, char *argv[]) {
	printf("Hello World\n");

	
	char * host = argv[1];
	char * port = argv[2];
	char * cal = argv[3];
	char * cmd = argv[4];
	char * file = argv[5];

	char buffer[BUFSIZ];
	char message[BUFSIZ];

	FILE *fp = fopen(file, "r");
	int n = fread(buffer, 1, BUFSIZ, fp);

	int fd = socket_dial(host, port);
	buffer[n] = '\0';
	strcat(message, cal);
	strcat(message, " ");
	strcat(message, cmd);
	strcat(message, " ");
	strcat(message, buffer);
	printf("%s\n", message);
	send(fd, message, BUFSIZ, 0);   

	recv(fd, buffer, 2*BUFSIZ, 0);
	printf("%s", buffer);
	return 0;
}

