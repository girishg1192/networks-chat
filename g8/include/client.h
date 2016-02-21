#include "sock_helper.h"

//For gethostname()
#include <unistd.h>
#include "list.h"

int client_connect(char *host, char *port);
int client_send(int sockfd, char *message);
void client_identify(int fd);
void client_receive(int fd);
void client_kill(int fd);
void client_list();

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

