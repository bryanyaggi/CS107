/*
 * mm_sfl.c
 *
 * Segregated free list implementation.
 *  - 9 size classes for storing free blocks
 *      index   size [bytes]
 *      0       24      -   32  (2^5)
 *      1       33      -   64  (2^6)
 *      2       65      -   128 (2^7)
 *      3       129     -   256 (2^8)
 *      4       257     -   512 (2^9)
 *      5       513     -   1024(2^10)
 *      6       1025    -   2048(2^11)
 *      7       2049    -   4096(2^12)
 *      8       4097    -   (inf)
 *  - First fit scheme in size class free list; block splitting implemented
 *  - Coalesce after freeing block and after extending heap
 *  - 4096 (2^12) byte minimum heap extension
 *  - Each block has boundary tags
 *  - Each free block has pointers to previous and next free block of same size class in header
 *
 * Initial inspiration from B&O Section 9.9.14.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

#define NUM_SIZE_CLASSES 9 // number of free block size classes
#define MIN_BLK_SIZE (3 * DWORD_SIZE)
#define MAX_BLK_SIZE INT_MAX
#define HEAP_EXT_SIZE (0x1 << 12) // size by which heap is extended

#pragma pack(1) // pack structs

/*
 * btag
 * struct for boundary tags
 *
 * Includes boundary tag.
 */
struct btag
{
    unsigned int blk_info; // 4 bytes
};

typedef struct btag btag;

/*
 * free_hdr
 * struct for free block headers
 *
 * Includes boundary tag and pointers to previous and next free blocks.
 */
struct free_hdr
{
    btag tag; // 4 bytes
    struct free_hdr * prev_hdr_addr; // 8 bytes
    struct free_hdr * next_hdr_addr; // 8 bytes
};

typedef struct free_hdr free_hdr;

/*
 * free_lists
 *
 * Array of linked lists to store free blocks by size category.
 * 
 *  index   size [bytes]
 *  0       24      -   32  (2^5)
 *  1       33      -   64  (2^6)
 *  2       65      -   128 (2^7)
 *  3       129     -   256 (2^8)
 *  4       257     -   512 (2^9)
 *  5       513     -   1024(2^10)
 *  6       1025    -   2048(2^11)
 *  7       2049    -   4096(2^12)
 *  8       4097    -   (inf)
 */
static free_hdr * free_lists[NUM_SIZE_CLASSES];

static void * heap_ptr; // pointer to initial block

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
 * make_btag
 *
 * Creates a boundary tag given the size and allocation status of block.
 * @param size size of block (including header and footer)
 * @param alloc allocation status of block (1 indicates allocated)
 * @return boundary tag
 */
static inline btag make_btag(size_t size, unsigned char alloc)
{
    btag new_btag;
    new_btag.blk_info = (size | alloc);
    return new_btag;
}

/*
 * make_free_hdr
 *
 * Creates a free block header given the boundary tag, previous free block, and next free block.
 * @param new_btag boundary tag for block
 * @param prev_hdr_addr address of previous free block
 * @param next_hdr_addr address of next free block
 * @return free block header
 */
static inline free_hdr make_free_hdr(btag new_btag, free_hdr * prev_hdr_addr, free_hdr * next_hdr_addr)
{
    free_hdr new_free_hdr;
    new_free_hdr.tag = new_btag;
    new_free_hdr.prev_hdr_addr = prev_hdr_addr;
    new_free_hdr.next_hdr_addr = next_hdr_addr;
    return new_free_hdr;
}

/*
 * put_val
 *
 * Write value at memory address.
 * @param mem_addr memory address
 * @param val value to write at address
 */
static inline void put_val(void * mem_addr, unsigned int val)
{
    * (unsigned int *) mem_addr = val;
}

/*
 * put_btag
 *
 * Write boundary tag at memory address.
 * @param mem_addr memory address
 * @param new_btag boundary tag
 */
static inline void put_btag(void * mem_addr, btag new_btag)
{
    * (btag *) mem_addr = new_btag;
}

/*
 * put_free_hdr
 *
 * Write free header at memory address.
 * @param mem_addr memory address
 * @param new_free_hdr free header
 */
static inline void put_free_hdr(void * mem_addr, free_hdr new_free_hdr)
{
    * (free_hdr *) mem_addr = new_free_hdr;
}

/*
 * get_size
 *
 * Extract size from boundary tag.
 * @param tag boundary tag
 * @return size of block
 */
