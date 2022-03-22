CC= 		gcc
CFLAGS= 	-g -Wall -std=gnu99 -Iinclude -fPIC -pthread 
TARGETS= 	mycalserver mycal

all: 		$(TARGETS)

src/cJSON.o: src/cJSON.c
	$(CC) $(CFLAGS) -c -o $@ $^

src/Server/%.o: src/Server/%.c 
	$(CC) $(CFLAGS) -c -o $@ $^

src/Client/%.o: src/Client/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

mycal: src/Client/client.o src/Client/client_socket.o
	$(CC) $(CFLAGS) -o $@ $^

mycalserver: src/Server/server.o src/Server/server_socket.o src/cJSON.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm src/*/*.o src/*.o $(TARGETS)
