#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main(){
  sockaddr_in server;
  int sock, epfd;
  char buf[32];
  int nfds, n;
  int val;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  server.sin_family = AF_INET;
  server.sin_port = htons(9981);
  inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);

  n = connect(sock, (sockaddr*)&server, sizeof(server));
  if(n != 0)
  {
    printf("conn error\n");
  }
  
  snprintf(buf, sizeof(buf), "GETaa");
  n = write(sock, buf, (int)strlen(buf));

  close(sock);
  
}
