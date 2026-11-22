#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define REQUESTS 10000
#define CYLINDERS 200
#define ALGORITHMS 6

typedef struct {
    long long movement;
    long long total_wait;
    long long max_wait;
} Metrics;

static int requests[REQUESTS];
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static Metrics fcfs(int *batch, int count)
{
    Metrics m = {0, 0, 0};
    int pos = 53;
    for (int i = 0; i < count; i++) {
        int d = abs(batch[i] - pos);
        m.movement += d;
        m.total_wait += d;
        if (d > m.max_wait) m.max_wait = d;
        pos = batch[i];
    }
    return m;
}

static Metrics sstf(int *batch, int count)
{
    Metrics m = {0, 0, 0};
    int pos = 53;
    int done[REQUESTS] = {0};
    for (int k = 0; k < count; k++) {
        int best = -1, bd = INT_MAX;
        for (int i = 0; i < count; i++) {
            if (done[i]) continue;
            int d = abs(batch[i] - pos);
            if (d < bd) { bd = d; best = i; }
        }
        done[best] = 1;
        m.movement += bd;
        m.total_wait += bd;
        if (bd > m.max_wait) m.max_wait = bd;
        pos = batch[best];
    }
    return m;
}

static Metrics elevator(int *batch, int count, int cscan, int look)
{
    Metrics m = {0, 0, 0};
    int sorted[REQUESTS];
    memcpy(sorted, batch, (size_t)count * sizeof sorted[0]);
    qsort(sorted, (size_t)count, sizeof sorted[0], cmp_int);
    int pos = 53, order[REQUESTS + 2], n = 0, i;

    for (i = count - 1; i >= 0; i--) if (sorted[i] < pos) order[n++] = sorted[i];
    if (!look) order[n++] = 0;
    for (i = 0; i < count; i++) if (sorted[i] > pos) order[n++] = sorted[i];
    if (cscan) {
        n = 0;
        for (i = 0; i < count; i++) if (sorted[i] > pos) order[n++] = sorted[i];
        if (!look) order[n++] = 199;
        for (i = 0; i < count; i++) if (sorted[i] < pos) order[n++] = sorted[i];
    }

    int start = 53;
    pos = start;
    for (i = 0; i < n; i++) {
        int d = abs(order[i] - pos);
        m.movement += d;
        m.total_wait += d;
        if (d > m.max_wait) m.max_wait = d;
        pos = order[i];
    }
    return m;
}

int main(void)
{
    srand(42);
    for (int i = 0; i < REQUESTS; i++) requests[i] = rand() % CYLINDERS;

    const char *levels[] = {"light", "medium", "heavy"};
    const int queue[] = {10, 100, 1000};
    printf("%-8s %-8s %12s %12s %12s %12s %14s\n",
           "Load", "Algorithm", "Movement", "Avg movement", "Avg wait", "Max wait", "Throughput");

    for (int level = 0; level < 3; level++) {
        long long tm[ALGORITHMS] = {0}, tw[ALGORITHMS] = {0}, mx[ALGORITHMS] = {0};
        int pos = 53;
        for (int base = 0; base < REQUESTS; base += queue[level]) {
            int count = queue[level];
            if (base + count > REQUESTS) count = REQUESTS - base;
            int batch[REQUESTS];
            memcpy(batch, requests + base, (size_t)count * sizeof batch[0]);

            Metrics ms[ALGORITHMS];
            ms[0] = fcfs(batch, count);
            ms[1] = sstf(batch, count);
            ms[2] = elevator(batch, count, 0, 0);
            ms[3] = elevator(batch, count, 1, 0);
            ms[4] = elevator(batch, count, 0, 1);
            ms[5] = elevator(batch, count, 1, 1);
            for (int a = 0; a < ALGORITHMS; a++) {
                tm[a] += ms[a].movement;
                tw[a] += ms[a].total_wait;
                if (ms[a].max_wait > mx[a]) mx[a] = ms[a].max_wait;
            }
            pos = pos;
        }
        const char *names[] = {"FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};
        for (int a = 0; a < ALGORITHMS; a++) {
            double avg_move = (double)tm[a] / REQUESTS;
            double avg_wait = (double)tw[a] / REQUESTS;
            double throughput = REQUESTS * 1000.0 / (double)tm[a];
            printf("%-8s %-8s %12lld %12.2f %12.2f %12lld %12.2f req/1000c\n",
                   levels[level], names[a], tm[a], avg_move, avg_wait, mx[a], throughput);
        }
    }
    return 0;
}
