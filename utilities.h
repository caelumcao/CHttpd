#ifndef UTILITIES_H
#define UTILITIES_H
#include <string>
using std::string;

class Utilities
{
public:
    Utilities();
    static int readline(int connfd, string &msg);
    static int readn(int connfd, string &msg, int n);

private:

};

#endif // UTILITIES_H
