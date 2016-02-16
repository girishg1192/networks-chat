#include "server.h"
#define MAX_NUMBER 8


int server_start(char *port)
{
  int err;
  struct addrinfo *servinfo; // will point to the results
  printf("Starting server\n");

  //Create a socket
  int sockfd = create_socket(&servinfo, NULL, port);

  //Do server stuff
  err = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  printf("bind %d ",err);
  if(err) return -1;
  err = listen(sockfd, MAX_NUMBER);
  printf("listen %d\n", err);

  freeaddrinfo(servinfo);
  printf("%d socket<--\n", sockfd);
//  server_accept(sockfd);
  return sockfd;
}
int server_accept(int sockfd)
{
  struct sockaddr_storage client_addr;
  socklen_t size_address = sizeof(struct sockaddr_storage);
  int newfd = accept(sockfd, (struct sockaddr *) &client_addr, &size_address);
  get_host_name(newfd);
  /*do
  {
  }while(ret);
  */

  printf("%d newfd?\n", newfd);
  return newfd;
}
void server_receive(int sockfd)
{
  char msg[255];//= "Server!!\0";
  int ret, i=0;
  ret = recv(sockfd, msg, 256, 0);
  if(ret>=0)
  {
    get_host_name(sockfd);
    msg[ret]='\0';
    printf("%s %d\n", msg, ret);
  }
}
void server_kill(int sockfd)
{
  close(sockfd);
}
