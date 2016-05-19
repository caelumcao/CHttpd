#ifndef REQUESTPROTOCOL_H
#define REQUESTPROTOCOL_H
#include <string>
using std::string;

class RequestProtocol
{
public:
    RequestProtocol(int connfd);

    string method() const;
    string url() const;
    string query_string() const;
    int content_length() const;
    string body() const;

private:
    string _method;
    string _url;
    string _query_string;
    string _body;
    int _content_length;

    int _connfd;
    int _request_state;

    void init();
    void copy_string(string &des, const string &src, char c, int &cur_pos);
};

#endif // REQUESTPROTOCOL_H
