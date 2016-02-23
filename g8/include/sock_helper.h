#ifndef __SOCKHELPH__
#define __SOCKHELPH__
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>

#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include "list.h"
#include "logger.h"

#define PORT "6667"
#define MAX_LENGTH 512
#define STDIN 0
#define MY_PC
#ifdef MY_PC
#define LOG printf
#else
#define LOG cse4589_print_and_log
#endif

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

bool validate_ip(char *ip);

fd_set wait_fd;
struct timeval tv;
int port;
int active_sockets;

struct ip_info
{
  char ip_addr[INET_ADDRSTRLEN];
  int port;
  struct list_elem elem;
};
#if IMPLEMENT_BLOCK_SORT
static bool sort_port(struct list_elem *al,struct list_elem *bl,
          void *aux) 
{
  struct ip_info *a = list_entry(al, struct ip_info , elem);
  struct ip_info *b = list_entry(bl, struct ip_info , elem);
  return a->port > b->port;
}
#endif
#endif
