# `command-not-found` handler for Chromebrew
A utility for showing suggestions on invalid commands executed by the user:

- Show corresponding Chromebrew packages for commands that are not installed
- Search for similar commands for other unrecognized commands

## Showcase
```
$ PATH='' ls
The command 'ls' is not currently installed

However, the following Chromebrew package(s) provide it:

  uutils_coreutils
  coreutils

Install one of them with 'crew install <package>'
```
```
$ ngiix
No command 'ngiix' found. Did you mean:

  Command 'nginx' from package nginx

Install one of them with 'crew install <package>'
```

## License
Copyright (C) 2013-2025 Chromebrew Authors

This project including all of its source files is released under the terms of [GNU General Public License (version 3 or later)](http://www.gnu.org/licenses/gpl.txt).
