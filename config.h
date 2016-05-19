#ifndef CONFIG_H
#define CONFIG_H


class Config
{
public:
    static const int PORT = 8080;
    static const int MAXLINE = 4096;
    static const int DEF_THREAD_NUM = 5;
private:
    Config();
};

#endif // CONFIG_H
