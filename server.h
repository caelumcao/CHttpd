#ifndef SERVER_H
#define SERVER_H
#include "socketqueue.h"
#include <vector>
#include <memory>
#include <string>
using std::vector;
using std::shared_ptr;
using std::string;

class Server
{
public:
    Server();
    void run();
    void stop();

private:
    static const string SERVER_STRING;

    static Server *_server;
    vector<pthread_t> _tids;
    vector<int> _tindexs;
    shared_ptr<SocketQueue> _socket_queue;

    int startup();
    void create_thread();
    static void *accept_request(void *arg);
    void request_handle(int connfd, int tindex);
    int write_msg(int connfd, const string &msg);
    void not_found(int connfd);
    void serve_file(int connfd, const string &path);
    void execute_cgi(int connfd, const string &path, const string &method, const string &query_string, const string &body);
    void cannot_execute(int connfd);

    string &header();
    void error_die(const char *str);
};

#endif // SERVER_H
