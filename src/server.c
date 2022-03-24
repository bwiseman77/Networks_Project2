/* server.c */

#include "../inc/server_func.h"

/* Globals */

int thread_count = 0;
pthread_t threads[MAX_THREADS] = {0};

/* main execution */

int main(int argc, char *argv[]) {
	// set up with config
	char buffer[BUFSIZ] = {0};
	FILE *fp = fopen(".mycal", "r");
	fread(buffer, 1, BUFSIZ, fp);
	fclose(fp);
    cJSON *config = cJSON_Parse(buffer);
	cJSON *item = cJSON_GetObjectItemCaseSensitive(config, "port");
	if (item == NULL) {
		puts("invalid config file");
		exit(EXIT_FAILURE);
	}

	char port[SMALL_BUFF];
	strcpy(port, item->valuestring);
	cJSON_Delete(config);

	// check for mt flag
	int mt = 0;
	if (argc == 2) {
		if (!strcmp(argv[1], "-mt")) {
			mt = 1;
		}
	}

	// register handler
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		exit(EXIT_FAILURE);
	}

	// make data directory
	mkdir("./data", 0777);

	// connect to port
	int server_fd = socket_listen(port);
	if (server_fd < 0)
		exit(EXIT_FAILURE);

	printf("listening on port %s\n", port);

	// main loop
	while(1) {
		puts("\nwaiting for client");
		int fd = socket_accept(server_fd);
		puts("accepted connection");

		// get message		
		recv(fd, buffer, BUFSIZ, 0);
		printf("message: %s", buffer);

		// create args to send to thread
		Arg args;
		strcpy(args.buffer, buffer);
		args.fd = fd;

		// if multi thread
		if (mt) {
			pthread_create(&threads[thread_count], NULL, handle_request, &args);
			thread_count++;
		
			// once max threads, join and reset count
			if (thread_count > MAX_THREADS) {
				for (int i = 0; i < MAX_THREADS; i++) {
					pthread_join(threads[i], NULL);
				}
				thread_count = 0;
			}

		// single thread
		} else {
			pthread_create(&threads[0], NULL, handle_request, &args);
			pthread_join(threads[0], NULL);
		}
	}

	exit(EXIT_SUCCESS);
}

