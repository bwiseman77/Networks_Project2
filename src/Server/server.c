#include "../../inc/server_socket.h"
#include "../../inc/cJSON.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
pthread_t threads[10];
int thread_count = 0;

void sig_handler(int signo) {
	if (signo == SIGINT) {
	
		int i;
		for (i = 0; i < thread_count; i++) {
			pthread_join(threads[i], NULL);
		}
		exit(EXIT_SUCCESS);	

	}

}
typedef struct Arg Arg;

struct Arg {
	char buffer[BUFSIZ];
	int fd;
};

typedef struct Date Date;

struct Date {
	char month[10];
	char day[10];
	char year[10];
};

void print_date(Date date) {
	printf("%s\n", date.month);
	printf("%s\n", date.day);
	printf("%s\n", date.year);
}

Date get_date(char *d) {
	Date date;
	date.month[0] = d[0];
	date.month[1] = d[1];
	date.month[2] = '\0';
	date.day[0] = d[2];
	date.day[1] = d[3];
	date.day[2] = '\0';
	date.year[0] = d[4];
	date.year[1] = d[5];
	date.year[2] = '\0';

	return date;
}

void put_date(Date date, char *str) {
	sprintf(str, "%s%s%s", date.month, date.day, date.year);	
}

typedef struct Event Event;

struct Event {
	int valid;
	Date date;
	char time[30];
	char duration[30];
	char name[256];
	char description[2048];
	char location[1024];
	char id[30]; 
};

void print_event(Event event) {
	if (event.valid) {
		puts("bad event");
		printf("%d\n", event.valid);
		return;
	}
	printf("%s\n", event.id);
	char str[30];
	put_date(event.date, str);
	printf("%s\n", str);
	printf("%s\n", event.time);
	printf("%s\n", event.duration);
	printf("%s\n", event.name);
	if (event.description[0]) 
		printf("%s\n", event.description);
	if (event.location[0])
		printf("%s\n", event.location);
}

Event create_event(char *buff) {
	Event event = {0};

	cJSON *request = cJSON_Parse(buff);
	cJSON *item = NULL;
	if (request == NULL)
		event.valid = 1;

	// required 
	// Date
	item = cJSON_GetObjectItemCaseSensitive(request, "date");
	if (item == NULL) {
		event.valid = 2;
		return event;
	}
	Date date = get_date(item->valuestring);
	event.date = date;

	// Time
	item = cJSON_GetObjectItemCaseSensitive(request, "time");
	if (item == NULL || item->valuestring == NULL ) {
		event.valid = 3;
		return event;
	}
	strcpy(event.time, item->valuestring);

	
	// Duration
	item = cJSON_GetObjectItemCaseSensitive(request, "duration");
	if (item == NULL || item->valuestring == NULL ) {
		event.valid = 4;
		return event;
	}
	strcpy(event.duration, item->valuestring);


	// Name
	item = cJSON_GetObjectItemCaseSensitive(request, "name");

	if (item == NULL || item->valuestring == NULL ) {
		event.valid = 5;
		return event;
	}
	strcpy(event.name, item->valuestring);

		
	// optional
	// description
	item = cJSON_GetObjectItemCaseSensitive(request, "description");
	if (item != NULL) {	
		if (item->valuestring == NULL ) {
			event.valid = 6;
			return event;
		}
		strcpy(event.description, item->valuestring);

	}

	// location
	item = cJSON_GetObjectItemCaseSensitive(request, "location");
	if (item != NULL) {	
		if (item->valuestring == NULL ) {
			event.valid = 7;
			return event;
		}
		strcpy(event.location, item->valuestring);

	}

	// id
	item = cJSON_GetObjectItemCaseSensitive(request, "id");
	if (item == NULL) {
		char str[30];
		put_date(event.date, str);
		sprintf(event.id, "%s%s", str, event.time);
	} else {
		if (item->valuestring == NULL) {
			event.valid = 8;
			return event;
		}
		strcpy(event.id, item->valuestring);
	}
			
	cJSON_Delete(request);
	return event;
}

char* event_to_string(Event event) {
	if (event.valid) 
		return NULL;
	cJSON *event_json = cJSON_CreateObject();
	char str[30];
	put_date(event.date, str);

	cJSON_AddStringToObject(event_json, "date", str);
	cJSON_AddStringToObject(event_json, "id", event.id);
	cJSON_AddStringToObject(event_json, "time", event.time);
	cJSON_AddStringToObject(event_json, "duration", event.duration);
	cJSON_AddStringToObject(event_json, "name", event.name);
	if (event.description[0])
		cJSON_AddStringToObject(event_json, "description", event.description);
	if (event.description[0])
		cJSON_AddStringToObject(event_json, "location", event.location);


	char *string = cJSON_Print(event_json);
	cJSON_Delete(event_json);
	return string;
}

