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

void queue_message(struct client_info *dest, char *message);
void send_queued_message(struct client_info *connection);

void block_client(struct client_info *, char *ip);
void unblock_client(struct client_info *blocker, char *ip);
bool check_if_blocked(struct client_info *sender, struct client_info * dest);


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
  connection->is_connected = true;
  list_init(&(connection->blocked_list));
  list_init(&(connection->inbox));
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
    list_insert_ordered(&client_list, &connection->elem, (list_less_func *)&sort_port, NULL);
  }
  else
  {
    connection->sockfd = fd;
    connection->is_connected = true;
  }
  print_client(connection);
  server_send_list(fd);
  send_queued_message(connection);
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
      print_success(1, sig);
      send_to_client(sockfd, temp);
      LOG("[%s:END]\n", sig);
    }
    else if(!strcmp(sig, "REFRESH"))
    {
      server_send_list(sockfd);
    }
    else if(!strcmp(sig, "BROADCAST"))
    {
      print_success(1, sig);
      send_to_all(sockfd, temp);
      LOG("[%s:END]\n", sig);
    }
    else if(!strcmp(sig, "BLOCK"))
    {
      struct client_info *block = find_client_by_fd(sockfd);
      block_client(block, temp);
    }
    else if(!strcmp(sig, "UNBLOCK"))
    {
      struct client_info *unblock = find_client_by_fd(sockfd);
      unblock_client(unblock, temp);
    }
    else if(!strcmp(sig, "LOGOUT"))
    {
      struct client_info *dead_client= find_client_by_fd(sockfd);
      dead_client->is_connected = false;
    }
  }
  if(ret == 0)
  {
    server_kill(sockfd);
  }
}
void server_kill(int sockfd)
{
  struct client_info *dead_client = find_client_by_fd(sockfd);
  list_remove(&dead_client->elem);
  free(dead_client);
  clear_fd(sockfd);
  close(sockfd);
  printf("Killed client\n");
}
struct client_info* find_client_by_fd(int fd)
{
  struct client_info* find_client;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_info, elem);
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
  if(!check_if_blocked(sender, dest))
  {
    get_host_name(sockfd, hostname);
    LOG("msg from:%s, to:%s\n[msg]:%s\n", sender->ip_addr, ip_addr, msg);
    if(dest!=NULL)
    {
      sprintf(relayed_message, "MSG msg from:%s\n[msg]:%s\n", hostname, tmp);
      if(dest->is_connected)
      {
        dest->recv_msg++;
        send(dest->sockfd, relayed_message, strlen(relayed_message), 0);
      }
      else
        queue_message(dest, relayed_message);
    }
  }
  else
    printf("blocked!\n");
}
void send_to_all(int sockfd, char *message)
{
  struct client_info *sender = find_client_by_fd(sockfd);
  sender->sent_msg++;

  char hostname[64];
  char relayed_message[256];
  get_host_name(sockfd, hostname);
  sprintf(relayed_message, "MSG msg from:%s\n[msg]:%s\n", hostname, message);
  LOG("msg from:%s, to:%s\n[msg]:%s\n", sender->ip_addr, "255.255.255.255", message);
  struct client_info* find_client;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_info, elem);
    if(find_client->sockfd!=sockfd
        && !(check_if_blocked(sender, find_client)))
    {
      if(find_client->is_connected)
      {
        find_client->recv_msg++;
        send(find_client->sockfd, relayed_message, strlen(relayed_message), 0);
      }
      else
        queue_message(find_client, relayed_message);
    }
  }
}
void queue_message(struct client_info *dest, char *message)
{
  struct queued_msg *a= malloc(sizeof(struct queued_msg));
  strcpy(a->message, message);
  list_push_back(&dest->inbox, &a->elem);
  printf("Queued %s", a->message);
}

void send_queued_message(struct client_info *connection)
{
  while(!list_empty(&connection->inbox))
  {
    struct list_elem *e = list_pop_front(&connection->inbox);
    struct queued_msg *msg = list_entry(e, struct queued_msg, elem);
    printf("Queued message %s", msg->message);
    connection->recv_msg++;
    send(connection->sockfd, msg, strlen(msg->message), 0);
    free(msg);
  }
}


void block_client(struct client_info *blocker, char *ip)
{
  struct ip_info *blocked_ip = malloc(sizeof(struct ip_info));
  strcpy(blocked_ip->ip_addr, ip);
  struct client_info *blocked_client = find_client_by_ip(ip);
  blocked_ip->port = blocked_client->port;
  printf("%s %d BLOCKED %s %d\n", blocker->ip_addr, blocker->port, 
      blocked_client->ip_addr, blocked_client->port);
  list_insert_ordered(&(blocker->blocked_list),  &blocked_ip->elem,
      (list_less_func *)&sort_block, NULL);
}
void unblock_client(struct client_info *blocker, char *ip)
{
  struct list_elem *iter = list_begin(&(blocker->blocked_list));
  while(iter!=list_end(&(blocker->blocked_list)))
  {
    struct ip_info *ip_check = list_entry(iter, struct ip_info, elem);
    if(!strcmp(ip_check->ip_addr, ip))
    {
      printf("found blocked client\n");
      iter = list_remove(iter);
      free(ip_check);
    }
    else 
      iter = list_next(iter);
  }
}
//Check if sender is in the destinations blocked list
bool check_if_blocked(struct client_info *sender, struct client_info * dest)
{
  struct list_elem *iter = list_begin(&(dest->blocked_list));
  while(iter!=list_end(&(dest->blocked_list)))
  {
    struct ip_info *ip_check = list_entry(iter, struct ip_info, elem);
    if(!strcmp(ip_check->ip_addr, sender->ip_addr) && 
        (ip_check->port == sender->port))
    {
      printf("Sender blocked\n");
      return true;
    }
    iter = list_next(iter);
  }
  return false;
}
void print_stats()
{
  struct client_info* id;
  int list_id=1;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    id = list_entry(iter, struct client_info, elem);
    LOG("%-5d%-35s%-8d%-8d%-8s\n", list_id++, id->hostname, 
        id->sent_msg, id->recv_msg, (id->is_connected?"online":"offline"));
  }
  //Nothing found
}
void print_connected_client_list()
{
  struct client_info* id;
  int list_id=1;
  for(struct list_elem *iter = list_begin(&client_list); iter!=list_end(&client_list);
      iter = list_next(iter))
  {
    id = list_entry(iter, struct client_info, elem);
    LOG("%-5d%-35s%-20s%-8d\n", list_id++, id->hostname, id->ip_addr, id->port);
  }
  //Nothing found
}
void print_blocked_clients(char *ip)
{
  struct client_info *dest = find_client_by_ip(ip);
  struct list_elem *iter = list_begin(&(dest->blocked_list));
  int list_id = 1;
  while(iter!=list_end(&(dest->blocked_list)))
  {
    struct ip_info *ip_check = list_entry(iter, struct ip_info, elem);
    struct client_info *print = find_client_by_ip_port(ip_check->ip_addr, ip_check->port);
    if(print!=NULL)
      LOG("%-5d%-35s%-20s%-8d\n", list_id++, print->hostname, print->ip_addr, print->port);
    iter = list_next(iter);
  }
}
