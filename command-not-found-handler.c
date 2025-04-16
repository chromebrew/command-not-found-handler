/*
  Copyright (C) 2013-2025 Chromebrew Authors

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see https://www.gnu.org/licenses/gpl-3.0.html.
*/

/*
  command-not-found-handler: Show suggestions for invalid command calls

  Usage: ./command-not-found-handler [command name] [filelist search path]

  cc ./command-not-found-handler -O2 -o command-not-found-handler
*/

#define _XOPEN_SOURCE 700 // for nftw()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ftw.h>
#include <libgen.h>
#include <linux/limits.h>

const char *cmd_search_path[2] = {
  "/usr/local/bin",
  "/usr/local/sbin"
};

int total_fuzzy_matches = 0,
    total_exact_matches = 0;

char fuzzy_matches[10][100], exact_matches[5][30];
char *filelist_path, *cmd_to_search;

float get_similarity(const char *str1, const char *str2) {
  // get_similarity(): Calculate similarity of two strings
  //
  // memo is the matrix for dynamic programming
  //
  // memo[i, j] = the edit distance between the prefixes of str1 and str2 of size i and j.
  int str1_len = strlen(str1),
      str2_len = strlen(str2);

  // initialize matrix with zeros
  char memo[str1_len + 1][str2_len + 1];
  memset(memo, 0, sizeof(memo[0][0]) * (str1_len + 1) * (str2_len + 1));

  for (int i = 0; i < str1_len; i++) memo[i + 1][0] = i + 1;
  for (int j = 0; j < str2_len; j++) memo[0][j + 1] = j + 1;

  for (int i = 0; i < str1_len; i++) {
    for (int j = 0; j < str2_len; j++) {
      memo[i + 1][j + 1] = memo[i][j];
      if (str1[i] != str2[j]) {
        if (memo[i + 1][j] < memo[i + 1][j + 1]) memo[i + 1][j + 1] = memo[i + 1][j];
        if (memo[i][j + 1] < memo[i + 1][j + 1]) memo[i + 1][j + 1] = memo[i][j + 1];
        memo[i + 1][j + 1]++;
      }
    }
  }

  return (str1_len - memo[str1_len][str2_len]) / (float) str1_len;
}

int compare_name(const char *path, const struct stat *info, int flag, struct FTW *ftwbuf) {
  // compare_name(): find similar commands from filelist by comparing the file name
  switch (flag) {
    case FTW_F:
    case FTW_SL:
    case FTW_SLN:
      // only scan for .filelist files
      if (strcmp((path + strlen(path) - 9), ".filelist") != 0) return 0;

      FILE *fp = fopen(path, "r");
      char *cmdName, pkgName[PATH_MAX], buf[PATH_MAX];

      if (fp == NULL) {
        // error
        fprintf(stderr, "Failed to open %s (%s)\n", path, strerror(errno));
        exit(errno);
      }

      // extract package name from path
      strncpy(pkgName, basename((char *) path), PATH_MAX);
      pkgName[strchr(pkgName, '.') - pkgName] = '\0';

      while (fgets(buf, sizeof(buf), fp) != NULL) {
        for (int i = 0; i < 2; i++) {
          // identify executable commands by checking whether the file is under executable paths
          if (strncmp(buf, cmd_search_path[i], strlen(cmd_search_path[i])) == 0) {
            // remove trailing newline
            buf[strcspn(buf, "\n")] = 0;

            // extract command name from path
            cmdName = basename(buf);

            if (strcmp(cmdName, cmd_to_search) == 0) {
              // command with exact name found
              strncpy(exact_matches[total_exact_matches++], pkgName, 30);
            } else if (get_similarity(cmdName, cmd_to_search) > 0.7) {
              // command with similar name found
              snprintf(fuzzy_matches[total_fuzzy_matches++], 100, "Command '%s' from package %s", cmdName, pkgName);
            }

            break;
          }
        }
      }
    break;
    case FTW_NS:
      // error
      fprintf(stderr, "%s: stat failed! (%s)\n", path, strerror(errno));
      exit(errno);
    break;
  }

  return 0;
}

int main(int argc, char **argv) {
  cmd_to_search = argv[1];
  filelist_path = argv[2];

  if (argc != 3) {
    fprintf(stderr, "Usage: %s [command name] [search path]\n", argv[0]);
    return 1;
  }

  // search and compare against all available commands from filelist
  nftw(filelist_path, compare_name, 100, FTW_PHYS | FTW_MOUNT);

  if (total_exact_matches > 0) {
    fprintf(stderr, "The command '%s' is not currently installed\n\n", cmd_to_search);
    fprintf(stderr, "However, the following Chromebrew package(s) provide it:\n\n");
    for (int i = 0; i < total_exact_matches; i++) fprintf(stderr, "  %s\n", exact_matches[i]);
    fprintf(stderr, "\nInstall one of them with 'crew install <package>'\n");
  } else if (total_fuzzy_matches > 0) {
    fprintf(stderr, "No command '%s' found. Did you mean:\n\n", cmd_to_search);
    for (int i = 0; i < total_fuzzy_matches; i++) fprintf(stderr, "  %s\n", fuzzy_matches[i]);
    fprintf(stderr, "\nInstall one of them with 'crew install <package>'\n");
  } else {
    fprintf(stderr, "%s: command not found\n", cmd_to_search);
  }

  return 0;
}
