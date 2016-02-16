#include <stdio.h>
#include "sock_helper.h"

int server_start(char *port);

int server_accept(int sockfd);
void server_receive(int sockfd);

fd_set wait_fd;
struct timeval tv;
