/**
 * @ubitname_assignment1
 * @author  Girish Gokul <g8@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include "../include/global.h"
#include "../include/logger.h"

#include "server.h"
#include "client.h"

int getaddrinfo(const char *node, // e.g. "www.example.com" or IP
    const char *service, // e.g. "http" or port number
    const struct addrinfo *hints,
    struct addrinfo **res);
//int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void fill_addrinfo(struct addrinfo *info);
int parse_shell();
void add_fd(int newfd);
int active_sockets=1; 

int main(int argc, char **argv)
{
  int status;
  char ip_addr[INET_ADDRSTRLEN];
  char *service;
  int server_sock = -1;
  bool is_server = false;
  int exit_flag = 0;

  fd_set temp;

	/*Init. Logger*/
	//cse4589_init_log(argv[2]);

  int sock;
  if(argc<3)
  {
    printf("No args\n");
    return 1;
  }

  FD_ZERO(&wait_fd);
  FD_ZERO(&temp);
  FD_SET(0, &wait_fd);
  tv.tv_sec = 15;
  tv.tv_usec = 0;

  if(!strcmp(argv[1], "s"))
  {
    server_sock = server_start(argv[2]);
    if(server_sock <=0)return 0;
    sock = server_sock;
    is_server = true;
    add_fd(sock);
  }
  else if(strcmp(argv[1], "c"))
  {
    printf("Usage ./sock [s/c]\n");
    return 0;
  }

//  printf("socket %d %d for %s\n", active_sockets, sock, argv[1]);
  int i =2;
  while(i && ! exit_flag)
  {
    temp = wait_fd;
    printf("Active sockets = %d\n", active_sockets);
    int ret = select(active_sockets, &temp, NULL, NULL, NULL);
    if(ret)
    {
      if(FD_ISSET(STDIN, &temp))
      {
        printf("stdin\n");
        exit_flag = parse_shell();
      }
      if(is_server && FD_ISSET(server_sock, &temp))
      {
        // server socket is active, check for new connections
        int new_socket = server_accept(server_sock);
        add_fd(new_socket);
      }
      for(int fd = 3; fd<=active_sockets; fd++)
      {
        if(FD_ISSET(fd, &temp))
        {
          printf("%d, socket\n", fd);
          server_receive(fd);
        }
      }
    }
  }

  server_kill(sock);
	fclose(fopen(LOGFILE, "w"));
  return 0;
}
int parse_shell()
{
  char shell_input[255];
  char *temp;
  static bool is_client_connected = false;
  fgets(shell_input, 255, stdin);
  if(strlen(shell_input) <= 1)
    return 0;
  shell_input[strlen(shell_input)-1] = '\0';
  char *arg;
  int argc = 0;
  char argv[4][255];
  char *command = strtok_r(shell_input, " ", &temp);
  for(arg = strtok_r(NULL, " ", &temp); arg; arg = strtok_r(NULL, " ", &temp))
  {
    strcpy(argv[argc], arg);
    argc++;
  }
  if(!strcmp("LOGIN", command))
  {
    if(is_client_connected || argc!=2)
    {
      printf("[%s:ERROR]\n", command);
    }
    else
    {
      printf("Connecting to server %s %s\n", argv[0], argv[1]);
      int newfd = client_connect(argv[0], argv[1]);
      printf(" %d\n", newfd);
      if(newfd<=1)
      {
        printf("[%s:ERROR]\n", command);
        return 0;
      }
      is_client_connected = true;
      add_fd(newfd);
    }
  }
  if(!strcmp("EXIT", command))
  {
    return 1;
  }
  printf("[%s:END]\n", command);
  return 0;
}
void add_fd(int newfd)
{
  FD_SET(newfd, &wait_fd);
  if(newfd >=active_sockets) active_sockets = newfd+1;
}
