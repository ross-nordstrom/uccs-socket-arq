// UDP Echo Client

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../inc/utils.h"
#include "../inc/saw.h"


void print_usage(char *pname)
{
  fprintf(stderr,
  "Usage: %s [-s data_size] host [-p port] src_filename dst_filename protocol\n\
   protocol: 0=Basic, 1=Stop-and-Wait, 2=Go Back N, 3=Selective Repeat\n", pname);
}

int main(int argc, char **argv)
{
    int     data_size = DEFLEN, port = SERVER_UDP_PORT, protocol;
    int     i, sd, server_len, bytes, num_frames;
    char    *pname, *host, *src_filename, *dst_filename, rbuf[BUF_SIZE], sbuf[BUF_SIZE];
    struct  hostent         *hp;
    struct  sockaddr_in     server;
    struct  timeval         start, end;
    unsigned long  address;

    // Seed the random number generator
    srand(time(NULL));

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
       if(argc == 3) {
        src_filename = *++argv;
        dst_filename = *++argv;
        protocol = atoi(*++argv);
       } else {
        print_usage(pname);
        exit(1);
       }
    } else {
       print_usage(pname);
       exit(1);
    }

    printf("START host=%s, port=%d, src_filename=%s, dst_filename=%s, protocol=%d\n", 
                host, port, src_filename, dst_filename, protocol);

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
       printf("Can't create a socket\n");
       exit(1);
    }

    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL) {
       printf("Can't get server's IP address\n");
       exit(1);
    }

    bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length);
 
    if (data_size > MAXLEN) {
       printf("Data is too big\n");
       exit(1);
    }

    bytes = readFile(sbuf, src_filename);

    // printf("   START Send data at %s\n", start);

    server_len = sizeof(server);

    num_frames = calculateNumFrames(bytes, data_size);

    switch(protocol) {
      case STOP_AND_WAIT:
        if( -1 == send_saw(server_len, server, sd, sbuf, num_frames, data_size, bytes, 0)) {
          printf("END [FAILURE] Error sending via UDP\n");
          return(1);
        }
        bytes = receive_saw(&server_len, &server, sd, rbuf, &data_size, 0);
        break;
      case GO_BACK_N:
        printf("END [FAILURE] GO BACK N not implemented yet\n");
        return(1);
        break;
      case SELECTIVE_REPEAT:
        printf("END [FAILURE] SELECTIVE REPEAT not implemented\n");
        return(1);
        break;
      default:
        print_usage(pname);
        return(1);
        break;
    }
    
    if (strncmp(sbuf, rbuf, bytes) != 0)
       printf("Data is corrupted\n");
    close(sd);

    printf("START Dump %d B of echo data into file: %s\n", bytes, dst_filename);
    data_size = writeFile(rbuf, dst_filename, bytes);
    printf("END Dumped %d B of echo data into file\n", data_size);

    return(0);
}