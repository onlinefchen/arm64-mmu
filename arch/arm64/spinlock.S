	.globl	spin_lock
	.globl	spin_unlock

/*
 * Acquire lock using load-/store-exclusive instruction pair.
 *
 * void spin_lock(spinlock_t *lock);
 */
spin_lock:
	mov	w2, #1
	sevl
l1:	wfe
l2:	ldaxr	w1, [x0]
	cbnz	w1, l1
	stxr	w1, w2, [x0]
	cbnz	w1, l2
	ret

/*
 * Release lock previously acquired by spin_lock.
 *
 * Use store-release to unconditionally clear the spinlock variable.
 * Store operation generates an event to all cores waiting in WFE
 * when address is monitored by the global monitor.
 *
 * void spin_unlock(spinlock_t *lock);
 */
spin_unlock:
	stlr	wzr, [x0]
	ret
