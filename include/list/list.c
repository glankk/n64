/* list.c
 * -glank
 */

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "list.h"

struct list_element_header
{
  struct list_element_header *prev, *next;
};

#define MAX_ALIGN(N) (((N) + _Alignof(max_align_t) - 1) & \
                      ~(_Alignof(max_align_t) - 1))
#define ELEMENT_HEADER_SIZE (MAX_ALIGN(sizeof(struct list_element_header)))


#ifdef __cplusplus
extern "C"
{
#endif

void list_init(struct list *list, size_t element_size)
{
  list->element_size = element_size;
  list->size = 0;
  list->first = 0;
  list->last = 0;
}

void *list_prev(const void *element)
{
  const struct list_element_header *header =
      (const struct list_element_header*)((const char*)element -
                                          ELEMENT_HEADER_SIZE);
  if (header->prev)
    return (char*)header->prev + ELEMENT_HEADER_SIZE;
  else
    return 0;
}

void *list_next(const void *element)
{
  const struct list_element_header *header =
      (const struct list_element_header*)((const char*)element -
                                          ELEMENT_HEADER_SIZE);
  if (header->next)
    return (char*)header->next + ELEMENT_HEADER_SIZE;
  else
    return 0;
}

void *list_prev_in(const struct list *list, const void *element)
{
  if (element == 0)
    return list->last;
  return list_prev(element);
}

void *list_next_in(const struct list *list, const void *element)
{
  if (element == 0)
    return list->first;
  return list_next(element);
}

void *list_at(const struct list *list, size_t position)
{
  if (list->size == 0)
    return 0;
  struct list_element_header *header = (struct list_element_header*)
                                       ((char*)list->first -
                                        ELEMENT_HEADER_SIZE);
  for (size_t i = 0; i < position; ++i) {
    if (header->next)
      header = header->next;
    else
      return 0;
  }
  return (char*)header + ELEMENT_HEADER_SIZE;
}

void *list_insert_size(struct list *list, void *element, size_t size,
                       const void *data)
{
  struct list_element_header *new_header = (struct list_element_header*)
                                           malloc(ELEMENT_HEADER_SIZE + size);
  if (!new_header)
    return 0;
  void *new_data = (char*)new_header + ELEMENT_HEADER_SIZE;
  if (data)
    memcpy(new_data, data, size);
  if (element) {
    struct list_element_header *header = (struct list_element_header*)
                                         ((char*)element -
                                          ELEMENT_HEADER_SIZE);
    new_header->prev = header->prev;
    new_header->next = header;
    if (header->prev)
      header->prev->next = new_header;
    header->prev = new_header;
    if (element == list->first)
      list->first = new_data;
  }
  else {
    if (list->size == 0) {
      new_header->prev = 0;
      list->first = new_data;
    }
    else {
      new_header->prev = (struct list_element_header*)((char*)list->last -
                                                       ELEMENT_HEADER_SIZE);
      new_header->prev->next = new_header;
    }
    new_header->next = 0;
    list->last = new_data;
  }
  ++list->size;
  return new_data;
}

void *list_insert(struct list *list, void *element, const void *data)
{
  return list_insert_size(list, element, list->element_size, data);
}

void *list_insert_after_size(struct list *list, void *element, size_t size,
                             const void *data)
{
  return list_insert_size(list, list_next_in(list, element), size, data);
}

void *list_insert_after(struct list *list, void *element, const void *data)
{
  return list_insert_size(list, list_next_in(list, element),
                          list->element_size, data);
}

void *list_push_back_size(struct list *list, size_t size, const void *data)
{
  return list_insert_size(list, 0, size, data);
}

void *list_push_back(struct list *list, const void *data)
{
  return list_insert_size(list, 0, list->element_size, data);
}

void list_erase(struct list *list, void *element)
{
  struct list_element_header *header = (struct list_element_header*)
                                       ((char*)element - ELEMENT_HEADER_SIZE);
  if (element == list->first) {
    if (header->next)
      list->first = (void*)((char*)header->next + ELEMENT_HEADER_SIZE);
    else
      list->first = 0;
  }
  if (element == list->last) {
    if (header->prev)
      list->last = (void*)((char*)header->prev + ELEMENT_HEADER_SIZE);
    else
      list->last = 0;
  }
  if (header->prev)
    header->prev->next = header->next;
  if (header->next)
    header->next->prev = header->prev;
  free(header);
  --list->size;
}

void list_destroy(struct list *list)
{
  struct list_element_header *header = 0;
  if (list->first)
    header = (struct list_element_header*)((char*)list->first -
                                           ELEMENT_HEADER_SIZE);
  for (; header; header = header->next)
    free(header);
}

#ifdef __cplusplus
}
#endif
