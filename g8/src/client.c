#include "client.h"
int client_connect(char *host, char *port)
{
  struct addrinfo *servinfo; // will point to the results
  //Create a socket
  int sockfd = create_socket(&servinfo, host, port);
  connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  get_host_name(sockfd, NULL);
  freeaddrinfo(servinfo);
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