static inline size_t get_size(btag * tag)
{
    return (tag->blk_info & ~0x7);
}

/*
 * get_alloc
 *
 * Extract size from boundary tag.
 * @param tag boundary tag
 * @return allocation status of block (1 indicates allocated)
 */
static inline size_t get_alloc(btag * tag)
{
    return (tag->blk_info & 0x1);
}

/*
 * get_hdr_addr
 *
 * Get block header address given its footer address.
 * @param ftr_addr block footer address
 * @return address of header
 */
static inline void * get_hdr_addr(btag * ftr_addr)
{
    return ((char *) ftr_addr - (get_size(ftr_addr) - WORD_SIZE));
}

/*
 * get_ftr_addr
 *
 * Get block footer address given its header address.
 * @param hdr_addr block header address
 * @return address of footer
 */
static inline void * get_ftr_addr(btag * hdr_addr)
{
    return ((char *) hdr_addr + (get_size(hdr_addr) - WORD_SIZE));
}

/*
 * get_prev_hdr_addr
 *
 * Get previous block header address given header address of block.
 * @param hdr_addr address of block header
 * @return address of previous block header
 */
static inline void * get_prev_hdr_addr(btag * hdr_addr)
{
    return get_hdr_addr((btag *) ((char *) hdr_addr - WORD_SIZE));
}

/*
 * get_next_hdr_addr
 *
 * Get next block header address given header address of block.
 * @param hdr_addr header address of block
 * @return address of next block header
 */
static inline void * get_next_hdr_addr(btag * hdr_addr)
{
    return ((char *) hdr_addr + get_size(hdr_addr));
}

/*
 * get_free_lists_index
 *
 * Returns the appropriate free_lists index for a given size.
 * @param size size of block
 * @return index
 */
static unsigned char get_free_lists_index(size_t size)
{
    size = (size - 1) >> 5;

    unsigned char index = 0;
    while ((size != 0x0) && (index < NUM_SIZE_CLASSES - 1))
    {
        size = size >> 1;
        index++;
    }

    return index;
}

/*
 * add_to_free_list
 *
 * Add block to beginning of free list. The block does not need a header or footer for this function.
 * @param mem_addr address of block
 * @param size size of block
 */
static void add_to_free_list(void * mem_addr, size_t size)
{
    btag new_btag = make_btag(size, 0);
    unsigned char index = get_free_lists_index(size);
    free_hdr new_free_hdr = make_free_hdr(new_btag, NULL, free_lists[index]);

    put_free_hdr(mem_addr, new_free_hdr); // rewrite header
    put_btag(get_ftr_addr(mem_addr), new_btag); // rewrite footer

    free_lists[index] = mem_addr; // insert at beginning of free list

    // Update prev_hdr_addr of next block
    if (free_lists[index]->next_hdr_addr != NULL)
    {
        (free_lists[index]->next_hdr_addr)->prev_hdr_addr = free_lists[index];
    }
}

/*
 * remove_from_free_list
 *
 * Remove block from free list and updates list.
 * @param blk_addr address of block header
 */
static void remove_from_free_list(free_hdr * blk_addr)
{
    if (blk_addr->prev_hdr_addr == NULL)
    // first block in list
    {
        // Update free list pointer
        unsigned char index = get_free_lists_index(get_size(&(blk_addr->tag)));
        free_lists[index] = blk_addr->next_hdr_addr;
    }
    else
    // block not first
    {
        // Update next_hdr_addr of previous block
        (blk_addr->prev_hdr_addr)->next_hdr_addr = blk_addr->next_hdr_addr;
    }
    
    // Update prev_hdr_addr of next block
    if (blk_addr->next_hdr_addr != NULL)
    {
        (blk_addr->next_hdr_addr)->prev_hdr_addr = blk_addr->prev_hdr_addr;
    }
}

/*
 * coalesce
 *
 * Checks neighboring blocks of free block (or block intended to be freed) and combines contiguous free blocks if found.
 * @param blk_addr address of block header
 * @return new block header address
 */
