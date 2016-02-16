#include "client.h"
int client_connect(char *host, char *port)
{
  struct addrinfo *servinfo; // will point to the results
  printf("Starting client\n");

  //Create a socket
  printf("Connect to %s[%s]\n", host, port);
  int sockfd = create_socket(&servinfo, host, port);
  printf("fd? %d\n", sockfd);
  connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  get_host_name(sockfd);
  freeaddrinfo(servinfo);
  //client_send(sockfd);
  return sockfd;
}
void client_send(int sockfd, char *buf)
{
  int ret, i=0;
  //sprintf(msg, "Server %d\n", i);
  //printf("%s\n", msg);
  ret = send(sockfd, buf, strlen(buf), 0);
  i++;
}
void client_close(int sockfd)
{
  close(sockfd);
}
