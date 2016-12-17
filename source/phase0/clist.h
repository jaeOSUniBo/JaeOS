/*
 *	CLIST.H
 *	Circular List with Tail Pointer Implementation File
 *
 *	Gruppo 28:
 *	Matteo Del Vecchio
 *	Simone Preite
 */

#ifndef CLIST_H
#define CLIST_H

#define container_of(ptr, type, member) ({																					\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);																	\
	(type *)( (char *)__mptr - offsetof(type,member) );																		\
})

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

struct clist {
	struct clist *next;
};

#define CLIST_INIT {NULL}

#define clist_enqueue(elem, clistp, member) ({																				\
	struct clist *oldTail = (clistp)->next;																					\
	(clistp)->next = &elem->member;																							\
	(clistp)->next->next = (oldTail == NULL) ? &elem->member : oldTail->next;												\
	if (oldTail != NULL) oldTail->next = (clistp)->next;																	\
})

#define clist_empty(clistx) (((clistx).next) == NULL)

#define clist_foreach(scan, clistp, member, tmp)																			\
	tmp = NULL; 																											\
	if ((clistp)->next != NULL)																								\
	for (scan = container_of((clistp)->next->next, typeof(*scan), member);													\
		tmp != (clistp)->next; tmp = &((scan)->member), scan = container_of((scan)->member.next, typeof(*scan), member))

#define clist_tail(elem, clistx, member)																					\
	(clistx.next == NULL) ? NULL : container_of(clistx.next, typeof(*elem), member);

#define clist_head(elem, clistx, member)																					\
	(clistx.next == NULL) ? NULL : container_of(clistx.next->next, typeof(*elem), member);

#define clist_push(elem, clistx, member) ({																					\
	struct clist *oldTail = (clistx)->next;																					\
	(clistx)->next = &elem->member;																							\
	(clistx)->next->next = (oldTail == NULL) ? &elem->member : oldTail->next;												\
	if (oldTail != NULL) {																									\
		oldTail->next = (clistx)->next;																						\
		(clistx)->next = oldTail;																							\
	}																														\
})

#define clist_pop(clistp) clist_dequeue(clistp)
#define clist_dequeue(clistp) ({																							\
	if ((clistp)->next == (clistp)->next->next) (clistp)->next = NULL;														\
	else (clistp)->next->next = (clistp)->next->next->next;																	\
})	

#define clist_foreach_all(scan, clistp, member, tmp) ((tmp) == (clistp)->next) 

#define clist_delete(elem, clistp, member) ({																				\
	int retVal = 1;																											\
	if ((clistp)->next != NULL) {																							\
		struct clist *scanner = (clistp)->next;																				\
		while (scanner->next != &elem->member && scanner->next != (clistp)->next) scanner = scanner->next;					\
		/* list with just one element */																					\
		if (scanner == &elem->member && scanner == (clistp)->next) {														\
			scanner->next = NULL;																							\
			scanner = NULL;																									\
			(clistp)->next = NULL;																							\
			retVal = 0;																										\
		}																													\
		/* list with two or more elements and deleting tail */																\
		else if (scanner->next == &elem->member && scanner->next == (clistp)->next) {										\
			scanner->next = scanner->next->next;																			\
			(clistp)->next->next = NULL;																					\
			(clistp)->next = scanner;																						\
			retVal = 0;																										\
		}																													\
		/* list with two or more elements and deleting any one of them but tail */											\
		else if (scanner->next == &elem->member) {																			\
			scanner->next = scanner->next->next;																			\
			retVal = 0;																										\
		}																													\
	}																														\
	retVal;																													\
})

#define clist_foreach_delete(scan, clistp, member, tmp)																		\
	if (tmp) {																												\
		struct clist *delptr = tmp;																							\
		if (&(scan->member) == (clistp)->next) {																			\
			(clistp)->next = delptr;																						\
			(delptr)->next = (scan)->member.next;																			\
		}																													\
		else if (&(scan->member) == (clistp)->next->next) {																	\
			delptr = (clistp)->next;																						\
			(delptr)->next = (scan)->member.next;																			\
		}																													\
		else (delptr)->next = (delptr)->next->next;																			\
	}																														\
	else clist_dequeue(clistp);
							 
#define clist_foreach_add(elem, scan, clistp, member, tmp)																	\
	if (tmp) {																												\
		(elem)->member.next = &((scan)->member);																			\
		((struct clist*)tmp)->next = &((elem)->member);																		\
	}																														\
	else clist_push(elem,clistp,member);

#endif