static void * coalesce(btag * blk_addr)
{
    btag * prev_blk_addr = get_prev_hdr_addr(blk_addr);
    btag * next_blk_addr = get_next_hdr_addr(blk_addr);
    size_t prev_alloc = get_alloc(prev_blk_addr);
    size_t next_alloc = get_alloc(next_blk_addr);
    size_t size = get_size(blk_addr);
    btag * new_hdr_addr;
    btag * new_ftr_addr;

    // neighboring blocks are allocated
    if (prev_alloc && next_alloc)
    {
        return blk_addr;
    }
    // previous block allocated, next free
    else if (prev_alloc && !next_alloc)
    {
        remove_from_free_list((free_hdr *) next_blk_addr);
        size += get_size(next_blk_addr);
        new_hdr_addr = blk_addr;
        new_ftr_addr = get_ftr_addr(next_blk_addr);
    }
    // previous block free, next allocated
    else if (!prev_alloc && next_alloc)
    {
        remove_from_free_list((free_hdr *) prev_blk_addr);
        size += get_size(prev_blk_addr);
        new_hdr_addr = prev_blk_addr;
        new_ftr_addr = get_ftr_addr(blk_addr);
    }
    // neighboring blocks are free
    else
    {
        remove_from_free_list((free_hdr *) prev_blk_addr);
        remove_from_free_list((free_hdr *) next_blk_addr);
        size += get_size(prev_blk_addr) + get_size(next_blk_addr);
        new_hdr_addr = prev_blk_addr;
        new_ftr_addr = get_ftr_addr(next_blk_addr);
    }

    // update boundary tags
    put_btag(new_hdr_addr, make_btag(size, 0));
    put_btag(new_ftr_addr, make_btag(size, 0));

    return new_hdr_addr;
}

/*
 * extend_heap
 *
 * Extends the heap by calling mem_sbrk function. The epilogue is updated and a new free block is added to the heap.
 * @param words words by which to extend heap (1 word = 4 bytes)
 * @return address of beginning of new heap memory
 */
