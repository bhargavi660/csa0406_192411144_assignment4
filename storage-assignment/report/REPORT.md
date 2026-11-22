# Operating Systems / Storage Architecture Report

## Task 1 – File-System Design Review (6)

### 1.1 Weaknesses of the proposed flat design

1. **Name clashes:** 12,000 users sharing one directory cannot safely use the same assignment name or extension.
2. **Linear lookup:** a flat directory is scanned linearly. For `n=12,000`, average unsuccessful lookup requires about `n/2=6,000` entry comparisons, while successful lookup averages fewer only when early entries are preferred. Each comparison can involve dentry and inode cache misses, and metadata growth reduces cache locality.
3. **Contiguous allocation:** files cannot grow if adjacent blocks are occupied; extending, defragmenting, or relocating a file is expensive. External fragmentation makes large allocation requests fail despite enough aggregate free space.
4. **No protection:** without ACLs or mode bits, every user can read or delete unrelated files.
5. **No sharing:** no link or group mechanism separates a student's private home directory from shared course data.
6. **Backup/recovery blast radius:** all users share one namespace and allocation/metadata area. A corrupted directory or failed restoration affects the entire institution.

### 1.2 Improved design

Use a tree rooted at `/home/<dept>/<year>/<rollno>/`. Naming follows `rollno_course_assignment_vN.ext`. Store owner, group, mode, size, timestamps, and ACL attributes in each inode. Apply POSIX `rwxrwxrwx` as a base mode-grid concept and enforce narrower defaults (`0700` private homes, course directories `2770`, public read-only materials `2755`). POSIX ACL entries permit named users and groups:

```sh
setfacl -m u:staff:rwx,g:cs2026:rwx /home/cse/2026/student123/project
getfacl /home/cse/2026/student123/project
```

Use an acyclic graph with hard links for shared read-only material and symlinks or group directories for project folders. Hard links share inode data but cannot normally cross filesystems. Symlinks should be limited to content users are authorized to reach. Enforce user and department quotas, separate backups and snapshots per department, and journal metadata consistently.

### 1.3 Privacy and sharing

**Privacy is protected by private default home directories, owner-only modes, named-group ACLs, and server-side audit logs.** Each shared project receives an explicit group containing only enrolled or staff members. POSIX ACL masks limit effective rights while course materials remain read-only to the broader community. Quotas prevent a single user from exhausting shared storage. Backups remain encrypted and access-controlled; staff escalation is logged. Hard links only expose authorized inode content, and symlinks never bypass directory permissions. (87 words)

### Alternative verdict

Google Drive or OneDrive offers indexed metadata and fast search, but file storage, copies, version history, and access-control history remain under the vendor. Institutional control is weakened by vendor-held keys, cross-region data residency, and DPAs. The shared server wins when campus privacy and retention rules are controlling, while a managed cloud suite wins for large-scale search and collaboration. For this 12,000-user deployment, a campus-controlled hierarchical filesystem with explicit ACLs and a recoverable backup tier is the better verdict.

## Task 2 – Workload to File-Allocation Method (8)

### 2.1 Operation table

| Operation | Frequency | Access |
|---|---:|---|
| Create | Medium | Direct metadata allocation |
| Write/append | Continuous, high | Sequential, with large extents preferred |
| Sequential read | High | Sequential |
| Seek by time/random read | Medium | Direct block lookup |
| Delete after 30 days | Scheduled | Direct metadata plus free-block marking |
| Directory operations | Low | Indexed metadata lookup |

### 2.2 Allocation comparison

| Method | Sequential | Direct/random | Growth | Fragmentation | Reliability |
|---|---|---|---|---|---|
| Contiguous | Excellent | Excellent | Poor without relocation | High external | A single failure may affect a long run |
| Linked/FAT | Good only when cached | Poor O(k) chain | Easy | Internal chain overhead | FAT corruption harms every link |
| Indexed/extent | Excellent in contiguous extents | Excellent | Good | Low external | Index loss is critical; checksums/backups needed |

Extent-based indexed allocation wins. It also matches the video workload: sequential reads use one large extent and random minute seeks use an extent/index lookup.

### 2.3 Calculations

A 4 GB file with 4 KB blocks needs:

`4 GB / 4 KB = 4,294,967,296 / 4,096 = 1,048,576 blocks`.

**Contiguous:** one block calculation, `block=floor(offset/blockSize)`, then one seek/read to that block. It is constant O(1).

**Linked/FAT:** block `k` requires walking `k` pointers. Without a cached FAT, block 1,048,575 can require 1,048,576 pointer reads in the worst case. With the FAT in memory, pointer lookup is O(k) memory work rather than disk reads.

