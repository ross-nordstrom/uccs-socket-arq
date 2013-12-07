// UDP Echo Client

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "udp_stop-and-wait.h"

#define SERVER_UDP_PORT         5000
#define MAXLEN                  4096
#define DEFLEN                  64

long delay(struct timeval t1, struct timeval t2)
{
   long d;
   d = (t2.tv_sec - t1.tv_sec) * 1000;
   d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
   return(d);
}

void print_usage(char *pname)
{
  fprintf(stderr,
  "Usage: %s [-s data_size] host [-p port] filename protocol\n\
   protocol: 0=Basic, 1=Stop-and-Wait, 2=Go Back N, 3=Selective Repeat\n", pname);
}

int bufferRandom(char *sbuf, int data_size) {
  int i, j;
  for (i = 0; i < data_size; i++) {
     j = (i < 26) ? i : i % 26;
     sbuf[i] = 'a' + j;
  } // construct data to send to the server
  return data_size;
}

int bufferFile(char *sbuf, char *filename) {
  return 0;
}

int main(int argc, char **argv)
{
    int     data_size = DEFLEN, port = SERVER_UDP_PORT, protocol;
    int     i, sd, server_len;
    char    *pname, *host, *filename, rbuf[MAXLEN], sbuf[MAXLEN];
    struct  hostent         *hp;
    struct  sockaddr_in     server;
    struct  timeval         start, end;
    unsigned long  address;

    pname = argv[0];
    argc--;
    argv++;
    if (argc > 0 && (strcmp(*argv, "-s") == 0)) {
       if (--argc > 0 && (data_size = atoi(*++argv))) {
          argc--;
          argv++;
       } else {
          print_usage(pname);
          exit(1);
       }
    }
    if (argc > 0) {
       host = *argv;
       if (--argc > 0) {
        if(strcmp(*argv, "-p") == 0)
          port = atoi(*++argv);
       } else {
        print_usage(pname);
        exit(1);
       }
       if(argc == 2) {
        filename = *++argv;
        protocol = atoi(*++argv);
       } else {
        print_usage(pname);
        exit(1);
       }
    } else {
       print_usage(pname);
       exit(1);
    }

    printf("START host=%s, port=%d, filename=%s, protocol=%d\n", host, port, filename, protocol);

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
       fprintf(stderr, "Can't create a socket\n");
       exit(1);
    }

    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL) {
       fprintf(stderr, "Can't get server's IP address\n");
       exit(1);
    }

    bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length);
 
    if (data_size > MAXLEN) {
       fprintf(stderr, "Data is too big\n");
       exit(1);
    }

    data_size = bufferRandom(sbuf, data_size);

    gettimeofday(&start, NULL); /* start delay measurement */
    server_len = sizeof(server);
    if (sendto(sd, sbuf, data_size, 0, (struct sockaddr *)
       &server, server_len) == -1) {
       fprintf(stderr, "sendto error\n");
       exit(1);
    }
    if (recvfrom(sd, rbuf, MAXLEN, 0, (struct sockaddr *)
       &server, &server_len) < 0) {
       fprintf(stderr, "recvfrom error\n");
       exit(1);
    }
    gettimeofday(&end, NULL); /* end delay measurement */
    if (strncmp(sbuf, rbuf, data_size) != 0)
       printf("Data is corrupted\n");
    close(sd);
    return(0);
}