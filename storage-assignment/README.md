# Storage Architecture Assignment

This repository contains source code, captured terminal output, and a written report for six operating-system/storage architecture tasks.

## Toolchain

The C programs use GCC with `-std=c11 -Wall -Wextra` and must compile without warnings. Commands below are POSIX/WSL/Linux commands.

## Build and run

```sh
cd storage-assignment
gcc -std=c11 -Wall -Wextra -o task2_allocation/alloc task2_allocation/alloc.c
./task2_allocation/alloc | tee task2_allocation/alloc_output.txt

gcc -std=c11 -Wall -Wextra -o task4_scheduling/sched task4_scheduling/sched.c
./task4_scheduling/sched | tee task4_scheduling/sched_output.txt

gcc -std=c11 -Wall -Wextra -o task4_scheduling/sched_sim task4_scheduling/sched_sim.c
./task4_scheduling/sched_sim | tee task4_scheduling/sim_output.txt

gcc -std=c11 -Wall -Wextra -o task6_design/stream_sim task6_design/stream_sim.c
./task6_design/stream_sim | tee task6_design/sim_output.txt

chmod +x task3_filesystems/inspect.sh
./task3_filesystems/inspect.sh | tee task3_filesystems/inspect_output.txt
```

Clean build:

```sh
rm -f task2_allocation/alloc task4_scheduling/sched task4_scheduling/sched_sim task6_design/stream_sim
```

## Source file map

| File | Task |
|---|---|
| `task2_allocation/alloc.c` | 2 |
| `task2_allocation/analysis.md` | 2 |
| `task4_scheduling/sched.c` | 4 |
| `task4_scheduling/sched_sim.c` | 4 |
| `task4_scheduling/analysis.md` | 4 |
| `task6_design/stream_sim.c` | 6 |
| `task6_design/analysis.md` | 6 |
| `task3_filesystems/inspect.sh` | 3 |
| `report/REPORT.md` | 1-6 |

## Results summary

- FCFS: 640 cylinders
- SSTF: 236 cylinders
- SCAN: 236 cylinders
- C-SCAN request service: 183 cylinders (+199-cylinder jump)
- LOOK: 208 cylinders
- C-LOOK: 322 cylinders including modeled repositioning
- Maximum inode-style file size: 48 KB + 4 MB + 4 GB + 4 TB = 4,402,345,721,856 bytes
- 4 TB bit vector at 4 KB blocks: 128 MiB
- Captured simulation winner in the heavy load table: LOOK/SCAN tie at 2,520 cylinder movement
