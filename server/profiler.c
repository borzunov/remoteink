#include "profiler.h"

#include <stdio.h>
#include <time.h>

long long get_time_nsec() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * NSECS_PER_SEC + spec.tv_nsec;
}

#ifdef ENABLE_PROFILER

long long stages_start[STAGES_COUNT],
        stages_min[STAGES_COUNT], stages_max[STAGES_COUNT];
double stages_sum[STAGES_COUNT];
int stages_calls[STAGES_COUNT];

void profiler_start(int stage) {
    if (stage < 0 || stage >= STAGES_COUNT)
        return;
    
    stages_start[stage] = get_time_nsec();
}

void profiler_finish(int stage) {
    if (stage < 0 || stage >= STAGES_COUNT)
        return;

    long long duration = get_time_nsec() - stages_start[stage];
    stages_sum[stage] += duration;
    if (!stages_calls[stage] || duration < stages_min[stage])
        stages_min[stage] = duration;
    if (!stages_calls[stage] || duration > stages_max[stage])
        stages_max[stage] = duration;
    stages_calls[stage]++;
}

void profiler_save(const char *filename) {
    FILE *f = fopen(filename, "w");
    int i;
    for (i = 0; i < STAGES_COUNT; i++) {
        double average = stages_sum[i] / stages_calls[i];
        
        fprintf(f, "Stage %d:\n", i);
        fprintf(f, "    average = %.1lf ms\n", average / NSECS_PER_MSEC);
        fprintf(f, "    total %.0lf ms in %d calls\n",
                stages_sum[i] / NSECS_PER_MSEC, stages_calls[i]);
        fprintf(f, "    min = %lld ms, max = %lld ms\n\n",
                stages_min[i] / NSECS_PER_MSEC,
                stages_max[i] / NSECS_PER_MSEC);
    }
    fclose(f);
}

#endif
