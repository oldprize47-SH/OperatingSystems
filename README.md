# Operating Systems Labs

This repository contains my operating-systems coursework in C, along with examples supplied for the course.

In [HW1](HW1/hw1_21800275.c), a program starts another command, receives its output through a pipe and searches that output. The [assignment README](HW1/README.txt) describes the command arguments and the exact and flexible search modes.

[HW2](HW2/rwlock.c) is a reader/writer synchronisation exercise. Its input sequence is stored in [sequence.txt](HW2/sequence.txt).

## Building

The code expects a Linux/POSIX environment with GCC, pthreads and semaphores. The following are suggested build commands; they have not been run in the Windows portfolio environment:

```sh
gcc -Wall -Wextra HW1/hw1_21800275.c -o wspipe
gcc -Wall -Wextra -pthread HW2/rwlock.c -o rwlock
```

Read the command passed to HW1 before executing it. The generated binaries in the archive are historical files, not portable builds. No new concurrency stress test or fairness measurement has been performed.

The `Source_Codes_for_Ch*` directories contain teaching examples rather than my original implementations.

[Original repository](https://github.com/oldprize47/2025_OS). Original history and attribution are retained.
