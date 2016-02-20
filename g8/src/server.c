#include "server.h"
#define MAX_NUMBER 8

struct list client_list;
void add_client(int fd, char *hostname);
int close(int);
struct client_info* find_client_by_fd(int fd);
void print_client(struct client_info *print);

int server_start(char *port)
{
  int err;
  struct addrinfo *servinfo; // will point to the results
  printf("Starting server\n");

  //Create a socket
  int sockfd = create_socket(&servinfo, NULL, port);

  //Do server stuff
  //err = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  
  struct sockaddr_in in;
  bzero(&in, sizeof(in));
  char *end;
  int hport = htons(strtol(port, &end, 10));
  in.sin_family = AF_INET;
  in.sin_addr.s_addr = htonl(INADDR_ANY);
  in.sin_port = hport;
  printf("%d sockfd\n", sockfd);

  err = bind(sockfd, (struct sockaddr *) &in, sizeof(in));
  printf("bind %d ",err);
  if(err) return -1;
  err = listen(sockfd, MAX_NUMBER);
  freeaddrinfo(servinfo);
  list_init(&client_list);
//  server_accept(sockfd);
  return sockfd;
}
int server_accept(int sockfd)
{
  struct sockaddr_storage client_addr;
  char host_name[128];
  socklen_t size_address = sizeof(struct sockaddr_storage);
  int newfd = accept(sockfd, (struct sockaddr *) &client_addr, &size_address);
  add_client(newfd, host_name);
  return newfd;
}
void add_client(int newfd, char *host_name)
{
  int argc =0;
  /* Parse the data sent from the client
   */
  get_host_name(newfd, host_name);

  /*
   * create a client_info structure
   */
  struct client_info *connection = malloc(sizeof(struct client_info));
  strcpy(connection->ip_addr, host_name);
  connection->port = -1;
  connection->sockfd = newfd;
  list_insert_ordered(&client_list, &connection->elem, (list_less_func *)&sort_port, NULL);
}
void add_info_to_client(char *args, int fd, struct client_info *connection)
{
  char *arg, *temp;
  int argc = 0;
  char client_auth[4][255];

  for(arg = strtok_r(args, " ", &temp); arg; arg = strtok_r(NULL, " ", &temp))
  {
    strcpy(client_auth[argc], arg);
    argc++;
  }
  char *end;
  connection->port = strtol(client_auth[0], &end, 10);
  strcpy(connection->hostname, client_auth[1]);
  print_client(connection);
}
void server_receive(int sockfd)
{
  char msg[255];//= "Server!!\0";
  int ret, i=0;
  ret = recv(sockfd, msg, 256, 0);
  if(ret>0)
  {
    get_host_name(sockfd, NULL);
    msg[ret]='\0';
    printf("%s\n", msg);
    struct client_info* transmitting_client = find_client_by_fd(sockfd);
    if(transmitting_client->port == -1)
      add_info_to_client(msg, sockfd, transmitting_client);
  }
  if(ret == 0)
  {
  //  printf("Connection closed\n");
    server_kill(sockfd);
  }
}
void server_kill(int sockfd)
{
  close(sockfd);
  clear_fd(sockfd);
}
struct client_info* find_client_by_fd(int fd)
{
  struct client_info* find_client;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_info, elem);
    //printf("Loop 1 %d %d\n", find_client->sockfd, fd);
    if(find_client->sockfd == fd)
      return find_client;
    //printf("next\n");
  }
  //Nothing found
  return NULL;
}
void print_client(struct client_info *print)
{
  printf("%d %s %s\n", print->port, print->ip_addr, print->hostname);
}
