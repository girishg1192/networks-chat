#include "sock_helper.h"
int client_connect(char *host, char *port);
void client_send(int sockfd, char *message);
void client_identify(int fd);
