// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 29

struct bucket {
	struct spinlock lock;
	struct buf head;
}

struct {
	struct spinlock lock;
	struct buf buf[NBUF];
	struct bucket hash_tables[NBUCKET];
	// Linked list of all buffers, through prev/next.
	// Sorted by how recently the buffer was used.
	// head.next is most recent, head.prev is least.
	// struct buf head;
} bcache;

	void
binit(void)
{
	struct buf *b;
	initlock(&bcache.lock, "bcache");
	for (b = bcache.buf; b < bcache.buf + NBUF; b ++) {
		b->prev = b;
		initsleeplock(&b->lock, "buffer");
	}
	for (int i = 0; i < NBUCKET; i ++)
		initlock(&bcache.hash_table[i].lock, "bcache");

	// 将所有的buf都链接到第0个bucket的头结点head上
	for (b = bcache.buf; b < bcache.buf + NBUF; b ++) {
		b->next = bcache.hash_table[0].head.next;
		bcache.hash_table[0].head.next = b;
	}
	/*
		 struct buf *b;

		 initlock(&bcache.lock, "bcache");

	// Create linked list of buffers
	bcache.head.prev = &bcache.head;
	bcache.head.next = &bcache.head;
	for(b = bcache.buf; b < bcache.buf+NBUF; b++){
	b->next = bcache.head.next;
	b->prev = &bcache.head;
	initsleeplock(&b->lock, "buffer");
	bcache.head.next->prev = b;
	bcache.head.next = b;
	}
	*/
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
	static struct buf*
bget(uint dev, uint blockno)
{
	struct buf *b;
	int addr = blockno % NBUCKET;

	acquire(&bcache.hash_table[addr].lock); // 对当前bucket上锁
	b = &bcache.hash_table[addr].head.next;
	while (b != (void *)0) {
		if (b->dev == dev && b->blockno == blockno) {
			b->refcnt ++;
			release(&bcache.hash_table[addr].lock);
			acquiresleep(&b->lock);
			return b;
		}
	}

	// 整条链都没有找到像样的，因此找这条链中找refcnt==0的结点
	// 原则：LRU(先不考虑将ticks最小的结点放在头部)
	int LRU = 65536;
	int found = 0;
	struct buf *tmp = &bcache.hash_table[addr].head.next;
	while (tmp != (void *)0) {
		if (tmp->refcnt == 0 && tmp->ticks < LRU) {
			LRU = tmp->ticks;
			found = 1;
			b = tmp;
		}
	}
	if (found) {
		b->dev = dev;
		b->blockno = blockno;
		b->valid = 0;
		b->refcnt = 1;
		release(&bcache.hash_table[addr].lock);
		acquiresleep(&b->lock);
		return b;
	}

	// 这条链中不存在，要在其他链中偷一个
	// 其他链中不可能存在dev和blockno都相等的结点
	// 所以只需要找refcnt==0，且满足LRU
	// 这种情况似乎可以和上面的合并
	for (int i = 0; i < NBUCKET; i ++) {
		if (i == addr)
			continue;
	}
	// 由于链表中元素是按照LRU从小到大存储的，每条链只找
	for (int i = 0; i < NBUCKET; i ++) {
		if (i == addr)
			continue;
		struct buf *now = &bcache.hash_table[i].head.next;
		while (now != (void *)0) {
			if (b->dev)
		}
	}
	/*
		 struct buf *b;

	// acquire(&bcache.lock);

	// Is the block already cached?
	for(b = bcache.head.next; b != &bcache.head; b = b->next){
	if(b->dev == dev && b->blockno == blockno){
	b->refcnt++;
	// release(&bcache.lock);
	acquiresleep(&b->lock);
	return b;
	}
	}

	// Not cached.
	// Recycle the least recently used (LRU) unused buffer.
	for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
	if(b->refcnt == 0) {
	b->dev = dev;
	b->blockno = blockno;
	b->valid = 0;
	b->refcnt = 1;
	// release(&bcache.lock);
	acquiresleep(&b->lock);
	return b;
	}
	}
	panic("bget: no buffers");
	*/
}

// Return a locked buf with the contents of the indicated block.
	struct buf*
bread(uint dev, uint blockno)
{
	struct buf *b;

	b = bget(dev, blockno);
	if(!b->valid) {
		virtio_disk_rw(b, 0);
		b->valid = 1;
	}
	return b;
}

// Write b's contents to disk.  Must be locked.
	void
bwrite(struct buf *b)
{
	if(!holdingsleep(&b->lock))
		panic("bwrite");
	virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
	void
brelse(struct buf *b)
{
	if(!holdingsleep(&b->lock))
		panic("brelse");

	releasesleep(&b->lock);

	acquire(&bcache.lock);
	b->refcnt--;
	if (b->refcnt == 0) {
		// no one is waiting for it.
		b->next->prev = b->prev;
		b->prev->next = b->next;
		b->next = bcache.head.next;
		b->prev = &bcache.head;
		bcache.head.next->prev = b;
		bcache.head.next = b;
	}

	release(&bcache.lock);
}

void
bpin(struct buf *b) {
	acquire(&bcache.lock);
	b->refcnt++;
	release(&bcache.lock);
}

void
bunpin(struct buf *b) {
	acquire(&bcache.lock);
	b->refcnt--;
	release(&bcache.lock);
}


