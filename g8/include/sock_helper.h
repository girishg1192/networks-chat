#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define PORT "6667"
#define STDIN 0

int create_socket(struct addrinfo **servinfo_, char* host, char* port);
void fill_addrinfo(struct addrinfo *);
void get_host_name(int sockfd, char* ip);

int inet_pton(int, char*, void*);
int inet_ntop(int, void*, char*, int);

void set_listening_port(char *port_);
int get_listening_port();

/*
 * Print success/failure of command invocation
 */
void print_success(int status, char *command);

/*
 * FD SET Helper functions!
 */
void add_fd(int newfd);
void clear_fd(int oldfd);

fd_set wait_fd;
struct timeval tv;
int port;
int active_sockets;
