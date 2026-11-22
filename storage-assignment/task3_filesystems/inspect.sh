#!/bin/sh
set -e
printf '=== uname ===\n'; uname -a
printf '=== block devices ===\n'; lsblk -f || true
printf '=== root fs ===\n'; stat -f / || true
printf '=== inode usage ===\n'; df -i || true
printf '=== disk usage ===\n'; df -h || true
printf '=== inode of a file ===\n'; ls -i /etc/hosts || true
printf '=== stat of a file ===\n'; stat /etc/hosts || true
printf '=== ext4 superblock (may need sudo) ===\n'; tune2fs -l "$(findmnt -no SOURCE / 2>/dev/null)" 2>/dev/null || dumpe2fs -h "$(findmnt -no SOURCE / 2>/dev/null)" 2>/dev/null || echo 'tune2fs/dumpe2fs unavailable or not readable — sudo not used'
printf '=== IO scheduler ===\n'
for d in /sys/block/*/queue/scheduler; do printf '%s: ' "$d"; cat "$d"; done
