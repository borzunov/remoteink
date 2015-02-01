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

long long traffic_compressed, traffic_uncompressed;

void profiler_traffic_init() {
	traffic_compressed = 0;
	traffic_uncompressed = 0;
}

void profiler_traffic_count_compressed(int value) {
	traffic_compressed += value;
}

void profiler_traffic_count_uncompressed(int value) {
	traffic_uncompressed += value;
}

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

#define BITS_PER_BYTE 8
#define BYTES_PER_MIB (1024LL * 1024)
#define BITS_PER_MBIT (1000LL * 1000)

FILE *f;

void defer_profiler_fclose_f() {
	fclose(f);
}

ExcCode profiler_save(const char *filename) {
	f = fopen(filename, "w");
	if (f == NULL)
		PANIC(ERR_FILE_OPEN_FOR_WRITING, filename);
	push_defer(defer_profiler_fclose_f);
	
	double speed = ((double) traffic_compressed *
				BITS_PER_BYTE / BITS_PER_MBIT) /
				(stages_sum[STAGE_TRANSFER] / NSECS_PER_SEC);
	if (fprintf(f,
		"Diffs Traffic:\n"
		"    Speed: %.1lf Mbit/s\n"
		"    Real: %.1lf MiB\n"
		"    Uncompressed: %.1lf MiB\n"
		"    Rate: %.1lf%%\n",
		speed,
		(double) traffic_compressed / BYTES_PER_MIB,
		(double) traffic_uncompressed / BYTES_PER_MIB,
		(double) traffic_compressed / traffic_uncompressed * 100.0
	) < 0)
		PANIC_WITH_DEFER(ERR_FILE_WRITE, filename);
	
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
			PANIC_WITH_DEFER(ERR_FILE_WRITE, filename);
	}
	
	pop_defer(defer_profiler_fclose_f);
	return 0;
}