void write_event(char *path, Event event) {
	FILE *file = fopen(path, "w+");

	char *string = event_to_string(event);
	fwrite(string, 1, (strlen(string) + 1), file);
	free(string);
	fclose(file);
}

int update_event(char *path, char *field, char *value) {
	FILE *infile = fopen(path, "r");

	char buffer[BUFSIZ];
	fread(buffer, 1, BUFSIZ, infile);
	Event event = create_event(buffer);
	if (!strcmp(field, "date")) {
		event.date = get_date(value);
	} else if (!strcmp(field, "time")) {
		strcpy(event.time, value);	
	} else if (!strcmp(field, "duration")) {
		strcpy(event.duration, value);
	} else if (!strcmp(field, "name")) {
		strcpy(event.name, value);
	} else if (!strcmp(field, "description")) {
		strcpy(event.description, value);
	} else if (!strcmp(field, "location")) {
		strcpy(event.location, value);
	}
	fclose(infile);

	write_event(path, event);

	return 1;

}

int add_event(char *json, char *id) {
	char path[40];
	Event event = create_event(json);

	if (event.valid) 
		return 1;	
	char *cal = "mycal";
	char *year = event.date.year;
	char *month = event.date.month;
	char *day = event.date.day;
	strcpy(id,event.id);


	sprintf(path, "./data/%s", cal);
	mkdir(path, 0777);

	sprintf(path, "./data/%s/%s", cal, year);
	mkdir(path, 0777);

	sprintf(path, "./data/%s/%s/%s", cal, year, month);
	mkdir(path, 0777);

	sprintf(path, "./data/%s/%s/%s/%s", cal, year, month, day);
	mkdir(path, 0777);

	sprintf(path, "./data/%s/%s/%s/%s/%s.json", "mycal", year, month, day, id);
	
	write_event(path, event);	
	
	return 0;
}

Event get_event(char *path) {
	char buffer[BUFSIZ];
	FILE *infile = fopen(path, "r");
	fread(buffer, 1, BUFSIZ, infile);
	Event event = create_event(buffer);
	return event;
}

int find_event(char *root, char *id, char dest[100][BUFSIZ]) {
	int n = 0;
	struct stat s;
	stat(root, &s);
	if (S_ISDIR(s.st_mode)) {
		DIR *d = opendir(root);

		for (struct dirent *e = readdir(d); e; e = readdir(d)) {
			if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) {
				continue;
			}
	
			char path[BUFSIZ];
			sprintf(path, "%s/%s", root, e->d_name);

			stat(path, &s);
			if (S_ISDIR(s.st_mode)) {
				n += find_event(path, id, dest);
			} else {

				if (strstr(path, id) != NULL) {
					strcpy(dest[n], path);
					n++;
				}
			}
		}

		closedir(d);
	}

	return n;
}

int find_event_range(char *start, char *stop) {
	char curr_day[30];
	char dest[100][BUFSIZ];
	strcpy(curr_day, start);

	int num_events = 0;
	while(strcmp(curr_day, stop)) {
		int m;
		if ((m = find_event("./data", curr_day, dest)) > 0) {
			num_events+=m;

			for (int i = 0; i < m; i++) {
				char buffer[BUFSIZ];
				FILE *infile = fopen(dest[i], "r");
				fread(buffer, 1, BUFSIZ, infile);
				Event event = create_event(buffer);
				print_event(event);
				fclose(infile);
			}
		}
		
		Date date = get_date(curr_day);

		
		int n = atoi(date.day) + 1;
		if (n < 10) {
			sprintf(date.day, "0%d", n);
		} else {
			sprintf(date.day, "%d", n);
		}

		if (atoi(date.day) > 31) {
			strcpy(date.day, "00");
			
			int n = atoi(date.month) + 1;
			if (n < 10) {
				sprintf(date.month, "0%d", n);
			} else {
				sprintf(date.month, "%d", n);
			}

			if (atoi(date.month) > 12) {
				strcpy(date.month, "00");
					
				int n = atoi(date.year) + 1;
				if (n < 10) {
					sprintf(date.year, "0%d", n);
				} else {
					sprintf(date.year, "%d", n);
				}


				if (atoi(date.year) > 99) {
					strcpy(date.year, "00");
				}
			}
		}

		put_date(date, curr_day);


	}	
	return num_events;
}

