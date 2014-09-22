#include "profiler.h"

#include <stdio.h>
#include <time.h>


long long get_time_nsec() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * NSECS_PER_SEC + spec.tv_nsec;
}


const char *stages_names[STAGES_COUNT] = {"Shot", "Diff", "Transfer", "Draw"};

long long stages_start[STAGES_COUNT],
        stages_min[STAGES_COUNT], stages_max[STAGES_COUNT];
double stages_sum[STAGES_COUNT];
int stages_calls[STAGES_COUNT];

long long traffic_diffs = 0;
long long traffic_uncompressed = 0;

void profiler_start(int stage) {
    stages_start[stage] = get_time_nsec();
}

void profiler_finish(int stage) {
    long long duration = get_time_nsec() - stages_start[stage];
    stages_sum[stage] += duration;
    if (!stages_calls[stage] || duration < stages_min[stage])
        stages_min[stage] = duration;
    if (!stages_calls[stage] || duration > stages_max[stage])
        stages_max[stage] = duration;
    stages_calls[stage]++;
}

#define BYTES_PER_MB (1024LL * 1024)

void profiler_save(const char *filename) {
    FILE *f = fopen(filename, "w");
    
    fprintf(f, "Diffs Traffic:\n");
    fprintf(f, "    Speed: %.1lf Mb/s\n",
            (traffic_diffs / BYTES_PER_MB) /
            (stages_sum[STAGE_TRANSFER] / NSECS_PER_SEC));
    fprintf(f, "    Real: %.1lf Mb\n",
            (double) traffic_diffs / BYTES_PER_MB);
    fprintf(f, "    Uncompressed: %.1lf Mb\n",
            (double) traffic_uncompressed / BYTES_PER_MB);
    fprintf(f, "    Rate: %.1lf%%\n",
            (double) traffic_diffs / traffic_uncompressed * 100.0);
    
    int i;
    for (i = 0; i < STAGES_COUNT; i++) {
        double average = stages_sum[i] / stages_calls[i];
        
        fprintf(f, "\nStage \"%s\":\n", stages_names[i]);
        fprintf(f, "    Average: %.1lf ms\n", average / NSECS_PER_MSEC);
        fprintf(f, "    Total: %.0lf ms in %d calls\n",
                stages_sum[i] / NSECS_PER_MSEC, stages_calls[i]);
        fprintf(f, "    min = %.1lf ms, max = %.1lf ms\n",
                (double) stages_min[i] / NSECS_PER_MSEC,
                (double) stages_max[i] / NSECS_PER_MSEC);
    }
    
    fclose(f);
}
