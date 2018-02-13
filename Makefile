main: server client

client: ./client/client.c ./packet.h
	gcc -g ./client/client.c -o ./client/client.o

server: ./server/server.c ./packet.h
	gcc -g ./server/server.c -o ./server/server.o

clean:
	rm ./client/client.o ./client/*.txt ./server/server.o
