#ifndef PROFILER_H
#define PROFILER_H


#define ENABLE_PROFILER //

/* Every iteration of mainloop contains 5 stages:
 *   1). Take part of the screenshot
 *   2). Find differences with previous image and compress them for transfer
 *   3). Transfer data
 *   4). Draw differences to reader's screen buffer via reader's API
 *   5). Repaint part of the reader's screen, return confirmation of repaint
 *
 * At this time, the server executes the following functions:
 *   1).   screenshot_get()  in "server/screen.c"   (STAGE_SHOT)
 *   2).   image_send_diff() in "server/transfer.c" (STAGE_DIFF)
 *   3).   send_buffer()     in "server/transfer.c" (STAGE_TRANSFER)
 *   4-5). wait_confirm()    in "server/transfer.c" (STAGE_DRAW)
 *
 * The client executes the following functions:
 *   1-3). client_mainloop() in "client/client.c"
 *   4-5). client_exec()     in "client/client.c"
 * 
 * Profiler helps to measure time of each stage and find perfomance
 * problems. */

#define STAGES_COUNT 4

#define STAGE_SHOT 0
#define STAGE_DIFF 1
#define STAGE_TRANSFER 2
#define STAGE_DRAW 3


#define NSECS_PER_MSEC (1000LL * 1000)
#define NSECS_PER_SEC ((NSECS_PER_MSEC) * 1000)

long long get_time_nsec();

#ifdef ENABLE_PROFILER

void profiler_start(int stage);
void profiler_finish(int stage);

void profiler_save(const char *filename);

#define PROFILER_BEGIN profiler_start
#define PROFILER_END profiler_finish

#else

#define PROFILER_BEGIN(stage)
#define PROFILER_END(stage)

#endif


#endif
