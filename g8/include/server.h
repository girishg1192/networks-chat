#include <stdio.h>
#include "sock_helper.h"

int server_start(char *port);

int server_accept(int sockfd);
void server_receive(int sockfd);
void server_kill(int sockfd);
void print_connected_client_list();

void print_blocked_clients(char *ip);

void print_stats();

fd_set wait_fd;
struct timeval tv;

struct client_info
{
  int port;
  int sockfd;
  char ip_addr[INET_ADDRSTRLEN];
  char hostname[128];

  bool is_connected;
  int sent_msg;
  int recv_msg;

  struct list blocked_list;

  struct list_elem elem;
};

static bool sort_port(struct list_elem *al,struct list_elem *bl,
          void *aux) 
{
  struct client_info *a = list_entry(al, struct client_info, elem);
  struct client_info *b = list_entry(bl, struct client_info, elem);
  return a->port < b->port;
}
