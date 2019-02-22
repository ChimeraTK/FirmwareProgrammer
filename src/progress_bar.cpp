#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <term.h>

#include <chrono>
using namespace std::chrono;

int last_value = 0;
high_resolution_clock::time_point last_time = high_resolution_clock::now();

int my_putchar(int c) {
  return putchar(c);
}

void ProgressBar(double TotalToDownload, double NowDownloaded) {
  int total_barlength;
  int col;
  struct winsize size;

  setupterm(NULL, fileno(stdout), (int*)0);

  ioctl(0, TIOCGWINSZ, (char*)&size); // get terminal size
  col = size.ws_col;
  total_barlength = col - 10;

  int progress = lround((NowDownloaded / TotalToDownload) * 100);
  int barlength = lround(((float)progress / 100) * total_barlength);

  if(progress != last_value) {
    // high_resolution_clock::time_point current_time =
    // high_resolution_clock::now(); duration<double, std::milli> time_diff =
    // current_time - last_time; printf("time_diff: %.2f ms\n", time_diff);
    // last_time = current_time;

    tputs(clr_eol, 1, my_putchar); // clear line
    printf("%4d%% [", progress);
    int ii = 0;
    for(; ii < barlength; ii++) {
      printf("=");
    }
    for(; ii < total_barlength; ii++) {
      printf(" ");
    }
    printf("]\r");
    fflush(stdout);

    last_value = progress;
  }
}