static void * extend_heap(size_t words)
{
    char * new_mem;
    size_t size;

    // Allocate an even number of words to maintain alignment.
    if (words % 2)
    {
        words++;
    }

    size = words * WORD_SIZE;

    if ((long) (new_mem = mem_sbrk(size)) == -1)
    {
        return NULL;
    }

    put_btag((char *) new_mem - WORD_SIZE + size, make_btag(0, 1)); // update epilogue

    put_btag((char *) new_mem - WORD_SIZE, make_btag(size, 0)); // add header to new block
    void * blk_addr = coalesce((btag *) ((char *) new_mem - WORD_SIZE));
    size = get_size((btag *) blk_addr);
    add_to_free_list(blk_addr, size); // add block to free list

    return blk_addr;
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
    if ((heap_ptr = mem_sbrk(4 * WORD_SIZE)) == (void *) -1)
    {
        return -1;
    }

    put_val(heap_ptr, 0x0); // alignment padding
    put_btag(heap_ptr + (1 * WORD_SIZE), make_btag(DWORD_SIZE, 1)); // prologue header
    put_btag(heap_ptr + (2 * WORD_SIZE), make_btag(DWORD_SIZE, 1)); // prologue footer
    put_btag(heap_ptr + (3 * WORD_SIZE), make_btag(0, 1)); // epilogue

    heap_ptr += (3 * WORD_SIZE); // point to memory after prologue

    // Initialize free lists
    for (int i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        free_lists[i] = NULL;
    }

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
 * Finds a free block for allocation or extends heap to create one. The block is removed from its free list. Function is intended to be used in conjunction with allocate.
 * @param size size required
 * @return address of free block header
 */
static free_hdr * find_fit(size_t size)
{
    unsigned char index = get_free_lists_index(size);
    free_hdr * blk_addr;

    while (index < NUM_SIZE_CLASSES)
    {
        blk_addr = free_lists[index];
        while (blk_addr != NULL)
        {
            if (get_size(&(blk_addr->tag)) >= size)
            {
                remove_from_free_list(blk_addr);
                return blk_addr;
            }

            blk_addr = blk_addr->next_hdr_addr;
        }

        index++;
    }

    size_t ext_size = max(size, HEAP_EXT_SIZE);
    extend_heap(ext_size / WORD_SIZE);

    return find_fit(size);
}

/*
 * allocate
 *
 * Allocate free block. Intended to be used in conjuction with find_fit.
 * @param blk_addr address of free block header
 * @param size required
 */
static void allocate(free_hdr * blk_addr, size_t size)
{
    size_t blk_size = get_size(&(blk_addr->tag));

    if ((blk_size - size) >= MIN_BLK_SIZE)
    {
        // Split block
        btag new_btag = make_btag(size, 1);
        put_btag(blk_addr, new_btag); // update header
        put_btag(get_ftr_addr((btag *) blk_addr), new_btag); // add new footer

        blk_addr = get_next_hdr_addr((btag *) blk_addr);
        add_to_free_list(blk_addr, blk_size - size); // add fragment to free list
    }
    else
    {
        btag new_btag = make_btag(blk_size, 1);
        put_btag(blk_addr, new_btag); // update header
        put_btag(get_ftr_addr((btag *) blk_addr), new_btag); // update footer
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
    size_t adj_size; // adjusted size to include overhead and satisfy alignment
    
    // Ignore spurious requests
    if ((size == 0) || (size > MAX_BLK_SIZE))
    {
        return NULL;
    }

    // Adjust block size to include overhead and satisfy alignment
    if (size <= 2 * DWORD_SIZE)
    {
        adj_size = MIN_BLK_SIZE;
    }
    else
    {
        adj_size = DWORD_SIZE * ((size + DWORD_SIZE + (DWORD_SIZE - 1)) / DWORD_SIZE);
    }

    free_hdr * blk_addr = find_fit(adj_size);
    allocate(blk_addr, adj_size);
    return (char *) blk_addr + WORD_SIZE; // return address for data storage
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

    void * blk_addr = coalesce((btag *) ((char *) ptr - WORD_SIZE));
    size_t size = get_size((btag *) blk_addr);
    add_to_free_list(blk_addr, size);
}

/*
 * mm_realloc
 *
 * Reallocates memory stored in one block to another (typically of greater capacity). If a size smaller than the original size is given, the memory will be reallocated but not resized.
 * @param old_ptr old memory pointer
 * @param size needed for data storage
 */
void * realloc(void * old_ptr, size_t size)
{
    if (size == 0)
    {
        free(old_ptr);
        return NULL;
    }

    if (old_ptr == NULL)
    {
        return malloc(size);
    }

    size_t old_size = get_size((btag *) ((char *) old_ptr - WORD_SIZE)) - DWORD_SIZE; // size usable for data storage
    size = max(old_size, size); // prevent realloc to smaller block

    void * new_ptr = malloc(size);

    if (!new_ptr)
    {
        return NULL;
    }

    memcpy(new_ptr, old_ptr, old_size); // copy contents to new block
    free(old_ptr); // free old block

    return new_ptr;
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
    void * new_ptr = malloc(bytes);
    memset(new_ptr, 0, bytes);

    return new_ptr;
}

/*
 * mm_checkheap
 *
 * Checks the heap for correctness. Exits when an error is detected.
 * @param verbose level of checking involved (not utilized)
 */
#ifdef DEBUG
void mm_checkheap(int verbose)
{
    // Check prologue
    if (* (unsigned int *) ((char *) heap_ptr - DWORD_SIZE) != 9)
    {
        dbg_printf("Invalid prologue header.\n");
        exit(1);
    }

    if (* (unsigned int *) ((char *) heap_ptr - WORD_SIZE) != 9)
    {
        dbg_printf("Invalid prologue footer.\n");
        exit(1);
    }

    // Check blocks and epilogue
    size_t num_free_blks[NUM_SIZE_CLASSES];

    for (int i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        num_free_blks[i] = 0;
    }

    btag * blk_ptr = (btag *) heap_ptr;

    while (!((get_size(blk_ptr) == 0) && (get_alloc(blk_ptr) == 1))) // stop at epilogue
    {
        size_t size = get_size(blk_ptr);
        size_t alloc = get_alloc(blk_ptr);
        void * ftr_addr = get_ftr_addr(blk_ptr);

        if ((get_size((btag *) ftr_addr) != size) && (get_alloc((btag *) ftr_addr) != alloc))
        {
            dbg_printf("Block header and footer do not agree at block address %p.\n", blk_ptr);
            exit(1);
        }

        if (alloc == 0)
        {
            num_free_blks[get_free_lists_index(size)]++;
        }

        blk_ptr = get_next_hdr_addr(blk_ptr);
    }

    // Check free lists
    unsigned char index = 0;
    while (index < NUM_SIZE_CLASSES)
    {
        free_hdr * free_blk_ptr = free_lists[index];
        size_t blk_count = 0;

        while (free_blk_ptr != NULL)
        {
            blk_count++;
            free_blk_ptr = free_blk_ptr->next_hdr_addr;
        }

        if (blk_count != num_free_blks[index])
        {
            dbg_printf("Free list does not agree with number of free blocks for index %u.\n", index);
            exit(1);
        }

        index++;
    }

    dbg_printf("Heap seems okay.\n");
}
#else
void mm_checkheap(int verbose)
{
    return;
}
#endif

