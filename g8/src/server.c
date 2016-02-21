#include "server.h"
#define MAX_NUMBER 8

struct list client_list;
struct client_info* add_client(int fd);
int close(int);
struct client_info* find_client_by_fd(int fd);
void print_client(struct client_info *print);
void server_send_list(int sockfd);
void send_to_client(int sockfd, char *msg);
void send_to_all(int, char *message);
struct client_info* find_client_by_ip_port(char *ip, int port);

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
  return newfd;
}
struct client_info* add_client(int newfd)
{
  char host_name[128];
  int argc =0;
  /* Parse the data sent from the client
   */
  get_host_name(newfd, host_name);
  /*
   * create a client_info structure
   */
  struct client_info *connection = malloc(sizeof(struct client_info));
  strcpy(connection->ip_addr, host_name);
  connection->sockfd = newfd;
  connection->sent_msg = 0;
  connection->recv_msg = 0;
  list_insert_ordered(&client_list, &connection->elem, (list_less_func *)&sort_port, NULL);
  return connection;
}
void add_info_to_client(char *args, int fd)
{
  char *arg, *temp;
  int argc = 0;
  char client_auth[4][255];
  struct client_info *connection;
  int set_port;
  char ip[128];
  get_host_name(fd, ip);

  for(arg = strtok_r(args, " ", &temp); arg; arg = strtok_r(NULL, " ", &temp))
  {
    strcpy(client_auth[argc], arg);
    argc++;
  }
  char *end;
  set_port = strtol(client_auth[0], &end, 10);
  if((connection = find_client_by_ip_port(ip, set_port)) == NULL)
  {
    connection = add_client(fd);
    connection->port = set_port;
    strcpy(connection->hostname, client_auth[1]);
  }
  else
    connection->sockfd = fd;
  print_client(connection);
  server_send_list(fd);
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
    printf("RECEIVE: %s\n", msg);
    char *temp = NULL;
    char *sig = strtok_r(msg, " ", &temp);
    if(!strcmp(sig, "LOGIN"))
    {
      add_info_to_client(temp, sockfd);
    }
    else if(!strcmp(sig, "SEND"))
    {
      send_to_client(sockfd, temp);
    }
    else if(!strcmp(sig, "REFRESH"))
    {
      server_send_list(sockfd);
    }
    else if(!strcmp(sig, "BROADCAST"))
    {
      printf("relaying:\n");
      send_to_all(sockfd, temp);
    }
  }
  if(ret == 0)
  {
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
  }
  //Nothing found
  return NULL;
}
void print_client(struct client_info *print)
{
  printf("%d %s %s\n", print->port, print->ip_addr, print->hostname);
}
void server_send_list(int sockfd)
{
  int ret;
  char buf[256];
  memset(buf, 0, 256);

  struct client_info* tmp;
  char cmd[] = "REFRESH";
  send(sockfd, cmd, strlen(cmd), 0);
  usleep(50*100);
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    printf("Sending list\n");
    tmp = list_entry(iter, struct client_info, elem);
    if(!tmp) continue;
    sprintf(buf, "LIST %d %s %s", tmp->port, tmp->hostname, tmp->ip_addr);
    buf[strlen(buf)+1]= '\0';
    printf("%s\n", buf);
    ret = send(sockfd, buf, strlen(buf), 0);
    usleep(150*1000);
    memset(buf, 0, 256);
  }
}
struct client_info* find_client_by_ip_port(char *ip, int port)
{
  struct client_info* find_client;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_info, elem);
    //printf("Loop 1 %d %d\n", find_client->sockfd, fd);
    if(!strcmp(find_client->ip_addr,ip) && (port == find_client->port))
      return find_client;
  }
  return NULL;
}
struct client_info* find_client_by_ip(char *ip)
{
  struct client_info* find_client;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_info, elem);
    if(!strcmp(find_client->ip_addr,ip))
      return find_client;
  }
  return NULL;
}
void send_to_client(int sockfd, char *msg)
{
  struct client_info *sender = find_client_by_fd(sockfd);
  sender->sent_msg++;
  char relayed_message[256];
  char hostname[64];

  //Find destination
  char *tmp;
  char *ip_addr = strtok_r(msg, " ", &tmp);
  struct client_info *dest = find_client_by_ip(ip_addr);

  printf("Send to IP: %s\n", ip_addr);
  get_host_name(sockfd, hostname);
  if(dest!=NULL)
  {
    printf("Sending:");
    sprintf(relayed_message, "MSG msg from:%s\n[msg]:%s\n", hostname, tmp);
    printf("Sending %s", relayed_message);
    send(dest->sockfd, relayed_message, strlen(relayed_message), 0);
  }
}
void send_to_all(int sockfd, char *message)
{
  struct client_info *sender = find_client_by_fd(sockfd);
  sender->sent_msg++;

  char hostname[64];
  char relayed_message[256];
  get_host_name(sockfd, hostname);
  sprintf(relayed_message, "MSG msg from:%s\n[msg]:%s\n", hostname, message);
  struct client_info* find_client;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_info, elem);
    if(find_client->sockfd!=sockfd)
    {
      send(find_client->sockfd, relayed_message, strlen(relayed_message), 0);
    }
  }
}
