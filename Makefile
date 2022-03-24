CC= 		gcc
CFLAGS= 	-g -Wall -std=gnu99 -Iinclude -fPIC -pthread 
TARGETS= 	mycalserver mycal

# variables
HEADERS = $(wildcard inc/*.h)
SERVER_OBJS = src/server.o src/server_socket.o src/event.o src/date.o src/cJSON.o src/server_func.o
CLIENT_OBJS = src/client.o src/client_socket.o

all: 		$(TARGETS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

mycal: $(CLIENT_OBJS) 
	$(CC) $(CFLAGS) -o $@ $^

mycalserver: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@echo "cleaning up"
	@rm -rf data/ $(SERVER_OBJS) $(CLIENT_OBJS) $(TARGETS)
