#include "sock_helper.h"

//For gethostname()
#include <unistd.h>

int client_connect(char *host, char *port);
int client_send(int sockfd, char *message);
int client_identify(int fd);
void client_receive(int fd);
void client_kill(int fd);
void client_list();
int client_send_msg(int sockfd, char *buf);
void add_to_block_list(char *ip);
void remove_from_block_list(char *ip);
void client_receive_file(int *sockfd_);
void client_send_file(char *ip, char *file_name);

void print_client_list();
struct ip_info* is_client_blocked(char *ip);

bool verify_ip(char *ip);

struct client_logged
{
  int port;
  char ip_addr[INET_ADDRSTRLEN];
  char hostname[128];
  struct list_elem elem;
};
static bool sort_port_client(struct list_elem *al,struct list_elem *bl,
          void *aux) 
{
  struct client_logged *a = list_entry(al, struct client_logged, elem);
  struct client_logged *b = list_entry(bl, struct client_logged, elem);
  return a->port < b->port;
}

