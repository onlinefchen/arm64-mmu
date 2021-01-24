#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

typedef struct spinlock {
        volatile uint32_t lock;
} spinlock_t;

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
#endif
