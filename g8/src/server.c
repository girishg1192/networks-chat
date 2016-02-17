#include "server.h"
#define MAX_NUMBER 8

struct list client_list;

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
  printf("listen %d\n", err);

  freeaddrinfo(servinfo);
  printf("%d socket<--\n", sockfd);
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
  get_host_name(newfd, host_name);

  char msg[255];
  int ret = recv(sockfd, msg, 256, 0);
  printf("Client Auth\n");
  printf("%s, %s\n",host_name, msg); 

  return newfd;
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
    printf("%s %d\n", msg, ret);
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
