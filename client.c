/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "packet.h"

void error(const char *);
int main(int argc, char *argv[])
{
    int sock, n;
    unsigned int length;
    struct sockaddr_in server, from;
    struct hostent *hp;              //hostent - hostname parameter
    uint8_t buffer[PACKET_SIZE];                //buffer is buffer
    packet_t p;
    
    //Serializing...
    //bcopy(&p.seq_no, buffer);

    if (argc != 4) { 
        printf("Usage: server port\n");
        exit(1);
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        error("socket");

    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);      //returns hostent with the addr of server
    if (hp==0) error("Unknown host");

    memcpy((char *)&server.sin_addr,
            (char *)hp->h_addr,         //Copies the server address from hp to server addr struct
            hp->h_length);
    server.sin_port = htons(atoi(argv[2]));  //Converts the host byte order to network byte order
    
    length=sizeof(struct sockaddr_in);       //size of the address struct
    
    memset(buffer, 0, PACKET_SIZE);                       //Init buffer to zero
        
    //Conection Request.
    printf("Sending request.\n");
    p.type = REQ;
    p.seq_no = 0;
    memset(&(p.data), 0, MAX_DATA);
    encode(buffer, &p);
    n = sendto(sock,buffer,PACKET_SIZE,0,(const struct sockaddr *)&server,length);
    if (n < 0) error("Sendto");

    while (1) {
        n = recvfrom(sock,buffer,PACKET_SIZE,0,(struct sockaddr *)&from, &length);
        if (n < 0) printf("Connection Failed.\n");
        decode (buffer, &p);
        if (p.type == ACK) break;
    }
    printf("Connection established.\n\n");

    //File request
    printf("Sending File name.\n");
    p.type = FILE_REQ;
    p.seq_no += 1;
    p.length = sizeof(argv[3]);
    encode(buffer, &p);
    encodeFilename(buffer, argv[3]);
    n = sendto(sock,buffer,PACKET_SIZE,0,(const struct sockaddr *)&server,length);
    if (n < 0) error("Sendto");

    while (1) {
        n = recvfrom(sock,buffer,PACKET_SIZE,0,(struct sockaddr *)&from, &length);
        if (n < 0) printf("Connection Failed.\n");
        decode (buffer, &p);
        if (p.type == FILE_REQ_ACK) break;
    }
    printf("File Request approved.\n\n");

    //File sending.
    int fWrite = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fWrite < 0) error("File creation error.");

    while (1){
        n = recvfrom(sock,buffer,PACKET_SIZE,0,(struct sockaddr *)&from, &length);
        decode(buffer, &p);
        if (p.type == TERM) break;
        //printf("Got data.");
        write(fWrite, p.data, p.length);
        memset(buffer, 0, PACKET_SIZE);
        p.type = ACK;
        p.seq_no += 1;
        encode(buffer, &p);
        n = sendto(sock,buffer,PACKET_SIZE,0,(const struct sockaddr *)&server,length);
    }
    
    close(sock);
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}