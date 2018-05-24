/**
 * cmap.h
 */

#ifndef _cmap_h
#define _cmap_h

/**
 * cmap type
 */
typedef struct cmap_implementation cmap;

/**
 * cmap_cleanup_fn
 */
typedef void (* cmap_cleanup_fn)(void * val_addr);

/**
 * cmap_create
 *
 * Creates a new empty cmap and returns a pointer to it.
 * @param val_size the size in bytes of values to be stored
 * @param cap_hint expected capacity of the cmap; capacity will stay fixed
 * @param cleanup_fn client callback used for value deallocation; pass NULL if not needed
 * @return pointer to cmap
 */
cmap * cmap_create(size_t val_size, int cap_hint, cmap_cleanup_fn cleanup_fn);

/**
 * cmap_dispose
 *
 * Disposes of cmap and deallocates its memory. Calls cmap_create cleanup function for each value.
 * @param cmap_ptr pointer to cmap
 */ 
void cmap_dispose(cmap * cmap_ptr);

/**
 * cmap_count
 *
 * Returns the number of entries currently stored in cmap.
 * @param cmap_ptr pointer to cmap
 */
int cmap_count(cmap * cmap_ptr);

/**
 * cmap_first
 *
 * Returns the key of the first blob stored in the cmap.
 * @param cmap_ptr pointer to cmap
 * @return key of first blob or NULL if cmap is empty
 */
char * cmap_first(cmap * cmap_ptr);

/**
 * cmap_get
 *
 * Searches the cmap for an entry with the given key. If found, a pointer to its associated value is returned. If no entry is found, the NULL is returned.
 * @param cmap_ptr pointer to cmap
 * @param key key for which to search
 * @return address of associated value
 */
void * cmap_get(const cmap * cmap_ptr, const char * key);

/**
 * cmap_next
 *
 * Returns the key of the entry(blob) following that of the supplied key.
 * @param cmap_ptr pointer to cmap
 * @param key key of previous blob
 * @return key of next entry (blob) in the cmap or NULL if last entry(blob)
 */
char * cmap_next(cmap * cmap_ptr, const char * key);

/**
 * cmap_put
 *
 * Associates the given key with a new value in cmap. If there is an existing value for the key, it is replaced with the new value. Before being overwritten, the cleanup_fn is called on the old value.
 * @param cmap_ptr pointer to cmap
 * @param key cstring key to add to cmap
 * @param val_addr address of value to associate with key
 */
void cmap_put(cmap * cmap_ptr, const char * key, const void * val_addr);

/**
 * cmap_remove
 *
 * Searches the cmap for an entry with the given key, and if found, removes the entry. If no key is found, no changes are made. Before a value is removed, the cleanup function is called on the value.
 * @param cmap_ptr pointer to cmap
 * @param key key for which to search
 */
void cmap_remove(cmap * cmap_ptr, const char * key);

#endif

