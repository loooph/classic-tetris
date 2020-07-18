#include <SDL2/SDL.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "constants.h"
#include "input.h"
#include "graphics.h"
#include "game.h"




void alarm_handler()
{
}


uint32_t struct_itimverval[] = { 0, 10000, 0, 10000 };
uint32_t struct_sa_handler[] = { 0, 0, 0, 0, 0 };
uint32_t struct_timespec[] = { 1, 0 };

struct itimerval val;
struct timeval interval;
struct timeval value;

int main()
{
    // ignore alarm signal
    //*struct_sa_handler = (uint32_t) &alarm_handler;
    //syscall(SYS_rt_sigaction, SIGALRM, (const struct sigaction*) struct_sa_handler, NULL);
    interval.tv_sec = 0;
    interval.tv_usec = 10000;
    value.tv_sec = 0;
    value.tv_usec = 10000;
    val.it_interval = interval;
    val.it_value = value;
    signal(SIGALRM, alarm_handler);

    // set a periodic interrupt
    syscall(SYS_setitimer, ITIMER_REAL, &val, NULL);

    setup_graphics();
    setup_inputs();

    for (;;) {
        struct piece current_piece;
        int last_inputs[NUM_BUTTONS] = { 0 };
        reset_playfield();
        draw_playfield();
        generate_next_piece(&current_piece);

        while (next_state(last_inputs, &current_piece)) {
            draw_playfield();
            syscall(SYS_nanosleep, struct_timespec, NULL);
        }
    }

    exit(0);
}
