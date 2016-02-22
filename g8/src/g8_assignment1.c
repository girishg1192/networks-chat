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
//#include "../include/logger.h"

#include "server.h"
#include "client.h"

int getaddrinfo(const char *node, // e.g. "www.example.com" or IP
    const char *service, // e.g. "http" or port number
    const struct addrinfo *hints,
    struct addrinfo **res);
//int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void fill_addrinfo(struct addrinfo *info);
int parse_shell();
void get_ip();
void add_fd(int newfd);
int active_sockets=1;
bool is_server = false;

int main(int argc, char **argv)
{
  int status;
  char ip_addr[INET_ADDRSTRLEN];
  char *service;
  int server_sock = -1;
  int exit_flag = 0;

  fd_set temp;

  /*Init. Logger*/
#ifndef MY_PC
  cse4589_init_log(argv[2]);
#endif

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
    is_server = true;
  }
  else if(strcmp(argv[1], "c"))
  {
    printf("Usage ./sock [s/c]\n");
    return 0;
  }
  set_listening_port(argv[2]);
  server_sock = server_start(argv[2]);
  if(server_sock <=0)return 0;
  sock = server_sock;
  add_fd(sock);

  int i =2;
  while(!exit_flag)
  {
    temp = wait_fd;
    int ret = select(active_sockets, &temp, NULL, NULL, NULL);
    if(ret)
    {
      if(FD_ISSET(STDIN, &temp))
      {
        exit_flag = parse_shell();
      }
      else if(FD_ISSET(server_sock, &temp) && is_server)
      {
        // server socket is active, check for new connections
        int new_socket = server_accept(server_sock);
        add_fd(new_socket);
      }
      else
      {
        for(int fd = 3; fd<=active_sockets; fd++)
        {
          if(FD_ISSET(fd, &temp))
          {
            if(is_server)
              server_receive(fd);
            else
              client_receive(fd);
          }
        }
      } //End of else
    } //end of select handling
  } //end loop
  clear_fd(sock);
  close(sock);
#ifndef MY_PC
  fclose(fopen(LOGFILE, "w"));
#endif
  return 0;
}
int parse_shell()
{
  char shell_input[MAX_LENGTH];
  char message[MAX_LENGTH];
  static bool is_client_connected = false;
  int ret = 0;
  static int server_sock;

  /*
   * Get the input
   */
  fgets(shell_input, MAX_LENGTH, stdin);
  if(strlen(shell_input) <= 1)
    return 0;
  shell_input[strlen(shell_input)-1] = '\0';
  strcpy(message, shell_input);

  /*
   * Tokenize the strings
   */
  char *arg;
  int argc = 0;
  char argv[4][256];
  char *temp;
  char *command = strtok_r(shell_input, " ", &temp);
  //TODO reorder all commands!!
  // ----------------Commands common for server/client ------------
  if(!strcmp("AUTHOR", command))
  {
    print_success(1, command);
    LOG("I, g8, have read and understood the course academic integrity policy.\n");
  }
  else if(!strcmp("PORT", command))
  {
    print_success(1, command);
    LOG("PORT:%d\n", get_listening_port());
  }
  else if(!strcmp("IP", command))
  {
    print_success(1, command);
    get_ip();
  }
  else if(!strcmp("LIST", command))
  {
    print_success(1, command);
    if(is_server)
      print_connected_client_list();
    else
      print_client_list();
  }
  else if(!strcmp("EXIT", command))
  {
    return 1;
  }
  // -------------------Common commands ---------------------------
  else if(!is_server)
  {
    char arg_copy[MAX_LENGTH];
    if(temp!=NULL)
    {
      strcpy(arg_copy, temp);
      for(arg = strtok_r(NULL, " ", &temp); arg; arg = strtok_r(NULL, " ", &temp))
      {
        strcpy(argv[argc], arg);
        argc++;
      }
    }
    if(!strcmp("LOGIN", command))
    {
      if(!is_server && !(is_client_connected || argc!=2) && validate_ip(argv[0]))
      {
        printf("Connecting to server %s %s\n", argv[0], argv[1]);
        int newfd = client_connect(argv[0], argv[1]);
        if(newfd<=1)
        {
          LOG("[%s:ERROR]\n", command);
          return 0;
        }
        //TODO client connects to unknown port?
        is_client_connected = true;
        add_fd(newfd);
        client_identify(newfd);
        server_sock = newfd;
        print_success(1, command);
      }
      else
        print_success(0, command);
    }
    else if(is_client_connected)
    {
      if(!strcmp("SEND", command))
      {
        if(!is_client_connected)
          print_success(0,command);
        else
          print_success(client_send_msg(server_sock, arg_copy), command);
      }
      else if(!strcmp("LOGOUT", command))
      {
        if(is_client_connected)
        {
          //close(server_sock);
          //clear_fd(server_sock);
          client_send(server_sock, command);
          is_client_connected  = false;
          //server_sock = -1;
          print_success(1, command);
        }
        else
          print_success(0, command);
      }
      else if(!strcmp("REFRESH", command))
      {
        client_send(server_sock, command);
        print_success(1, command);
      }
      else if(!strcmp("BROADCAST", command))
      {
        if(argc)
        {
          client_send(server_sock, message);
          print_success(1, command);
        }
        else print_success(0, command);
      }
      else if(!strcmp("BLOCK", command))
      {
        printf("asda\n");
        if(argc == 1 && verify_ip(argv[0])
            && !is_client_blocked(argv[0]))
        {
          //TODO CHECK IP in list!! done!
          print_success(1, command);
          add_to_block_list(argv[0]);
          client_send(server_sock, message);
        }
        else
        {
          print_success(0, command);
        }
      }
      else if(!strcmp("UNBLOCK", command))
      {
        if(argc == 1 && verify_ip(argv[0])
            && is_client_blocked(argv[0]))
        {
          print_success(1, command);
          remove_from_block_list(argv[0]);
          client_send(server_sock, message);
        }
        else
          print_success(0, command);
      }
      else
        print_success(0, command);
    }
    else
      print_success(0, command);
  }
  else
  {
    if(!strcmp("STATISTICS", command))
    {
      print_success(1, command);
      print_stats();
    }
    else if(!strcmp("BLOCKED", command))
    {
      if(temp == NULL)
        print_success(0, command);
      else
      {
        char *ip = strtok_r(NULL, " ", &temp);
        if(temp==NULL && validate_ip(ip))
        {
          print_blocked_clients(ip);
        }
        print_success(1, command);
      }
    }
    else
      print_success(0, command);
  }

  LOG("[%s:END]\n", command);
  return 0;
}
void get_ip()
{
  struct sockaddr_storage connected_server;
  struct addrinfo *servinfo;
  socklen_t len = sizeof(connected_server);
  char ipstr[INET_ADDRSTRLEN];

  int ip_sock = create_socket(&servinfo, "www.google.com", "80");
  connect(ip_sock, servinfo->ai_addr, servinfo->ai_addrlen);
  freeaddrinfo(servinfo);

  getsockname(ip_sock, (struct sockaddr*)&connected_server, &len);
  struct sockaddr_in *in = (struct sockaddr_in *)&connected_server;
  inet_ntop(AF_INET, &in->sin_addr, ipstr, sizeof(ipstr));

  close(ip_sock);
  LOG("IP:%s\n", ipstr);
}
