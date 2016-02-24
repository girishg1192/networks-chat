#include <stdio.h>
#include "sock_helper.h"

int server_start(char *port);

int server_accept(int sockfd);
void server_receive(int sockfd);
void server_kill(int sockfd);
void print_connected_client_list();

int print_blocked_clients(char *ip);

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
  struct list inbox;

  struct list_elem elem;
};

struct queued_msg
{
  char message[256];
  struct list_elem elem;
};

static bool sort_port(struct list_elem *al,struct list_elem *bl,
          void *aux) 
{
  struct client_info *a = list_entry(al, struct client_info, elem);
  struct client_info *b = list_entry(bl, struct client_info, elem);
  return a->port < b->port;
}
static bool sort_block(struct list_elem *al,struct list_elem *bl,
          void *aux) 
{
  struct ip_info *a = list_entry(al, struct ip_info, elem);
  struct ip_info *b = list_entry(bl, struct ip_info, elem);
  return a->port < b->port;
}
