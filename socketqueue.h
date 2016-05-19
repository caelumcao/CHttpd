#ifndef SOCKETQUEUE_H
#define SOCKETQUEUE_H
#include <pthread.h>
#include <queue>
using std::queue;

class SocketQueue
{
public:
    SocketQueue();
    void enqueue(int sockfd);
    int dequeue();

private:
    pthread_mutex_t _queue_mutex;
    pthread_cond_t _queue_cond;
    queue<int> _squeue;
};

#endif // SOCKETQUEUE_H
