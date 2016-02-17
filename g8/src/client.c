#include "client.h"
int client_connect(char *host, char *port)
{
  struct addrinfo *servinfo; // will point to the results
  printf("Starting client\n");

  //Create a socket
  printf("Connect to %s[%s]\n", host, port);
  int sockfd = create_socket(&servinfo, host, port);
  connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  get_host_name(sockfd);
  freeaddrinfo(servinfo);
  //client_send(sockfd);
  return sockfd;
}
void client_identify(int fd)
{
  char port[6];
  sprintf(port, "%d", get_listening_port());
  printf("%s\n", port);
  client_send(fd, port);
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