**Simple inode-based case:** an inode can directly point to the needed block. The address lookup is part of one inode read, followed by one data read: 2 disk reads.

**Single-level external index:** `4,096 / 4 = 1,024 entries per index block`, so `1,048,576 / 1,024 = 1,024` index blocks. Finding a block in the worst case is 1,024 index-block reads plus 1 data read = 1,025 reads. Multi-level indexes reduce depth logarithmically.

**Inode maximum file size:**

- 12 direct × 4 KB = 48 KB
- 1 indirect = `4 KB/4 B=1,024` pointers × 4 KB = 4 MB
- Double indirect = `1,024×1,024×4 KB=4 GB`
- Triple indirect = `1,024^3×4 KB=4 TB`

Exact sum:

`48×1024 + 4×1024×1024 + 4×1024×1024×1024 + 4×1024×1024×1024×1024`

`= 49,152 + 4,194,304 + 4,294,967,296 + 4,398,046,511,104`

`= 4,402,345,721,856 bytes`.

### 2.4 Free-space management

For 4 TB with 4 KB blocks:

- Blocks = `4×1024^4 / 4×1024 = 1,073,741,824`
- Bits = 1,073,741,824
- Bit-vector bytes = `1,073,741,824/8 = 134,217,728 bytes = 128 MiB`

| Method | Cost | Allocation behavior |
|---|---|---|
| Bit vector | 128 MiB | Fast scan for a run, but one allocation may read multiple bitmap blocks |
| Linked free list | One pointer per free block | Traversal is sequential and poor for finding a contiguous run |
| Grouping/counting | n free block addresses/count per block | Fewer disk reads to find free blocks or runs; good for extent allocators |

Grouping/counting is the practical verdict for a large server because it serves runs and space needs with fewer reads.

## Task 3 – Modern File Systems (9)

### 3.1 Filesystem comparison

| FS | Max file | Typical max volume | Journal | Snapshots | Checksums | CoW |
|---|---|---|---|---|---|---|
| FAT32 | 4 GB | 2 TB | No | No | No | No |
| NTFS | 16 EiB (theoretical) | 256 TB common | Yes | VSS is separate | Limited metadata | No default |
| ext4 | 16 TB | 1 EiB | Yes | External tooling | Limited metadata | No |
| XFS | 8 EiB | 8 EiB | Yes | External tooling | Limited metadata | No |
| Btrfs | 16 EiB | 16 EiB | Copy-on-write metadata | Built-in | Yes | Yes |
| ZFS | 16 EiB | 16 EiB | Transaction groups/CoW | Built-in | Yes | Yes |
| APFS | 8 EiB | ~Exabytes | Yes | Built-in | Yes for many structures | Yes |

Exact limits vary by implementation/version.

### 3.2 Failure behavior

Without journaling, a power failure mid-write can leave directory, inode, and data bitmap entries inconsistent. `fsck` must scan the whole filesystem and may lose or misplace blocks.

Journaling writes metadata intent to a write-ahead log, then replays completed or unfinished transactions after crash. ext4 modes differ:

- **journal:** data and metadata in the journal; strongest consistency, slowest.
- **ordered:** metadata journaled, data written before metadata commit; ext4 default and a balance.
- **writeback:** metadata journal only; stale or corrupted data blocks are possible, fastest of the three.

CoW writes a new copy and atomically swaps the root pointer, so live data is never overwritten in place. Interrupted writes leave the old root valid.

### 3.3 Recommendations

- **Pen drive for Windows+Linux:** exFAT, because FAT32 cannot store a file above 4 GB and NTFS is less portable than exFAT.
- **Linux DB server:** XFS for large parallel I/O and ext4 when mature ordered journaling, a conservative rollout, and straightforward tooling matter.
- **Laptop:** ext4 on Linux and APFS on macOS.
- **Backup server:** ZFS or Btrfs for checksums, snapshots, and send/receive.

Btrfs/ZFS provide near-instant logical crash recovery and snapshots, but CoW writes amplify small random DB updates and snapshot/checksum metadata adds overhead. As an illustration, an extra 10% snapshot/metadata reserve is an estimate of the space needed for a 90% full backup pool; the source of the number is this report's planning assumption, not a filesystem benchmark.

### 3.4 Captured evidence

The real WSL evidence reported:

```text
Linux Bhargavi 6.18.33.2-microsoft-standard-WSL2
Block size: 4096       Fundamental block size: 4096
Blocks: Total: 263940717  Free: 263337764  Available: 249911896
Inodes: Total: 67108864   Free: 67056683
/dev/sdd       67108864   52181 67056683    1% /
File: /etc/hosts
size: 413 Blocks: 8 IO Block: 4096 regular file
Device: 8,48 Inode: 743 Links: 1
Access: (0644/-rw-r--r--)
```

