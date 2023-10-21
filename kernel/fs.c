#include "types.h"
#include "defs.h"
#include "fs.h"

// there should be one superblock per disk device, but we run with
// only one device
struct superblock sb;

// Read the super block.
static void readsb(int dev, struct superblock *sb)
{
    struct buf *bp;

    bp = bread(dev, 1);
    memmove(sb, bp->data, sizeof(*sb));
    brelse(bp);
}

void fsinit(int dev)
{
    cprintf("fsinit begins...\n");
    readsb(dev, &sb);
    if (sb.magic != FSMAGIC)
        panic("invalid file system");
    cprintf("magic valid!\n");
    // TODO: initlog(dev, &sb);
}