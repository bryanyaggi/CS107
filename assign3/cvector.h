/**
 * cvector.h
 */

#ifndef _cvector_h
#define _cvector_h

#include <stddef.h>
#include <stdbool.h>

/**
 * cvector type
 */
typedef struct cvec_implementation cvector;

/**
 * cvec_cleanup_fn
 *
 * type for pointer to client-supplied cleanup function
 */
typedef void (* cvec_cleanup_fn)(void * elem_addr);

/**
 * cvec_cmp_fn
 *
 * type for pointer to client-supplied compare function
 */
typedef int (* cvec_cmp_fn)(const void * elem_a_addr, const void * elem_b_addr);

/**
 * cvec_create
 *
 * Creates a new empty cvector and returns a pointer to it.
 * @param elem_size the size in bytes of elements to be stored
 * @param cap_hint expected capactity of cvector; cvector automatically resized if necessary
 * @param cleanup_fn client callback used for element deallocation; pass NULL if not needed
 * @return pointer to new empty cvector
 */
cvector * cvec_create(size_t elem_size, int cap_hint, cvec_cleanup_fn cleanup_fn);

/**
 * cvec_dispose
 *
 * Disposes of cvector and deallocates its memory. Calls cvec_create cleanup function for each element.
 * @param cvec pointer to cvector
 */
void cvec_dispose(cvector * cvec_ptr);

/**
 * cvec_append
 *
 * Appends a new element to the end of the cvector. The new element is copied from the memory pointed to by elem_addr. cvector capacity increased if necessary.
 * @param cvec_ptr pointer to cvector
 * @param elem_addr address of element
 */
void cvec_append(cvector * cvec_ptr, const void * elem_addr);

/**
 * cvec_count
 *
 * Returns the number of elements stored in a cvector.
 * @param cvec_ptr pointer to cvector
 * @param number of elements in cvector
 */
int cvec_count(const cvector * cvec_ptr);

/**
 * cvec_first
 *
 * Returns a pointer to the first element in cvector.
 * @param cvec_ptr pointer to cvector
 * @return pointer to first element in cvector
 */
void * cvec_first(const cvector * cvec_ptr);

/**
 * cvec_insert
 *
 * Inserts a new element into the cvector at the given index. Shifts elements up to make room. cvector capacity increased if necessary.
 * @param cvec_ptr pointer to cvector
 * @param elem_addr address of element to insert
 * @param index index to insert
 */
void cvec_insert(cvector * cvec_ptr, const void * elem_addr, int index);

/**
 * cvec_next
 *
 * Returns a pointer to the next element in cvector.
 * @param cvec_ptr pointer to cvector
 * @param elem_ptr pointer to element in cvector
 * @return pointer to next element in cvector or NULL no next element
 */
void * cvec_next(const cvector * cvec_ptr, void * elem_ptr);

/**
 * cvec_nth
 *
 * Returns a pointer to the element stored at the given index.
 * @param cvec_ptr pointer to cvector
 * @param index index of element
 * @return pointer to the element stored at the given index
 */
void * cvec_nth(const cvector * cvec_ptr, int index);

/**
 * cvec_remove
 *
 * Removes element at given index. Calls cleanup function before removing element.
 * @param cvec_ptr pointer to cvector
 * @param index index of element to remove
 */
void cvec_remove(cvector * cvec_ptr, int index);

/**
 * cvec_replace
 *
 * Overwrites the element at the specified index. Calls cleanup function on element being replaced.
 * @param cvec_ptr pointer to cvector
 * @param elem_addr address of element to add
 * @param index index of element to overwrite
 */
void cvec_replace(cvector * cvec_ptr, const void * elem_addr, int index);

/**
 * cvec_search
 *
 * Searhes a cvector for the given key and returns the index at which it is found.
 * @param cvec_ptr pointer to cvector
 * @param key pointer to value for which to search
 * @param cmp_fn function to compare elements
 * @param start_index index at which to start search
 * @param sorted true if elements are sorted
 * @return index where value was found or -1 if no match
 */
int cvec_search(cvector * cvec_ptr, const void * key, cvec_cmp_fn cmp_fn, int start_index, bool sorted);

/**
 * cvec_sort
 *
 * Sorts a cvector according to client-provided element compare function.
 * @param cvec_ptr pointer to cvector
 * @param cmp_fn function to compare elements
 */
void cvec_sort(cvector * cvec_ptr, cvec_cmp_fn cmp_fn);

#endif

