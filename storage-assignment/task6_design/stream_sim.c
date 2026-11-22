#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define STUDENTS 200000
#define CONCURRENT 5000
#define BITRATE_BPS 4000000
#define BLOCK_KB 64

int main(void)
{
    long long n[] = {1000, 5000, 10000};
    const char *labels[] = {"1k", "5k", "10k"};
    printf("Video-stream design simulation\n");
    printf("Students: %d, target concurrency: %d, bitrate: %d Mbps\n",
           STUDENTS, CONCURRENT, BITRATE_BPS / 1000000);

    printf("\nCapacity needed (raw library copies)\n");
    printf("%-6s %-18s %-18s\n", "N", "300 MB/video", "2 GB/video");
    for (int i = 0; i < 3; i++) {
        printf("%-6s %14lld MB %14lld MB\n", labels[i],
               n[i] * 300LL, n[i] * 2000LL);
    }
    printf("A: 3 copies => 300 MB case: 3*N*300 MB; 2 GB case: 3*N*2,000 MB\n");

    double mbps = (double)CONCURRENT * 4.0;
    double gbps = mbps / 1000.0;
    printf("\nTarget throughput: %.0f Mbps = %.3f Gbps\n", mbps, gbps);
    printf("At 10 Gbps links, aggregate overhead headroom: %.1f streams/link-equivalent\n",
           10000.0 / 4.0);

    printf("\nArbitrary-minute disk reads (64 KB blocks)\n");
    long long file_mb = 300;
    long long file_bytes = file_mb * 1024 * 1024;
    long long block_bytes = BLOCK_KB * 1024;
    for (int minute = 1; minute <= 10; minute += 3) {
        long long offset = (long long)minute * 60 * 1000000;
        if (offset >= file_bytes) offset = file_bytes - block_bytes;
        long long block = offset / block_bytes;
        printf("minute %d: offset=%lld bytes, block=%lld, 1 index read + 1 data read\n",
               minute, offset, block);
    }
    printf("For a 2 GB file at 64 KB, offset=2000000000 bytes -> block=%lld\n",
           (long long)(2000000000LL / block_bytes));

    printf("\nCache hit ratio effect on origin load\n");
    printf("Hit ratio  Origin streams  Origin load (of 5000 streams)\n");
    for (int hit = 50; hit <= 95; hit += 5) {
        double origin = CONCURRENT * (1.0 - hit / 100.0);
        printf("%3d%%      %12.0f  %8.1f Mbps\n",
               hit, origin, origin * 4.0);
    }

    printf("\nDesign B cost estimate (assumptions: $0.023/GB-month, $0.09/GB egress)\n");
    double storage_gb[] = {300.0 * 10000.0 / 1000.0, 2000.0 * 10000.0 / 1000.0};
    double months_egress_fraction = 1.0;
    for (int i = 0; i < 2; i++) {
        double storage = storage_gb[i];
        double storage_cost = storage * 0.023;
        double egress_tb = 5000.0 * 4.0 * 3600.0 * 730.0 / 8.0 / 1000000000.0 * months_egress_fraction;
        double egress_cost = egress_tb * 1000.0 * 0.09;
        printf("2 GB? %s: %.0f GB storage=$%.2f/month; model egress=%.2f TB=$%.2f/month\n",
               i == 0 ? "300-MB-model" : "2-GB-model",
               storage, storage_cost, egress_tb, egress_cost);
    }

    printf("\nA vs B\n");
    printf("Startup latency: A lower in cache; B depends on CDN edge/RTT\n");
    printf("Throughput: both can scale; A local SSD/HDD throughput and B CDN bandwidth\n");
    printf("Cost: A hardware+operations; B pay-per-GB storage/egress\n");
    printf("Operational effort: A cache, replicas, replacement, tiering; B object/CDN managed\n");
    return 0;
}
