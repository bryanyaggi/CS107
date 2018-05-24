/**
 * cvector.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <search.h>

#include "cvector.h"

#define DEFAULT_CAP 10

struct cvec_implementation
{
    size_t elem_size;
    int cap_hint;
    //void (* cleanup_fn)(void * elem_addr);
    cvec_cleanup_fn cleanup_fn;
    int capacity;
    int num_elems;
    void * elems;
};

cvector * cvec_create(size_t elem_size, int cap_hint, cvec_cleanup_fn cleanup_fn)
{
    assert(elem_size > 0);
    assert(cap_hint >= 0);

    cvector * cvec_ptr = malloc(sizeof(cvector));
    cvec_ptr->elem_size = elem_size;
    if (cap_hint == 0)
    {
        cvec_ptr->cap_hint = DEFAULT_CAP;
    }
    else
    {
        cvec_ptr->cap_hint = cap_hint;
    }
    cvec_ptr->capacity = cvec_ptr->cap_hint;
    cvec_ptr->cleanup_fn = cleanup_fn;
    cvec_ptr->num_elems = 0;
    cvec_ptr->elems = malloc(cvec_ptr->elem_size * cvec_ptr->capacity);

    assert(cvec_ptr->elems != NULL);

    return cvec_ptr;
}

void cvec_dispose(cvector * cvec_ptr)
{
    if (cvec_ptr->cleanup_fn != NULL)
    {
        for (int i = 0; i < cvec_ptr->num_elems; i++)
        {
            cvec_ptr->cleanup_fn(cvec_nth(cvec_ptr, i));
        }
    }

    free(cvec_ptr->elems);
    free(cvec_ptr);
}

/**
 * grow
 *
 * Increases the capacity of cvec_ptr->elems by cap_hint.
 * @param cvec_ptr pointer to cvector
 */
static void grow(cvector * cvec_ptr)
{
    cvec_ptr->capacity = 2 * cvec_ptr->capacity;
    cvec_ptr->elems = realloc(cvec_ptr->elems, cvec_ptr->capacity * cvec_ptr->elem_size);
   
    assert(cvec_ptr->elems != NULL);
}

void cvec_append(cvector * cvec_ptr, const void * elem_addr)
{
    if (cvec_ptr->num_elems == cvec_ptr->capacity)
    {
        grow(cvec_ptr);
    }

    cvec_ptr->num_elems++;
    void * new_elem_addr = cvec_nth(cvec_ptr, cvec_ptr->num_elems - 1);

    memcpy(new_elem_addr, elem_addr, cvec_ptr->elem_size);
}

int cvec_count(const cvector * cvec_ptr)
{
    return cvec_ptr->num_elems;
}

void * cvec_first(const cvector * cvec_ptr)
{
    return cvec_nth(cvec_ptr, 0);
}

/**
 * shift_elems
 *
 * Shifts elements in memory by one elem_size starting at specified index. Increments or decrements cvec_ptr->num_elems accordingly.
 * @param cvec_ptr pointer to cvector
 * @param index where to start shifting
 * @param up true to shift elements up, false to shift elements down
 */
static void shift_elems(cvector * cvec_ptr, int index, bool up)
{
    void * src = cvec_nth(cvec_ptr, index);
    void * dest;
    size_t num_bytes = (char *) cvec_nth(cvec_ptr, cvec_ptr->num_elems - 1) + cvec_ptr->elem_size - (char *) cvec_nth(cvec_ptr, index);

    if (up)
    {
        cvec_ptr->num_elems++;
        dest = cvec_nth(cvec_ptr, index + 1);
    }
    else
    {
        cvec_ptr->num_elems--;
        dest = cvec_nth(cvec_ptr, index - 1);
    }
    
    memmove(dest, src, num_bytes);
}


void cvec_insert(cvector * cvec_ptr, const void * elem_addr, int index)
{
    assert(index >= 0 && index <= cvec_ptr->num_elems);

    if (cvec_ptr->num_elems == cvec_ptr->capacity)
    {
        grow(cvec_ptr);
    }

    if (index == cvec_ptr->num_elems)
    {
        cvec_append(cvec_ptr, elem_addr);
    }
    else
    {
        shift_elems(cvec_ptr, index, true);
        void * new_elem_addr = cvec_nth(cvec_ptr, index);

        memcpy(new_elem_addr, elem_addr, cvec_ptr->elem_size);
    }
}

/**
 * get_index
 *
 * Returns the cvector index of an element address.
 * @param cvec_ptr pointer to cvector
 * @param elem_addr address of element in cvector
 */
static int get_index(const cvector * cvec_ptr, const void * elem_addr)
{
    return ((char *) elem_addr - (char *) cvec_first(cvec_ptr)) / cvec_ptr->elem_size;
}

void * cvec_next(const cvector * cvec_ptr, void * elem_addr)
{
    assert(elem_addr >= cvec_first(cvec_ptr));
    assert(elem_addr <= cvec_nth(cvec_ptr, cvec_ptr->num_elems - 1));
    assert(((char *) elem_addr - (char *) cvec_first(cvec_ptr)) % cvec_ptr->elem_size == 0);

    int index = get_index(cvec_ptr, elem_addr) + 1;

    if (index > cvec_ptr->num_elems - 1)
    {
        return NULL;
    }
    else
    {
        return cvec_nth(cvec_ptr, index);
    }
}

static void * get_elem_addr(const cvector * cvec_ptr, int index)
{
    return (char *) cvec_ptr->elems + index * cvec_ptr->elem_size;
}

void * cvec_nth(const cvector * cvec_ptr, int index)
{
    assert(index >= 0 && index < cvec_ptr->num_elems);

    return get_elem_addr(cvec_ptr, index);
}

void cvec_remove(cvector * cvec_ptr, int index)
{
    assert(index >= 0 && index <= cvec_ptr->num_elems - 1);

    if (cvec_ptr->cleanup_fn != NULL)
    {
        cvec_ptr->cleanup_fn(cvec_nth(cvec_ptr, index));
    }

    if (index < cvec_ptr->num_elems - 1)
    {
        shift_elems(cvec_ptr, index + 1, false);
    }
    else
    {
        cvec_ptr->num_elems--;
    }
}

void cvec_replace(cvector * cvec_ptr, const void * elem_addr, int index)
{
    assert(index >= 0 && index <= cvec_ptr->num_elems - 1);

    if (cvec_ptr->cleanup_fn != NULL)
    {
        cvec_ptr->cleanup_fn(cvec_nth(cvec_ptr, index));
    }

    memcpy(cvec_nth(cvec_ptr, index), elem_addr, cvec_ptr->elem_size);
}

void cvec_sort(cvector * cvec_ptr, cvec_cmp_fn cmp_fn)
{
    qsort(cvec_ptr->elems, cvec_ptr->num_elems, cvec_ptr->elem_size, cmp_fn);
}

int cvec_search(cvector * cvec_ptr, const void * key, cvec_cmp_fn cmp_fn, int start_index, bool sorted)
{
    assert(start_index >= 0 && start_index < cvec_ptr->num_elems);

    void * base = cvec_nth(cvec_ptr, start_index);
    void * elem_addr = NULL;
    size_t num = cvec_ptr->num_elems - start_index;

    if (sorted)
    {
        elem_addr = bsearch(key, base, num, cvec_ptr->elem_size, cmp_fn);
    }
    else
    {
        elem_addr = lfind(key, base, &num, cvec_ptr->elem_size, cmp_fn);
    }

    if (elem_addr == NULL)
    {
        return -1;
    }
    else
    {
        return get_index(cvec_ptr, elem_addr);
    }
}

