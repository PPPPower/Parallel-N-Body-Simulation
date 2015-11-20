#include "utils.h"

bool gui;
double mass, t, Gmm, angle;
Body *bodies, *new_bodies;
int num_thread, iters, num_body;

nanoseconds total_time;

void move_nth_body(int index)
{
    Body &a = bodies[index], &new_a = new_bodies[index];
    double f_sum_x = 0, f_sum_y = 0;
    for (int i = 0; i < num_body; ++i) {
        if (index == i) continue;
        Body &b = bodies[i];
        double dx = b.x - a.x, dy = b.y - a.y,
               radius_cube_sqrt = CUBE(sqrt(SQUARE(dx) + SQUARE(dy))) + 10e-7;
        f_sum_x +=  Gmm * dx / radius_cube_sqrt;
        f_sum_y +=  Gmm * dy / radius_cube_sqrt;
    }
    new_a.vx = a.vx + f_sum_x * t / mass;
    new_a.vy = a.vy + f_sum_y * t / mass;
    new_a.x  = a.x + new_a.vx * t;
    new_a.y  = a.y + new_a.vy * t;
}

int main(int argc, char const **argv)
{
    init_env(argc, argv);
    high_resolution_clock::time_point s;

    Gmm = G * mass * mass;
    for (int i = 0; i < iters; ++i) {
        if (gui) draw_points(0);
        s = high_resolution_clock::now();
        for (int j = 0; j < num_body; ++j) move_nth_body(j);
        Body* t = new_bodies; new_bodies = bodies; bodies = t;
        total_time += timeit(s);
    }
    INFO("Run in " << total_time.count() / 1000 << " ms");
    return 0;
}
