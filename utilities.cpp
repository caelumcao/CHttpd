#include "utilities.h"
#include "config.h"
#include <sys/socket.h>

Utilities::Utilities()
{

}

int Utilities::readline(int connfd, string &msg)
{
    char c = '\0';
    int n;
    while (true) {
        n = recv(connfd, &c, 1, 0);
        if (n > 0) {
            if (c == '\r') {
                n = recv(connfd, &c, 1, MSG_PEEK);
                if (n == 1 && c == '\n')
                    recv(connfd, &c, 1, 0);
                return 1;
            }
            msg.push_back(c);
        } else if (n == 0) {
            return 0;
        } else if (errno != EINTR){
            return -1;
        }
    }
    return 1;
}

int Utilities::readn(int connfd, string &msg, int n)
{
    char buf[Config::MAXLINE];
    size_t nleft = n;
    ssize_t nread;
    while (nleft > 0) {
        if ((nread = recv(connfd, buf, nleft, 0)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0) {
            break;
        }
        msg.append(buf, nread);
        nleft -= nread;
    }
    return (n - nleft);
}

