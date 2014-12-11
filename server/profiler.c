#include "../common/exceptions.h"
#include "../common/messages.h"
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

FILE *f;

void profiler_close_file() {
	fclose(f);
}

#define BYTES_PER_MB (1024LL * 1024)

void profiler_save(const char *filename) {
	f = fopen(filename, "w");
	if (f == NULL)
		throw_exc(ERR_FILE_OPEN_FOR_WRITING, filename);
	push_finally(profiler_close_file);
	
	if (fprintf(f,
		"Diffs Traffic:\n"
		"    Speed: %.1lf Mb/s\n"
		"    Real: %.1lf Mb\n"
		"    Uncompressed: %.1lf Mb\n"
		"    Rate: %.1lf%%\n",
		(traffic_diffs / BYTES_PER_MB) /
				(stages_sum[STAGE_TRANSFER] / NSECS_PER_SEC),
		(double) traffic_diffs / BYTES_PER_MB,
		(double) traffic_uncompressed / BYTES_PER_MB,
		(double) traffic_diffs / traffic_uncompressed * 100.0
	) < 0)
		throw_exc(ERR_FILE_WRITE, filename);
	
	int i;
	for (i = 0; i < STAGES_COUNT; i++) {
		double average = stages_sum[i] / stages_calls[i];
		
		if (fprintf(f,
			"\nStage \"%s\":\n"
			"    Average: %.1lf ms\n"
			"    Total: %.0lf ms in %d calls\n"
			"    min = %.1lf ms, max = %.1lf ms\n",
			stages_names[i],
			average / NSECS_PER_MSEC,
			stages_sum[i] / NSECS_PER_MSEC, stages_calls[i],
			(double) stages_min[i] / NSECS_PER_MSEC,
					(double) stages_max[i] / NSECS_PER_MSEC
		) < 0)
			throw_exc(ERR_FILE_WRITE, filename);
	}
	
	pop_finally();
}
