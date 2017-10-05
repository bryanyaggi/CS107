/*
 * mm_ifl.c
 *
 * Implicit free list implementation.
 *
 * Based on B&O Section 9.9.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*
 * Macro for debugging
 */
#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...) // no debug printing
#endif

#ifdef DRIVER
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif

#define ALIGNMENT 8 // double-word alignment
#define WORD_SIZE 4 // size of word in bytes
#define DWORD_SIZE 8 // size of double-word in bytes

// maximum heap size is (0x1 << 32) bytes [2^32]
#define HEAP_EXT_SIZE (0x1 << 12) // size by which heap is extended

static void * heaplist_ptr;

/*
 * max
 *
 * Returns the largest of two values.
 * @param a first unsigned int
 * @param b second unsigned int
 * @return largest value
 */
static inline unsigned int max(unsigned int a, unsigned int b)
{
    if (a >= b)
    {
        return a;
    }

    return b;
}

/*
 * pack
 *
 * Perform bitwise or on size and alloc values. Used for packing data in block header and footers.
 * @param size size of block
 * @param alloc 0x1 if allocated, 0x0 if free
 * @return result of bitwise or
 */
static inline unsigned int pack(size_t size, unsigned char alloc)
{
    return (size | alloc);
}

/*
 * put
 *
 * Write value at memory address.
 * @param mem_addr memory address
 * @param val value to write at address
 */
static inline void put(void * mem_addr, unsigned int val)
{
    * (unsigned int *) mem_addr = val;
}

/*
 * get
 *
 * Get value at memory address.
 * @param mem_addr memory address
 * @return value at address
 */
static inline unsigned int get(void * mem_addr)
{
    return * (unsigned int *) mem_addr;
}

/*
 * get_size
 *
 * Read size of block.
 * @param mem_addr memory address
 * @return size of block
 */
static inline unsigned int get_size(void * mem_addr)
{
    return (get(mem_addr) & ~0x7);
}

/*
 * get_alloc
 *
 * Read whether block is allocated (0x0) or free (0x1).
 * @param mem_addr memory address
 * @return value indicating whether block is allocated or free
 */
static inline unsigned int get_alloc(void * mem_addr)
{
    return (get(mem_addr) & 0x1);
}

/*
 * get_hdr_addr
 *
 * Get the header address for a given block pointer.
 * @param blk_ptr block pointer
 * @return header address
 */
static inline void * get_hdr_addr(void * blk_ptr)
{
    return (char *) blk_ptr - WORD_SIZE;
}

/*
 * get_ftr_addr
 *
 * Get the footer address for a given block addr.
 * @param blk_ptr block pointer
 * @return footer address
 */
static inline void * get_ftr_addr(void * blk_ptr)
{
    return ((char *) blk_ptr + get_size(get_hdr_addr(blk_ptr)) - DWORD_SIZE);
}

/*
 * get_next_blk_ptr
 *
 * Get next block pointer for a given block pointer.
 * @param blk_ptr block pointer
 * @return next block pointer
 */
static inline void * get_next_blk_ptr(void * blk_ptr)
{
    return ((char *) blk_ptr + get_size((char *) blk_ptr - WORD_SIZE));
}

/*
 * get_prev_blk_ptr
 *
 * Get previous block pointer for a given block pointer.
 * @param blk_ptr block pointer
 * @return previous block pointer
 */
static inline void * get_prev_blk_ptr(void * blk_ptr)
{
    return ((char *) blk_ptr - get_size((char *) blk_ptr - DWORD_SIZE));
}

/*
 * coalesce
 */
static void * coalesce(void * blk_ptr)
{
    size_t prev_alloc = get_alloc(get_ftr_addr(get_prev_blk_ptr(blk_ptr)));
    size_t next_alloc = get_alloc(get_hdr_addr(get_next_blk_ptr(blk_ptr)));
    size_t size = get_size(get_hdr_addr(blk_ptr));

    // Neighboring blocks are allocated
    if (prev_alloc && next_alloc)
    {
        return blk_ptr;
    }
    // Previous block allocated, next block free
    else if (prev_alloc && !next_alloc)
    {
        size += get_size(get_hdr_addr(get_next_blk_ptr(blk_ptr)));
        put(get_hdr_addr(blk_ptr), pack(size, 0)); // update header
        put(get_ftr_addr(blk_ptr), pack(size, 0)); // update footer
    }
    // Previous block free, next block allocated
    else if (!prev_alloc && next_alloc)
    {
        size += get_size(get_hdr_addr(get_prev_blk_ptr(blk_ptr)));
        put(get_hdr_addr(get_prev_blk_ptr(blk_ptr)), pack(size, 0)); // update previous header
        put(get_ftr_addr(blk_ptr), pack(size, 0)); // update previous footer
        blk_ptr = get_prev_blk_ptr(blk_ptr);
    }
    // Neighboring blocks are free
    else
    {
        size += get_size(get_hdr_addr(get_prev_blk_ptr(blk_ptr))) + get_size(get_hdr_addr(get_next_blk_ptr(blk_ptr)));
        put(get_hdr_addr(get_prev_blk_ptr(blk_ptr)), pack(size, 0)); // update previous header
        put(get_ftr_addr(get_next_blk_ptr(blk_ptr)), pack(size, 0)); // update next footer
    }

    return blk_ptr;
}

static void * extend_heap(size_t words)
{
    char * blk_ptr;
    size_t size;

    // Allocate an even number of words to maintain alignment.
    if (words % 2)
    {
        words++;
    }

    size = words * WORD_SIZE;

    if ((long) (blk_ptr = mem_sbrk(size)) == -1)
    {
        return NULL;
    }

    // Initialize free block header and footer, new epilogue
    put(get_hdr_addr(blk_ptr), pack(size, 0)); // new free block header
    put(get_ftr_addr(blk_ptr), pack(size, 0)); // new free block footer
    put(get_hdr_addr(get_next_blk_ptr(blk_ptr)), pack(0, 1)); // new epilogue

    // Coalesce if the previous block was free.
    return coalesce(blk_ptr);
}

