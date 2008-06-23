/*
 * Copyright (c) 2001-2004 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup sync
 * @{
 */

/**
 * @file
 * @brief	Mutexes.
 */
 
#include <synch/mutex.h>
#include <synch/semaphore.h>
#include <synch/synch.h>
#include <debug.h>

/** Initialize mutex.
 *
 * @param mtx		Mutex.
 * @param type		Type of the mutex.
 */
void mutex_initialize(mutex_t *mtx, mutex_type_t type)
{
	mtx->type = type;
	semaphore_initialize(&mtx->sem, 1);
}

/** Acquire mutex.
 *
 * Timeout mode and non-blocking mode can be requested.
 *
 * @param mtx		Mutex.
 * @param usec		Timeout in microseconds.
 * @param flags		Specify mode of operation.
 *
 * For exact description of possible combinations of
 * usec and flags, see comment for waitq_sleep_timeout().
 *
 * @return		See comment for waitq_sleep_timeout().
 */
int _mutex_lock_timeout(mutex_t *mtx, uint32_t usec, int flags)
{
	int rc;

	if (mtx->type == MUTEX_PASSIVE) {
		rc = _semaphore_down_timeout(&mtx->sem, usec, flags);
	} else {
		ASSERT(mtx->type == MUTEX_ACTIVE);
		ASSERT(usec == SYNCH_NO_TIMEOUT);
		ASSERT(!(flags & SYNCH_FLAGS_INTERRUPTIBLE));
		do {
			rc = semaphore_trydown(&mtx->sem);
		} while (SYNCH_FAILED(rc) &&
		    !(flags & SYNCH_FLAGS_NON_BLOCKING));
	}

	return rc;
}

/** Release mutex.
 *
 * @param mtx		Mutex.
 */
void mutex_unlock(mutex_t *mtx)
{
	semaphore_up(&mtx->sem);
}

/** @}
 */
