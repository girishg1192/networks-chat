#include "client.h"

struct list connected_list;
void client_fill_list();

int client_connect(char *host, char *port)
{
  struct addrinfo *servinfo; // will point to the results
  //Create a socket
  int sockfd = create_socket(&servinfo, host, port);
  connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  get_host_name(sockfd, NULL);
  freeaddrinfo(servinfo);
  list_init(&connected_list);
  return sockfd;
}
void client_identify(int fd)
{
  char command[132];
  char host_name[128];
  gethostname(host_name, 128);
  sprintf(command, "%d %s", get_listening_port(), host_name);
  printf("%s\n", command);
  client_send(fd, command);
}
void client_send(int sockfd, char *buf)
{
  int ret;
  ret = send(sockfd, buf, strlen(buf), 0);
}
void client_close(int sockfd)
{
  close(sockfd);
}
void client_receive(int sockfd)
{
  char msg[255];//= "Server!!\0";
  memset(msg, 0, 255);
  int ret, i=0;
  ret = recv(sockfd, msg, 256, 0);
  if(ret>0)
  {
    char *temp = NULL;
    char *command = strtok_r(msg, " ", &temp);
    if(!strcmp(command, "LIST"))
    {
      printf("LIST = %s\n", temp);
      client_fill_list(temp);
    }
    else if(!strcmp(command, "MSG"))
    {
      printf("[EVENT]%s\n", temp);
    }
  }
  if(ret == 0)
  {
    client_kill(sockfd);
  }
}
void client_kill(int sockfd)
{
  close(sockfd);
  clear_fd(sockfd);
}
void client_fill_list(char *id)
{
  char *tmp = NULL;
  char argv[3][255];
  int argc = 0;
  char *end = NULL;
  for(char * arg = strtok_r(id, " ", &tmp); arg; arg = strtok_r(NULL, " ", &tmp))
  {
    strcpy(argv[argc++], arg);
  }
  printf("parsed %s %s %s\n", argv[0], argv[1], argv[2]);
  struct client_logged *add_client = malloc(sizeof(struct client_logged *));
  add_client->port = strtol(argv[0], &end, 10);
  strcpy(add_client->hostname, argv[1]);
  strcpy(add_client->ip_addr, argv[2]);
  list_insert_ordered(&connected_list, &add_client->elem, 
      (list_less_func *)&sort_port_client, NULL);
}
void print_client_list()
{
  struct client_logged* id;
  int list_id=1;
  for(struct list_elem *iter = list_begin(&connected_list); iter!=list_end(&connected_list);
      iter = list_next(iter))
  {
    id = list_entry(iter, struct client_logged, elem);
    printf("%s\n", "in here?");
    printf("%-5d%-35s%-20s%-8d\n", list_id++, id->hostname, id->ip_addr, id->port);
  }
  //Nothing found
}
