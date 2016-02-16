#include "sock_helper.h"

int create_socket(struct addrinfo **servinfo_, char* host, char *port)
{
  struct addrinfo *servinfo = *servinfo_;
  struct addrinfo hints;
  fill_addrinfo(&hints);
  getaddrinfo(host, port, &hints, &servinfo);
  int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  *servinfo_ = servinfo;
  return sockfd;
}

void get_host_name(int sockfd)
{
  struct sockaddr_storage connected_server;
  socklen_t len = sizeof(connected_server);
  struct sockaddr_in *in;
  char ipstr[INET_ADDRSTRLEN];
  char host[1024];
  char service[1024];

  getpeername(sockfd, (struct sockaddr*)&connected_server, &len);

  in = (struct sockaddr_in *)&connected_server;
  /*
   * TODO Add hostname code
  struct sockaddr *strt = (struct sockaddr *) &connected_server;
  getnameinfo(&strt, sizeof strt, host, 1024, service, 1024, 0);
  */
  inet_ntop(AF_INET, &in->sin_addr, ipstr, sizeof(ipstr));
  printf("%s:%s:[%d]\n", ipstr, host, ntohs(in->sin_port));
  printf("%s\n", service);
}
void fill_addrinfo(struct addrinfo *info)
{
  if(info==NULL)
  {
    printf("addrinfo structure NULL!\n");
    exit(1);
  }
  memset(info, 0, sizeof(struct addrinfo));
  info->ai_family = AF_INET;
  info->ai_socktype = SOCK_STREAM;
  //info->ai_flags = AI_PASSIVE; //Fill with my IP
}
