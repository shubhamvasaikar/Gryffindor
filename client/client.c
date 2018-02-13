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
#include "../packet.h"


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sock, n;
    unsigned int length;
    struct sockaddr_in server, from;
    struct hostent *hp;              //hostent - hostname parameter
    uint8_t buffer[PACKET_SIZE];                //buffer is buffer
    packet_t p;
    int debug = 1;
    
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
        else error("File request error\n");
    }
    printf("File Request approved.\n\n");

    //File sending.
    int fWrite = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fWrite < 0) error("File creation error.");

    int flag = 1;
    int prev_seq_no = 0;

    while (1){
        n = recvfrom(sock,buffer,PACKET_SIZE,0,(struct sockaddr *)&from, &length);
        decode(buffer, &p);
        if (debug == 1) printf("Got seq: %d\n",p.seq_no);
        
        //Uncommenting the next line will cause 
        //the client to drop every 100th packet.
        //Use this to test retransmission.
        //if (p.seq_no % 100 == 0) (flag == 1) ? (flag = 0) : (flag = 1);
        
        if (p.type == TERM) break;

        if (p.seq_no != prev_seq_no) write(fWrite, p.data, p.length);
        memset(buffer, 0, PACKET_SIZE);
        p.type = ACK;
        encode(buffer, &p);
        if (flag)
            n = sendto(sock,buffer,PACKET_SIZE,0,(const struct sockaddr *)&server,length);
        prev_seq_no = p.seq_no;
    }
    
    close(sock);
    return 0;
}