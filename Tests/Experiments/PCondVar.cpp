// Works only on POSIX systems and Windows with pthreads.

#include <pthread.h>
#include <stdio.h>
#include <Atomic.hpp>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex;
hsd::atomic_i32 counter;

void* thread_func(void* arg)
{
    pthread_mutex_lock(&mutex);
    
    while (counter.load() < 10)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    
    printf("Thread %d\n", *reinterpret_cast<int*>(arg));
    counter.store(0);
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

int main()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_t thread1, thread2;
    int arg1 = 1, arg2 = 2;
    counter.store(0);

    pthread_create(&thread1, nullptr, thread_func, reinterpret_cast<void*>(&arg1));
    pthread_create(&thread2, nullptr, thread_func, reinterpret_cast<void*>(&arg2));

    pthread_mutex_lock(&mutex);
    counter.fetch_add(10);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(thread1, nullptr);

    pthread_mutex_lock(&mutex);
    counter.fetch_add(10);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(thread2, nullptr);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}