The root reports 67,108,864 inodes with 67,056,683 free, only 52,181 used. The 4 KB block size matches the assignment calculations. Free block count is over 263 million and disk usage is 954 GB available. `tune2fs`/`dumpe2fs` could not inspect the virtual root superblock without valid device access and `sudo` was not used. The `/sys/block/*/queue/scheduler` output reports `[none]` as active for sda/sdb/sdc/sdd, while mq-deadline and kyber remain alternatives.

## Task 4 – Disk Scheduling (7)

### 4.1 Work arithmetic

Captured orders and totals:

| Algorithm | Order | Total |
|---|---|---:|
| FCFS | 98,183,37,122,14,124,65,67 | 640 |
| SSTF | 65,67,37,14,98,122,124,183 | 236 |
| SCAN | 37,14,0,65,67,98,122,124,183 | 236 |
| C-SCAN | 65,67,98,122,124,183,0,14,37 | 183 + 199 jump |
| LOOK | 37,14,65,67,98,122,124,183 | 208 |
| C-LOOK | 65,67,98,122,124,183,14,37 | 322 |

Detailed arithmetic is in `task4_scheduling/analysis.md`.

### 4.2 10,000-request measured table

| Load | Algorithm | Movement | Avg movement | Avg wait | Max wait | Throughput |
|---|---|---:|---:|---:|---:|---:|
| Light | FCFS | 657821 | 65.78 | 65.78 | 196 | 15.20 req/1000c |
| Light | SSTF | 234813 | 23.48 | 23.48 | 197 | 42.59 |
| Light | SCAN | 234865 | 23.49 | 23.49 | 157 | 42.58 |
| Light | C-SCAN | 340399 | 34.04 | 34.04 | 199 | 29.38 |
| Light | LOOK | 199479 | 19.95 | 19.95 | 155 | 50.13 |
| Light | C-LOOK | 306627 | 30.66 | 30.66 | 199 | 32.61 |
| Medium | FCFS | 663364 | 66.34 | 66.34 | 196 | 15.07 |
| Medium | SSTF | 28683 | 2.87 | 2.87 | 155 | 348.64 |
| Medium | SCAN | 25053 | 2.51 | 2.51 | 63 | 399.15 |
| Medium | C-SCAN | 39305 | 3.93 | 3.93 | 199 | 254.42 |
| Medium | LOOK | 24769 | 2.48 | 2.48 | 63 | 403.73 |
| Medium | C-LOOK | 39011 | 3.90 | 3.90 | 199 | 256.34 |
| Heavy | FCFS | 664531 | 66.45 | 66.45 | 196 | 15.05 |
| Heavy | SSTF | 2892 | 0.29 | 0.29 | 147 | 3457.81 |
| Heavy | SCAN | 2520 | 0.25 | 0.25 | 54 | 3968.25 |
| Heavy | C-SCAN | 3970 | 0.40 | 0.40 | 199 | 2518.89 |
| Heavy | LOOK | 2520 | 0.25 | 0.25 | 54 | 3968.25 |
| Heavy | C-LOOK | 3970 | 0.40 | 0.40 | 199 | 2518.89 |

Waiting values are cylinder-movement units within the batch; throughput is `10,000/(movement/1000)` requests per 1000 cylinder-movement units.

### 4.3 Fairness

SSTF minimizes immediate distance and can starve requests at cylinder 183 while nearer requests keep arriving. SCAN sweeps and favors middle tracks only within the sweep; C-SCAN treats the disk as circular and bounds wait more uniformly. LOOK/C-LOOK are equivalent to less wasteful turn points.

The demo kept 1,000 cylinder-60-neighborhood arrivals at 1 ms while SSTF had cylinder 183 pending. The reported wait was **1000 ms**.

### 4.4 Linux schedulers

Linux commonly uses `none` or `mq-deadline` for SSD/NVMe. SSD seek cost is low and NVMe is parallel, so elevator sorting can add queue latency rather than improve throughput. HDD workloads benefit more from elevator batching and deadline fairness. The real WSL machine reported `[none]` active for sda/sdb/sdc/sdd.

## Task 5 – Distributed and Cloud Storage (7)

### 5.1 Storage systems

| System | Consistency | Availability | Performance | Cost |
|---|---|---|---|---|
| NFS | Close-to-open cache consistency; not a distributed database transaction | Server/export failure affects clients | Good for POSIX metadata, but server can become a bottleneck | Low/mixed: local network and server hardware |
| HDFS/Ceph | HDFS single-writer with strong read visibility after close; Ceph tunable consistency and replication pools | Replication and replica placement tolerate failures | High parallel throughput, not ideal for millions of small operations | Moderate cluster operation cost |
| S3/GCS | Strong read-after-write consistency for new objects and overwrites; object semantics | Highly replicated and regional/multi-zone | High aggregate throughput, not POSIX random mutation | Pay per storage, API, transfer, and operations |

