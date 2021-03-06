#include <pthread.h>
#include "utils.h"

bool gui, finsish = false;
double mass, t, Gmm, angle;
Body *bodies, *new_bodies;
int num_thread, iters, num_body;

int queuing_jobs = 0, num_done = 0;
pthread_mutex_t queuing;
pthread_cond_t processing, iter_fin;
nanoseconds total_time;

void* worker(void* param)
{
    while (true) {
        pthread_mutex_lock(&queuing);
        while (!finsish && queuing_jobs <= 0)
            pthread_cond_wait(&processing, &queuing);
        int i = --queuing_jobs;
        pthread_mutex_unlock(&queuing);
        if (finsish) break;
        move_nth_body(i);
        pthread_mutex_lock(&queuing);
        num_done++;
        if (num_done >= num_body) pthread_cond_signal(&iter_fin);
        pthread_mutex_unlock(&queuing);
    }
}

int main(int argc, char const **argv)
{
    init_env(argc, argv);
    high_resolution_clock::time_point s;

    pthread_t workers[num_thread];
    pthread_mutex_init(&queuing, NULL);
    pthread_cond_init(&iter_fin, NULL);
    pthread_cond_init(&processing, NULL);

    for (int i = 0; i < num_thread; ++i)
        pthread_create(&workers[i], NULL, worker, NULL);

    Gmm = G * mass * mass;
    for (int i = 0; i < iters; ++i) {
        if (gui) draw_points(0);
        s = high_resolution_clock::now();
        pthread_mutex_lock(&queuing);
        queuing_jobs = num_body, num_done = 0;
        pthread_cond_broadcast(&processing);
        pthread_cond_wait(&iter_fin, &queuing);
        pthread_mutex_unlock(&queuing);
        Body* t = new_bodies; new_bodies = bodies; bodies = t;
        total_time += timeit(s);
    }
    finsish = true;
    pthread_mutex_lock(&queuing);
    pthread_cond_broadcast(&processing);
    pthread_mutex_unlock(&queuing);
    for (int j = 0; j < num_thread; ++j) pthread_join(workers[j], NULL);
    INFO("Run in " << total_time.count() / 1000 << " us");
    return 0;
}
