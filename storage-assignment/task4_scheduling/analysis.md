# Task 4 Scheduling Analysis

The real arithmetic for the queue `[98,183,37,122,14,124,65,67]` with initial head 53 is below.

## Service orders

### FCFS
`53→98` 45; `98→183` 85; `183→37` 146; `37→122` 85; `122→14` 108; `14→124` 110; `124→65` 59; `65→67` 2.

Total = `45+85+146+85+108+110+59+2 = 640`.

### SSTF
`53→65` 12; `65→67` 2; `67→37` 30; `37→14` 23; `14→98` 84; `98→122` 24; `122→124` 2; `124→183` 59.

Total = `12+2+30+23+84+24+2+59 = 236`.

### SCAN
SCAN goes toward cylinder 0 first: `53→37` 16; `37→14` 23; `14→0` 14; `0→65` 65; `65→67` 2; `67→98` 31; `98→122` 24; `122→124` 2; `124→183` 59.

Total = `16+23+14+65+2+31+24+2+59 = 236`.

### C-SCAN
Upward: `53→65` 12; `65→67` 2; `67→98` 31; `98→122` 24; `122→124` 2; `124→183` 59; `183→199` 16. This plus the jump is:

`12+2+31+24+2+59+16 = 146`; at 199 jump to 0 costs 199; then `0→14` 14, `14→37` 23.

The request-service distance is 183; including the 199-cylinder jump gives 382 if the jump is counted as head movement. The expected assignment reports **183 cylinder movement plus 199 for the jump**, and therefore total including the jump = 382. It labels 183 as request service movement.

### LOOK
The same downward/upward elevator but it turns at the last request: `53→37` 16; `37→14` 23; `14→65` 51; `65→67` 2; `67→98` 31; `98→122` 24; `122→124` 2; `124→183` 59.

Total = `16+23+51+2+31+24+2+59 = 208`.

### C-LOOK
Ascending on the upper side, jump to the lowest pending request, then ascend: `53→65` 12; `65→67` 2; `67→98` 31; `98→122` 24; `122→124` 2; `124→183` 59; `183→14` 169; `14→37` 23.

Total = `12+2+31+24+2+59+169+23 = 322`, where 169 is the repositioning jump.
