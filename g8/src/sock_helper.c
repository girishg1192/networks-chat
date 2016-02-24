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

void get_host_name(int sockfd, char *ip)
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
  //well, Don't copy to a null argument.
  if(ip!=NULL)
    strcpy(ip, ipstr);
  else
    printf("%s:[%d]\n", ipstr, ntohs(in->sin_port));
}
void fill_addrinfo(struct addrinfo *info)
{
  if(info==NULL)
  {
    exit(1);
  }
  memset(info, 0, sizeof(struct addrinfo));
  info->ai_family = AF_INET;
  info->ai_socktype = SOCK_STREAM;
  //info->ai_flags = AI_PASSIVE; //Fill with my IP
}
void set_listening_port(char *port_)
{
  char *end;
  port = strtol(port_, &end, 10);
}
int get_listening_port()
{
  return port;
}
void print_success(int ret, char *command)
{
  if(ret)
    LOG("[%s:SUCCESS]\n", command);
  else
    LOG("[%s:ERROR]\n", command);
}

/* FDSET operators
 */
void add_fd(int newfd)
{
  FD_SET(newfd, &wait_fd);
  if(newfd >=active_sockets) active_sockets = newfd+1;
}
void clear_fd(int oldfd)
{
  FD_CLR(oldfd, &wait_fd);
}
/* Check IP
 */
bool validate_ip(char *ip)
{
  char ip_copy[INET_ADDRSTRLEN];
  strcpy(ip_copy, ip);

  char *tmp;
  char *end;
  int ip_byte;
  char *token = strtok_r(ip_copy, ".", &tmp);
  int bytes=1;
  while(token)
  {
    if(bytes++>4) return false;
    ip_byte = strtol(token, &end, 10);
    if(!(ip_byte<=255 && ip_byte>=0))
        return false;
    token = strtok_r(NULL, ".", &tmp);
  }
  if(bytes!=5)
    return false;
  return true;
}
bool validate_port(char *port_)
{
  unsigned int port;
  char *end;
  port = strtol(port_, &end, 10);
  if(port>=1024 && port<=65535)
      return true;
  return false;
}
