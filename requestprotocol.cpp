#include "requestprotocol.h"
#include "utilities.h"

RequestProtocol::RequestProtocol(int connfd)
{
    _connfd = connfd;
    _content_length = 0;
    _request_state = 0;
    init();
}

string RequestProtocol::method() const
{
    return _method;
}

string RequestProtocol::url() const
{
    return _url;
}

string RequestProtocol::query_string() const
{
    return _query_string;
}

int RequestProtocol::content_length() const
{
    return _content_length;
}

string RequestProtocol::body() const
{
    return _body;
}

void RequestProtocol::init()
{
    string msg;
    Utilities::readline(_connfd, msg);
    string::size_type cur_pos = 0;
    string::size_type exp_pos = msg.find(' ', cur_pos);
    _method = msg.substr(cur_pos, exp_pos - cur_pos);
    cur_pos = exp_pos + 1;
    exp_pos = msg.find(' ', cur_pos);
    _url = msg.substr(cur_pos, exp_pos - cur_pos);
    if (_method == "GET") {
        exp_pos = _url.find('?');
        if (exp_pos != string::npos) {
            _query_string = _url.substr(exp_pos + 1);
            _url = _url.substr(0, exp_pos);
        }
    }
    string key;
    while (true) {
        Utilities::readline(_connfd, msg);
        if (msg == "")
            break;
        exp_pos = msg.find(':');
        key = msg.substr(0, exp_pos);
        if (key == "Content-Length") {
            string value = msg.substr(exp_pos + 2);
            _content_length = atoi(value.c_str());
        }
        msg.clear();
    }

    if (_method == "POST")
        Utilities::readn(_connfd, _body, _content_length);
}

void RequestProtocol::copy_string(string &des, const string &src, char c, int &cur_pos)
{
    int exp_pos = src.find(c, cur_pos);
    des = src.substr(cur_pos, exp_pos - cur_pos);
    cur_pos = exp_pos + 1;
}

