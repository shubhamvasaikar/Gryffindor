main: server client

client: ./client.c ./packet.h
	gcc -g client.c -o client

server: ./server.c ./packet.h
	gcc -g server.c -o server

clean:
	rm client server
