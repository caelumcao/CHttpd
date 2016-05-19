#include "server.h"
#include "config.h"
#include "requestprotocol.h"
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

const string Server::SERVER_STRING = "Server: jdbhttpd/0.1.0\r\n";
Server *Server::_server = nullptr;


Server::Server() : _tids(vector<pthread_t>(Config::DEF_THREAD_NUM, 0)), _socket_queue(new SocketQueue)
{
    _server = this;
}

void Server::run()
{
    create_thread();
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int sockfd = startup();
    printf("httpd running on port %d\n", Config::PORT);
    int connfd;
    while (true) {
        connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd == -1)
            error_die("accept");
        _socket_queue->enqueue(connfd);
    }
}

void Server::stop()
{

}

int Server::startup()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
        error_die("socket");
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(Config::PORT);
    int on = -1;
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
        error_die("setsockopt");
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        error_die("bind");
    if (listen(listenfd, 5) < 0)
        error_die("listen");
    return listenfd;
}

void Server::create_thread()
{
    for (int i = 0; i < Config::DEF_THREAD_NUM; ++i)
        _tindexs.push_back(i);
    for (int i = 0; i < Config::DEF_THREAD_NUM; ++i) {
        if (pthread_create(&_tids.at(i), NULL, &accept_request, (void *)&_tindexs.at(i)) != 0)
            fprintf(stderr, "Failed to create thread %d\n", _tindexs.at(i));
    }
}

void *Server::accept_request(void *arg)
{
    int tindex = *(int *)arg;
    printf("thread %d starting\n", tindex);
    int connfd;
    while (true) {
        connfd = _server->_socket_queue->dequeue();
        printf("accepted by thread %d: %d\n", tindex, connfd);
        _server->request_handle(connfd, tindex);
        printf("handled by thread %d\n", tindex);
    }
    return nullptr;
}

void Server::request_handle(int connfd, int tindex)
{
    string msg;
    RequestProtocol request(connfd);
    bool b_cgi = false;
    if (request.method() == "POST" || !request.query_string().empty())
        b_cgi = true;
    string path = "htdocs" + request.url();
    if (path.back() == '/')
        path += "index.html";
    struct stat st;
    if (stat(path.c_str(), &st) == -1) {
        not_found(connfd);
    } else {
        if ((st.st_mode & S_IFMT) == S_IFDIR)
            path += "/index.html";
        if ((st.st_mode & S_IXUSR) ||(st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
            b_cgi = true;
        if (b_cgi)
            execute_cgi(connfd, path, request.method(), request.query_string(), request.body());
        else
            serve_file(connfd, path);
    }
    close(connfd);
}

int Server::write_msg(int connfd, const string &msg)
{
    const char *buf = msg.c_str();
    const char *ptr = buf;
    int nleft = msg.size();
    int nwritten;
    while (nleft > 0) {
        if ((nwritten = write(connfd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return msg.size();
}

void Server::not_found(int connfd)
{
    static string msg("HTTP/1.0 404 NOT FOUND\r\n"
               + SERVER_STRING
               + "Content-Type: text/html\r\n"
               + "\r\n"
               + "<HTML><TITLE>Not Found</TITLE>\r\n"
               + "<BODY><P>The server could not fulfill\r\n"
               + "your request because the resource specified\r\n"
               + "is unavailable or nonexistent.\r\n"
               + "</BODY></HTML>\r\n");
    write_msg(connfd, msg);
}

void Server::serve_file(int connfd, const string &path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        not_found(connfd);
    } else {
        char buf[Config::MAXLINE];
        string msg(header());
        int nread;
        while ((nread = read(fd, buf, Config::MAXLINE)) > 0) {
            buf[nread] = '\0';
            msg += buf;
        }
        write_msg(connfd, msg);
    }
    close(fd);
}

void Server::execute_cgi(int connfd, const string &path, const string &method, const string &query_string, const string &body)
{
    int cgi_output[2], cgi_input[2];
    if (pipe(cgi_output) < 0 || pipe(cgi_input) < 0) {
        cannot_execute(connfd);
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        cannot_execute(connfd);
        return;
    } else if (pid == 0) {
        dup2(cgi_output[1], 1);
        dup2(cgi_input[0], 0);
        close(cgi_output[0]);
        close(cgi_input[1]);
        char meth_env[255];
        sprintf(meth_env, "REQUEST_METHOD=%s", method.c_str());
        putenv(meth_env);
        if (method == "GET") {
            char query_env[255];
            sprintf(query_env, "QUERY_STRING=%s", query_string.c_str());
            putenv(query_env);
        } else {
            char length_env[255];
            sprintf(length_env, "CONTENT_LENGTH=%ld", body.size());
            putenv(length_env);
        }
        execl(path.c_str(), path.c_str(), NULL);
        exit(0);
    } else {
        close(cgi_output[1]);
        close(cgi_input[0]);
        if (method == "POST")
            write(cgi_input[1], body.c_str(), body.size());
        write_msg(connfd, "HTTP/1.0 200 OK\r\n");
        char buf[Config::MAXLINE];
        int n;
        while ((n = read(cgi_output[0], buf, Config::MAXLINE)) > 0)
            write(connfd, buf, n);
        close(cgi_output[0]);
        close(cgi_input[1]);
        int status;
        waitpid(pid, &status, 0);
    }
}

void Server::cannot_execute(int connfd)
{
    static string msg(string() + "HTTP/1.0 500 Internal Server Error\r\n"
                      + "Content-type: text/html\r\n"
                      + "\r\n"
                      + "<P>Error prohibited CGI execution.\r\n");
    write_msg(connfd, msg);
}

string &Server::header()
{
    static string header_string("HTTP/1.0 200 OK\r\n"
                                + SERVER_STRING
                                + "Content-Type: text/html\r\n"
                                + "\r\n");
    return header_string;
}

void Server::error_die(const char *str)
{
    perror(str);
    exit(0);
}
