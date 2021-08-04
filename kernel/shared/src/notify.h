//notify.h
//Sloppy thread wakeups
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef NOTIFY_H
#define NOTIFY_H

#include <sys/types.h>

//Data about one thread waiting for one notification.
typedef struct notify_dst_s
{
	id_t tid; //Thread to unblock
	struct notify_dst_s *next; //Link in list
} notify_dst_t;

//Source of notifications.
//Does not provide locking. Should be in a structure that is properly locked.
typedef struct notify_src_s
{
	notify_dst_t *dsts; //All destinations to unblock
} notify_src_t;

//Sets up the current thread to receive notifications from the given source
void notify_add(notify_src_t *src, notify_dst_t *dst);

//Removes the current thread from the given notification source
void notify_remove(notify_src_t *src, notify_dst_t *dst);

//Waits for the calling thread to be notified at least once since the last call.
//Returns 0 on success or a negative error number (primarily, -EINTR if a signal was caught).
int notify_wait(void);

//Notifies all threads waiting on the given notify source.
void notify_send(notify_src_t *src);

#endif //NOTIFY_H