/*
 * mm_init
 *
 * Creates a heap with an initial free block.
 * @return 0 if successful, -1 if unsuccessful
 */
int mm_init(void)
{
    // Create initial empty heap
    if ((heaplist_ptr = mem_sbrk(4 * WORD_SIZE)) == (void *) -1)
    {
        return -1;
    }

    put(heaplist_ptr, 0); // alignment padding
    put(heaplist_ptr + (1 * WORD_SIZE), pack(DWORD_SIZE, 1)); // prologue header
    put(heaplist_ptr + (2 * WORD_SIZE), pack(DWORD_SIZE, 1)); // prologue footer
    put(heaplist_ptr + (3 * WORD_SIZE), pack(0, 1)); // epilogue
    
    heaplist_ptr += (2 * WORD_SIZE);

    // Extend heap.
    if (extend_heap(HEAP_EXT_SIZE / WORD_SIZE) == NULL)
    {
        return -1;
    }

    return 0;
}

/*
 * find_fit
 *
 * Find first block in free list with required capacity.
 * @param adj_size adjusted size of block (including header and footer)
 * @return block pointer of candidate, NULL if no candidates
 */
static void * find_fit(size_t adj_size)
{
    void * blk_ptr;

    for (blk_ptr = heaplist_ptr; get_size(get_hdr_addr(blk_ptr)) > 0; blk_ptr = get_next_blk_ptr(blk_ptr))
    {
        if (!get_alloc(get_hdr_addr(blk_ptr)) && (adj_size <= get_size(get_hdr_addr(blk_ptr))))
        {
            return blk_ptr;
        }
    }

    return NULL; // no fit
}

/*
 * place
 *
 * Allocate free block. Split block if extra space not needed.
 * @param blk_ptr block pointer
 * @param adj_size adjusted size required
 */
static void place(void * blk_ptr, size_t adj_size)
{
    size_t blk_size = get_size(get_hdr_addr(blk_ptr));

    if ((blk_size - adj_size) >= (2 * DWORD_SIZE))
    {
        put(get_hdr_addr(blk_ptr), pack(adj_size, 1)); // header
        put(get_ftr_addr(blk_ptr), pack(adj_size, 1)); // footer

        blk_ptr = get_next_blk_ptr(blk_ptr);

        put(get_hdr_addr(blk_ptr), pack(blk_size - adj_size, 0)); // next header
        put(get_ftr_addr(blk_ptr), pack(blk_size - adj_size, 0)); // next footer
    }
    else
    {
        put(get_hdr_addr(blk_ptr), pack(blk_size, 1)); // header
        put(get_ftr_addr(blk_ptr), pack(blk_size, 1)); // footer
    }
}

/*
 * mm_malloc
 *
 * Allocates a block from the free list.
 * @param size number of bytes requested
 * @return address of allocated block
 */
void * malloc(size_t size)
{
    size_t adj_size; // adjusted block size
    size_t ext_size; // amount to extend heap if no fit
    char * blk_ptr;

    // Ignore spurious requests.
    if (size == 0)
    {
        return NULL;
    }

    // Adjust block size to include overhead and alignment requirements.
    if (size <= DWORD_SIZE)
    {
        adj_size = 2 * DWORD_SIZE;
    }
    else
    {
        adj_size = DWORD_SIZE * ((size + DWORD_SIZE + DWORD_SIZE - 1) / DWORD_SIZE);
    }

    // Search free list for fit
    if ((blk_ptr = find_fit(adj_size)) != NULL)
    {
        place(blk_ptr, adj_size);
        return blk_ptr;
    }

    // No fit found. Get more memory and place the block.
    ext_size = max(adj_size, HEAP_EXT_SIZE);

    if ((blk_ptr = extend_heap(ext_size / WORD_SIZE)) == NULL)
    {
        return NULL;
    }

    place(blk_ptr, adj_size);
    return blk_ptr;
}

/*
 * mm_free
 *
 * Frees a block.
 * @param ptr address of block to free
 */
void free(void * ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    size_t size = get_size(get_hdr_addr(ptr));

    put(get_hdr_addr(ptr), pack(size, 0));
    put(get_ftr_addr(ptr), pack(size, 0));

    coalesce(ptr);
}

/*
 * mm_realloc
 */
void * realloc(void * old_ptr, size_t size)
{
    size_t old_size;
    void * new_blk_ptr;

    if (size == 0)
    {
        free(old_ptr);
        return NULL;
    }

    if (old_ptr == NULL)
    {
        return malloc(size);
    }

    new_blk_ptr = malloc(size);

    if (!new_blk_ptr)
    {
        return NULL;
    }

    old_size = get_size(get_hdr_addr(old_ptr));

    if (size < old_size)
    {
        old_size = size;
    }

    memcpy(new_blk_ptr, old_ptr, old_size);

    free(old_ptr);

    return new_blk_ptr;
}

/*
 * mm_calloc
 *
 * Allocate block and set contents to zero.
 * @param num_elems number of elements
 * @param elem_size size of elements
 * @return block pointer
 */
void * calloc(size_t num_elems, size_t elem_size)
{
    size_t bytes = num_elems * elem_size;
    void * blk_ptr = malloc(bytes);
    memset(blk_ptr, 0, bytes);

    return blk_ptr;
}

/*
 * mm_checkheap
 */
void mm_checkheap(int verbose)
{
    verbose = verbose;
}