### 5.2 Replication

HDFS uses rack-aware replication, commonly 3 copies. A three-copy policy survives two block failures if placement is on three racks and no common failure is present, and read locality reduces network distance. Replication factor is not durability by itself: it protects against node/rack loss but not software corruption or accidental deletion. Checksums and recovery are still required.

Strong consistency means users immediately see the authoritative write. Eventual consistency can return stale data temporarily. For a university file share, strong metadata/close semantics prevent duplicate or lost submissions; eventual object consistency can cause a student to upload a submission and briefly receive an old version.

### 5.3 Device management

- **Device drivers:** sit below a class or block interface and translate kernel operations into controller/device commands.
- **Buffering:** holds data across mismatched producer/consumer speeds and units.
- **Caching:** keeps frequently reused blocks or metadata to avoid slower I/O.
- **Spooling:** queues independent work such as prints or batch uploads for later serialized processing.
- **Block-device abstraction:** presents fixed-size random sectors/blocks through `/dev` and filesystems, hiding geometry and controller details.

The alternative one-big-NFS-server scales badly: as `N` users grow, metadata, network, disk, and cache pressure converge on one server. Storage is 1× rather than replicated and a server failure makes the share unavailable.

## Task 6 – Design Challenge (13)

### Stage 1: Workload and testable requirements

| Operation | Pattern |
|---|---|
| Create/upload | Low-frequency bulk ingest |
| Write | Large sequential writes |
| Sequential read | High during playback |
| Random seek | Medium: random minute in one video |
| Delete | Scheduled after retention policy |

Measurable requirements:

- 95th-percentile startup latency below 2 seconds at 5,000 concurrent streams.
- Sustained application throughput of at least 20 Gbps.
- Cache hit ratio at least 80%, with origin below 4 Gbps.
- Three replicas for hot content and erasure coding for cold content.
- 99.9% monthly availability.
- Rebuild a failed SSD cache without replacing service availability.

### Stage 2: Architecture

Use SSD cache tier + HDD origin, with object cold storage for unviewed or expired content. The cache stores 64 KB–1 MB blocks and uses indexed/extent allocation. HDD origin uses larger extents for sequential playback. Cold storage uses immutable objects and checksums.

### Stage 3: Decisions

- **Block size:** 64 KB–1 MB. It reduces index entries and I/O counts for video; 4 KB causes excessive metadata and I/O.
- **SSD scheduler:** `none` or `mq-deadline` to retain parallelism and deadline behavior.
- **HDD scheduler:** `mq-deadline` or `bfq` for fairness and batching.
- **Cache:** LRU with 80% minimum target; cache size is a deployment parameter. For 20 Gbps, a 25% hot-set rule requires approximately 5 Gbps of cache egress, plus growth headroom.
- **Replication:** 3 replicas for hot regional content; erasure coding can reduce cold-object storage overhead.

### Stage 4: Calculation and simulation

The measured throughput is 20 Gbps. A 64 KB block maps to `floor(offset/65,536)`. A minute seek is one index read plus one data read. Cache hit ratios reduce origin load linearly. Real `stream_sim.c` output is:

- 300 MB video × 10,000: 3,000,000 MB raw
- 2 GB video × 10,000: 20,000,000 MB raw
- 20 Gbps target
- At 80% hit, origin 1,000 streams and 4 Gbps
- At 95% hit, origin 250 streams and 1 Gbps

See `task6_design/sim_output.txt`.

### Stage 5: Recommendation

**Design A is recommended for the startup.** It provides 64 KB cached random seeks, 3 regional replicas for hot content, predictable 20 Gbps origin capacity, and predictable hardware cost. Use an object/CDN cold tier for old or low-demand videos.

Design B is attractive for global scale because a CDN moves popular content near users and object storage removes replica management. The assumption is 30–50 ms edge RTT. For an explicit RTT model, use 30 ms to a nearby edge and 120 ms for a distant edge; object/CDN startup remains under 2 s if the first segment is cached, and it must retry/fetch if cold.

Storage for 10,000 300 MB videos is about 3,000 GB in Design B; 2 GB videos are 20,000 GB. The report scenario assumes `$0.023/GB-month` storage and `$0.09/GB` egress. At 3,000 GB, storage is `$69/month`; at 20,000 GB, `$460/month`. A simple monthly egress model reports 6.57 TB and `$591.30/month`. Operational effort is lower for B, but egress and institutional control may be higher.
