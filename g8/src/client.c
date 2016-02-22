#include "client.h"

struct list connected_list;
struct list blocked_list;
void client_fill_list();
void bind_client(int);

bool verify_ip(char *ip);

void print_clients(struct client_logged *print);
struct client_logged* find_client_ip_port(char *ip, int port);
void clear_list();

int client_connect(char *host, char *port)
{
  struct addrinfo *servinfo; // will point to the results
  //Create a socket
  int sockfd = create_socket(&servinfo, host, port);
  bind_client(sockfd);
  connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  get_host_name(sockfd, NULL);
  freeaddrinfo(servinfo);
  list_init(&connected_list);
  list_init(&blocked_list);
  return sockfd;
}
void client_identify(int fd)
{
  char command[132];
  char host_name[128];
  gethostname(host_name, 128);
  sprintf(command, "LOGIN %d %s", get_listening_port(), host_name);
  printf("%s\n", command);
  client_send(fd, command);
}
int client_send(int sockfd, char *buf)
{
  return send(sockfd, buf, strlen(buf), 0);
}
int client_send_msg(int sockfd, char *buf)
{
  int ret;
  char msg[MAX_LENGTH];
  //strcpy(msg, buf);
  char *tmp;
  char *ip = strtok_r(buf, " ", &tmp);
  if(!verify_ip(ip))
    return 0;
  sprintf(msg, "SEND %s %s",ip, tmp);
  ret = send(sockfd, msg, strlen(msg), 0);
  return ret;
}
void client_close(int sockfd)
{
  close(sockfd);
}
void client_receive(int sockfd)
{
  char msg[MAX_LENGTH];//= "Server!!\0";
  memset(msg, 0, MAX_LENGTH);
  int ret, i=0;
  ret = recv(sockfd, msg, MAX_LENGTH, 0);
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
      print_success(1, "RECEIVED");
      LOG("%s", temp);
      LOG("[%s:END]\n", "RECEIVED");
    }
    else if(!strcmp(command, "REFRESH"))
    {
      clear_list();
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
  //TODO SEGFAULTS!!!!
  int set_port = strtol(argv[0], &end, 10);
  struct client_logged *add_client = malloc(sizeof(struct client_logged));
  add_client->port = set_port;
  memset(add_client->hostname, 0, sizeof(add_client->hostname));
  strcpy(add_client->hostname, argv[1]);
  memset(add_client->ip_addr, 0, sizeof(add_client->ip_addr));
  strcpy(add_client->ip_addr, argv[2]);
  print_clients(add_client);
  list_insert_ordered(&connected_list, &add_client->elem, 
      (list_less_func *)&sort_port_client, NULL);
  printf("Inserted\n");
}
void print_client_list()
{
  struct client_logged* id;
  int list_id=1;
  for(struct list_elem *iter = list_begin(&connected_list); iter!=list_end(&connected_list);
      iter = list_next(iter))
  {
    id = list_entry(iter, struct client_logged, elem);
    LOG("%-5d%-35s%-20s%-8d\n", list_id++, id->hostname, id->ip_addr, id->port);
  }
  //Nothing found
}
void bind_client(int sockfd)
{
  struct sockaddr_in in;
  bzero(&in, sizeof(in));
  char *end;
  int hport = htons(get_listening_port());
  in.sin_family = AF_INET;
  in.sin_addr.s_addr = htonl(INADDR_ANY);
  in.sin_port = hport;
  bind(sockfd, (struct sockaddr *) &in, sizeof(in));
}
bool verify_ip(char *ip)
{
  if(!validate_ip(ip))
    return false;
  struct client_logged* find_client;
  for(struct list_elem *iter = list_begin(&connected_list); iter!=list_end(&connected_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_logged, elem);
    if(!strcmp(find_client->ip_addr,ip))
      return true;
  }
  return false;
}
void print_clients(struct client_logged *print)
{
  printf("%d %s %s\n", print->port, print->ip_addr, print->hostname);
}
struct client_logged* find_client_ip_port(char *ip, int port)
{
  struct client_logged* find_client;
  for(struct list_elem *iter = list_begin(&connected_list); iter!=list_end(&connected_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct client_logged, elem);
    //printf("Loop 1 %d %d\n", find_client->sockfd, fd);
    if(!strcmp(find_client->ip_addr,ip) && (port == find_client->port))
      return find_client;
  }
  return NULL;
}

struct ip_info* is_client_blocked(char *ip)
{
  struct ip_info *find_client;
  for(struct list_elem *iter = list_begin(&blocked_list); iter!=list_end(&blocked_list);
      iter = list_next(iter))
  {
    find_client = list_entry(iter, struct ip_info, elem);
    if(!strcmp(find_client->ip_addr,ip))
      return find_client;
  }
  return NULL;
}

void clear_list()
{
  while(!list_empty(&connected_list))
  {
    struct list_elem *e = list_pop_front(&connected_list);
    struct client_logged *free_node = list_entry(e, struct client_logged, elem);
    free(free_node);
  }
}
void add_to_block_list(char *ip)
{
  struct ip_info *blocked_ip = malloc(sizeof(struct ip_info));
  strcpy(blocked_ip->ip_addr, ip);
  list_push_back(&blocked_list, &blocked_ip->elem);
}
void remove_from_block_list(char *ip)
{
  struct list_elem *iter = list_begin(&blocked_list);
  while(iter!=list_end(&blocked_list))
  {
    struct ip_info *ip_check = list_entry(iter, struct ip_info, elem);
    if(!strcmp(ip_check->ip_addr, ip))
    {
      iter = list_remove(iter);
      free(ip_check);
    }
    else 
      iter = list_next(iter);
  }
}
