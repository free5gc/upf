#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _ListHead {
	struct _ListHead *next;
    struct _ListHead *prev;
}ListHead;

#define ListHeadInit(__nameHeadPtr) do { \
    (__nameHeadPtr)->next = __nameHeadPtr; \
    (__nameHeadPtr)->prev = __nameHeadPtr; \
} while (0)

#define ListFirst(__namePtr) ((void *)((__namePtr)->next))

#define ListPrev(__nodePtr) ((void *)(((ListHead *)(__nodePtr))->prev))

#define ListNext(__nodePtr) ((void *)(((ListHead *)(__nodePtr))->next))

#define ListIsEmpty(__namePtr) ((__namePtr)->next == NULL)

#define __ListInsert(__newPtr,__nameHeadPtr,__nameHeadNextPtr) do { \
    ((ListHead *) __nameHeadNextPtr)->prev = (ListHead *) __newPtr; \
    ((ListHead *) __newPtr)->next = (ListHead *) __nameHeadNextPtr; \
    ((ListHead *) __newPtr)->prev = (ListHead *) __nameHeadPtr; \
    ((ListHead *) __nameHeadPtr)->next = (ListHead *) __newPtr; \
} while(0)

#define ListInsert(__newPtr, __namePtr) do { \
    __ListInsert(__newPtr, __namePtr, ((ListHead *) __namePtr)->next); \
} while(0)

#define ListInsertTail(__newPtr, __namePtr) do { \
    __ListInsert(__newPtr, ((ListHead *) __namePtr)->next, __namePtr); \
} while(0)


/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
#define __ListRemove(__namePrevPtr, __nameNextPtr) do { \
    ((ListHead *) __nameNextPtr)->prev = (ListHead *) __namePrevPtr; \
    ((ListHead *) __namePrevPtr)->next = (ListHead *) __nameNextPtr; \
} while(0)

#define __ListRemoveEntry(entry) do { \
   __ListRemove( ((ListHead *) entry)->prev, ((ListHead *) entry)->next); \
} while(0)

typedef int (*ListHeadCompare)(ListHead *pnode1, ListHead *pnode2);
/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
#define ListRemove(entry) do { \
	__ListRemoveEntry((ListHead *) entry); \
	((ListHead *) entry)->next = NULL; \
	((ListHead *) entry)->prev = NULL; \
} while(0)


typedef int (*ListHeadCompare)(ListHead *pnode1, ListHead *pnode2);

#define ListInsertSorted(__newPtr, __namePtr, __cmpCallback) do { \
    ListHeadCompare callbackPtr = (ListHeadCompare)__cmpCallback; \
    ListHead *iter = ((ListHead *) __namePtr); \
    while (iter->next) { \
        if (iter->next == __namePtr ) { \
            break; \
        } \
        if ((*callbackPtr)((ListHead *)(__newPtr), ListNext(iter)) < 0) { \
            break; \
        } \
        iter = ListNext(iter); \
    } \
    ListInsert( __newPtr, iter); \
} while (0)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @__nameNextPtr:   	the &struct list_head to use as a loop cursor.
 * @__nameNextPtrNext:	another &struct list_head to use as temporary storage
 * @__namePtr:        	the head for your list.
 */

#define ListForEachSafe(__nameNextPtr, __nameNextPtrNext, __namePtr) \
	for ( __nameNextPtr = ListFirst(__namePtr), __nameNextPtrNext = ListNext(__nameNextPtr); \
          (void *) __nameNextPtr != (void *)__namePtr; \
		  (__nameNextPtr) = __nameNextPtrNext, __nameNextPtrNext = ListNext(__nameNextPtr) \
        )

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIST_H__ */