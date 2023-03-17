// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "progress_bar.h"

#include <sys/ioctl.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <term.h>

/**********************************************************************************************************************/

int my_putchar(int c) {
  return putchar(c);
}

/**********************************************************************************************************************/

bool ProgressBar::_doNotShow{false};

/**********************************************************************************************************************/

void ProgressBar::setDoNotShow(bool doNotShow) {
  _doNotShow = doNotShow;
}

/**********************************************************************************************************************/

void ProgressBar::update(double TotalToDownload, double NowDownloaded) {
  if(_doNotShow) return;

  int total_barlength;
  int col;
  struct winsize size {};

  setupterm(nullptr, fileno(stdout), nullptr);

  ioctl(0, TIOCGWINSZ, static_cast<void*>(&size)); // get terminal size
  col = size.ws_col;
  total_barlength = col - 10;

  auto progress = int(lround((NowDownloaded / TotalToDownload) * 100));
  auto barlength = lround((double(progress) / 100) * total_barlength);

  if(progress != last_value) {
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

/**********************************************************************************************************************/
