# Task 2 Allocation Analysis

## Captured allocation state

The initial allocation and release produce:

```text
AAAAAA........CCCCC.............
```

The free space is in two runs: blocks 6-13 (8 blocks) and blocks 19-31 (13 blocks). There are therefore 21 free blocks in total, but only the first run can start a contiguous 10-block allocation. `E` starts at 19 because that is the first run of at least 10 blocks.

After contiguous allocation at 19-28, the state is:

```text
AAAAAA........CCCCCEEEEEEEEEE....
```

The 21 free blocks are physically split and cannot satisfy the contiguous request. Thus the run demonstrates external fragmentation. Indexed allocation succeeds because it uses block 6 as the index block and 10 separate data blocks: 7-13 and 29-31. No contiguous run is required.

## 4 GB calculation

- 4 GB = 4,294,967,296 bytes.
- Block size = 4 KB = 4,096 bytes.
- Block count = 4,294,967,296 / 4,096 = 1,048,576 blocks.
- One index entry per block is 4 bytes.
- Entry capacity per 4 KB index block = 4,096 / 4 = 1,024 entries.
- A single-level index for the file needs 1,048,576 / 1,024 = 1,024 index blocks.

For a simple inode/direct-pointer model, reading arbitrary block `k` requires the inode read plus direct address lookup and one data read: 2 reads, with no FAT-chain walk. In a system that uses one external 1,024-block index array, the worst-case single-level case is 1,024 index reads plus 1 data read. A multi-level or inode-based tree needs multiple levels, not a linear pointer chain.

Linked/FAT allocation performs `k` pointer reads for block `k`; in the worst uncached 4 GB case that can become 1,048,576 FAT/pointer reads. If the entire FAT is cached, each step is an in-memory lookup and the disk cost is much lower, but cache residency is not guaranteed.
