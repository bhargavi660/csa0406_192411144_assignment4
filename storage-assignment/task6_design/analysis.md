# Task 6 Design Analysis

## Workload requirements

The target 5,000 concurrent streams at 4 Mbps requires:

`5,000 × 4 Mbps = 20,000 Mbps = 20 Gbps`.

This is aggregate sustained application throughput; the actual CDN or network path should retain additional link and packet overhead headroom.

## Capacity

For 10,000 videos:

- 300 MB each: `10,000 × 300 MB = 3,000,000 MB`, or about 2.93 TiB.
- 2 GB each: `10,000 × 2,000 MB = 20,000,000 MB`, or about 19.07 TiB.
- Design A with 3 replicas: about 8.79 TiB for the 300 MB case and 57.21 TiB for the 2 GB case.

Design A capacity should also allow 20–30% SSD cache headroom, snapshots, and replacement overhead. For the table, raw library plus three copies is the baseline.

## Seek calculation

At 64 KB, a block is 65,536 bytes. For an offset:

`block = floor(offset / 65,536)`.

A one-minute offset in a 300 MB file is `60,000,000 bytes`; it maps to block 915. An arbitrary minute therefore requires one index lookup plus one data read under indexed/extent allocation. The design may be many index levels deep for large files, but a hot inode/extent tree can remain cached.

## Cache effect

The simulation shows that at a 80% cache hit ratio only 20% of 5,000 streams reaches origin: 1,000 streams or 4 Gbps. At 95%, origin load drops to 250 streams or 1 Gbps. Cache performance is more significant than origin CPU alone.

## Cost

Design B assumes $0.023/GB-month storage and $0.09/GB egress for illustration. For 10,000 300 MB videos, storage is 3,000 GB and costs about `$69/month`. For 10,000 2 GB videos, storage is 20,000 GB and costs about `$460/month`. The simulation's simple continuous egress model is `6.57 TB/month = $591.30/month`. Real CDN pricing and transfer discounts vary.
