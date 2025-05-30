// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

class ProgressBar {
 public:
  void update(double TotalToDownload, double NowDownloaded);

  static void setDoNotShow(bool doNotShow);

 private:
  static bool _doNotShow;

  int last_value = 0;
};