char *send_response(char *cmd, char *cal, char *id, char *success, char *msg, char *data) {
	cJSON *response = cJSON_CreateObject();

	cJSON_AddStringToObject(response, "Command", cmd);
	cJSON_AddStringToObject(response, "Calendar", cal);
	cJSON_AddStringToObject(response, "id", id);
	cJSON_AddStringToObject(response, "success", success);
	cJSON_AddStringToObject(response, "message", msg);
	cJSON_AddStringToObject(response, "data", data);

	char *str = cJSON_Print(response);
	cJSON_Delete(response);
	return str;
}


void *handle_request(void *args) {
	char buffer[BUFSIZ];	
	Arg *arg = (Arg *)args;
	int fd = arg->fd;
	char *cmd = strtok(arg->buffer, " ");
	if (!strcmp(cmd, "add")) {
		puts("command: add");
		char ID[BUFSIZ];
		char *json = strtok(NULL, "\0");
		if (add_event(json, ID)) {
			puts("invalid event");
		} else {
			printf("event id: %s\n", ID);
			char *msg = send_response("add", "mycal", ID, "true", " ", " ");
			strcpy(buffer, msg);
			buffer[strlen(msg) + 1] = '\0';
			send(fd, buffer, (strlen(buffer)+1), 0);
			
			printf("%s", buffer);
			fflush(stdout);
			free(msg);
		}


	} else if (!strcmp(cmd, "get")) {
		puts("command: get");

		char *id = strtok(NULL, "\n");
		char dest[100][BUFSIZ];
		int n;	
		if ( (n = find_event("./data", id, dest)) > 0) {
			for (int i = 0; i < n; i++) {
				FILE *infile = fopen(dest[i], "r");
				fread(buffer, 1, BUFSIZ, infile);
				Event event = create_event(buffer);
				print_event(event);
			}
		} else {
			puts("could not find id");
		}


	} else if (!strcmp(cmd, "remove")) {
		puts("command: remove");
		char *id = strtok(NULL, "\n");
		char dest[100][BUFSIZ];
		if (find_event("./data", id, dest)) {
			remove(dest[0]);				
			puts("removed");
		} else {
			puts("could not find id");
		}


	} else if (!strcmp(cmd, "update")) {
		puts("command: update");

		char *id = strtok(NULL, " ");
		char *field = strtok(NULL, " ");
		char *value = strtok(NULL, "\n");
		char dest[100][BUFSIZ];
		if (find_event("./data", id, dest)) {
			update_event(dest[0], field, value);			
			puts("updated");
		} else {
			puts("could not find id");
		}


	} else if (!strcmp(cmd, "getrange")) {
		puts("command: getrange");
		char *start = strtok(NULL, " ");
		char *stop = strtok(NULL, "\n");
		int n = find_event_range(start, stop);
		printf("%d events found\n", n);
	}

	return 0;	
}

int main(int argc, char *argv[]) {
	int mt = 0;
	if (argc == 3) {
		if (!strcmp(argv[2], "-mt")) {
			mt = 1;
		}
	}
	if (signal(SIGINT, sig_handler) != SIG_ERR) {
	}
	mkdir("./data", 0777);	
	printf("Hello World\n");
	char *port = argv[1];
	int server_fd = socket_listen(port);
	if (server_fd < 0)
		return EXIT_FAILURE;

	printf("listening on port %s\n", port);

	while(1) {
		puts("\nwaiting for client");
		int fd = socket_accept(server_fd);
		puts("accepted connection");
		char buffer[BUFSIZ];
			
		recv(fd, buffer, BUFSIZ, 0);
		printf("message: %s", buffer);

		Arg args;
		strcpy(args.buffer, buffer);
		args.fd = fd;

		if (mt) {
			pthread_create(&threads[thread_count], NULL, handle_request, &args);
			thread_count++;
		
			if (thread_count > 10) {
				int i;
				for (i = 0; i < 10; i++) {
					pthread_join(threads[i], NULL);
				}
				thread_count = 0;
			}
		} else {
			pthread_create(&threads[0], NULL, handle_request, &args);
			pthread_join(threads[0], NULL);
		}
	}

	return 0;
}

