int send_udp(int client_len, struct sockaddr_in client, int sd, char *buf, int n, int dropRate);
int receive_udp(int client_len, struct sockaddr_in client, int sd, char *buf, int n);