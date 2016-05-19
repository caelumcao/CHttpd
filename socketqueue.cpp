#include "socketqueue.h"

SocketQueue::SocketQueue()
{
    _queue_mutex = PTHREAD_MUTEX_INITIALIZER;
    _queue_cond = PTHREAD_COND_INITIALIZER;
}

void SocketQueue::enqueue(int connfd)
{
    pthread_mutex_lock(&_queue_mutex);
    _squeue.push(connfd);
    pthread_mutex_unlock(&_queue_mutex);
    pthread_cond_signal(&_queue_cond);
}

int SocketQueue::dequeue()
{
    pthread_mutex_lock(&_queue_mutex);
    while (_squeue.empty())
        pthread_cond_wait(&_queue_cond, &_queue_mutex);
    int connfd = _squeue.front();
    _squeue.pop();
    pthread_mutex_unlock(&_queue_mutex);
    return connfd;
}

