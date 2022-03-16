CC= 		gcc
CFLAGS= 	-g -Wall -std=gnu99
TARGETS= 	mycalserver mycal

all: 		$(TARGETS)

src/Server/%.o: src/Server/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

src/Client/%.o: src/Client/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

mycal: src/Client/client.o src/Client/client_socket.o
	$(CC) $(CFLAGS) -o $@ $^

mycalserver: src/Server/server.o src/Server/server_socket.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm src/*/*.o $(TARGETS)
