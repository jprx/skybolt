#pragma once
#include <types.h>

// Much of this is directly inspired by or copied from the Linux kernel.
// see include/linux/list.h and include/linux/container_of.h

struct listlink {
  struct listlink *next, *prev;
};

#define container_of(ptr, type, member) ({ \
    (type*)((u8*)ptr - offsetof(type,member)); })

static inline void list_init(struct listlink *l) {
  l->next = l;
  l->prev = l;
}

static inline void list_add(struct listlink *new_element, struct listlink *head) {
  head->next->prev = new_element;
  new_element->next = head->next;
  new_element->prev = head;
  head->next = new_element;
}

static inline void list_del(struct listlink *e) {
  e->next->prev = e->prev;
  e->prev->next = e->next;
  e->next = NULL;
  e->prev = NULL;
}

#define list_for_each(pos, head) \
  for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

#define list_for_each_from(start, pos, head) \
  for ((pos) = (start); (pos) != (head); (pos) = (pos)->next)

#define list_for_each_safe(pos, tmp, head) \
  for ((pos) = (head)->next, (tmp) = (pos)->next; \
       (pos) != (head);                           \
       (pos) = (tmp), (tmp) = (pos)->next)
