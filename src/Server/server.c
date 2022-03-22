#include "../../inc/server_socket.h"
#include "../../inc/cJSON.h"

typedef struct Date Date;

struct Date {
	int month;
	int day;
	int year;
};

Date get_date(int d) {
	Date date = {0};
	date.year = d % 100;
	date.day = (d / 100) % 100;
	date.month = (d / 10000) % 100;

	return date;
}

int put_date(Date date) {
	return (((date.month * 100) + date.day) * 100) + date.year;
}

void print_date(Date date) {
	printf("%d\n", date.month);
	printf("%d\n", date.day);
	printf("%d\n", date.year);
}

typedef struct Event Event;

struct Event {
	int valid;
	Date date;
	int time;
	int duration;
	char name[256];
	char description[2048];
	char location[1024];
	int id; 
};

void print_event(Event event) {
	if (event.valid) {
		puts("bad event");
		return;
	}
	printf("%d\n", put_date(event.date));
	printf("%d\n", event.time);
	printf("%d\n", event.duration);
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
		event.valid = 1;
		return event;
	}
	Date date = get_date(item->valueint);
	event.date = date;

	// Time
	item = cJSON_GetObjectItemCaseSensitive(request, "time");
	if (item == NULL) {
		event.valid = 1;
		return event;
	}
	event.time = item->valueint;

	
	// Duration
	item = cJSON_GetObjectItemCaseSensitive(request, "duration");
	if (item == NULL) {
		event.valid = 1;
		return event;
	}
	event.duration = item->valueint;


	// Name
	item = cJSON_GetObjectItemCaseSensitive(request, "name");
	if (item == NULL) {
		event.valid = 1;
		return event;
	}
	strcpy(event.name, item->valuestring);

		
	// optional
	// description
	item = cJSON_GetObjectItemCaseSensitive(request, "description");
	if (item != NULL) {	
		strcpy(event.description, item->valuestring);

	}

	// location
	item = cJSON_GetObjectItemCaseSensitive(request, "location");
	if (item != NULL) {	
		strcpy(event.location, item->valuestring);

	}

	cJSON_Delete(request);
	return event;
}

char* event_to_string(Event event) {
	if (event.valid) 
		return NULL;
	cJSON *event_json = cJSON_CreateObject();
	
	cJSON_AddNumberToObject(event_json, "date", put_date(event.date));
	cJSON_AddNumberToObject(event_json, "time", event.time);
	cJSON_AddNumberToObject(event_json, "duration", event.duration);
	cJSON_AddStringToObject(event_json, "name", event.name);
	if (event.description[0])
		cJSON_AddStringToObject(event_json, "description", event.description);
	if (event.description[0])
		cJSON_AddStringToObject(event_json, "location", event.location);

	return cJSON_Print(event_json);
}

int main(int argc, char *argv[]) {
	
	printf("Hello World\n");
	char buff[BUFSIZ];

	FILE *fp = fopen(argv[1], "r");
	int n;
	n = fread(buff, 1, BUFSIZ, fp);
	buff[n-1] = '\0';
	printf("%s %d\n", buff, n);
	
	Event event = create_event(buff);
	print_event(event);	
	char *message = event_to_string(event);
	if (message) 
		printf("%s\n", message);
	event = create_event(message);
	print_event(event);

	char *port = argv[2];
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
		printf("%s\n", buffer);

		Event event = create_event(buff);
		print_event(event);	
	}



	return 0;
}

