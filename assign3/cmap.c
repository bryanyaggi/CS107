/**
 * cmap.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cmap.h"

#define DEFAULT_CAP 10

struct cmap_implementation
{
    size_t val_size;
    int num_buckets;
    // void (* cleanup_fn)(void * val_addr);
    cmap_cleanup_fn cleanup_fn;
    int num_entries;
    void ** buckets;
};

cmap * cmap_create(size_t val_size, int cap_hint, cmap_cleanup_fn cleanup_fn)
{
    assert(val_size > 0);
    assert(cap_hint >= 0);

    cmap * cmap_ptr = malloc(sizeof(cmap));
    cmap_ptr->val_size = val_size;
    if (cap_hint == 0)
    {
        cmap_ptr->num_buckets = DEFAULT_CAP;
    }
    else
    {
        cmap_ptr->num_buckets = cap_hint;
    }
    cmap_ptr->cleanup_fn = cleanup_fn;
    cmap_ptr->num_entries = 0;
    cmap_ptr->buckets = calloc(cmap_ptr->num_buckets, sizeof(void *));

    assert(cmap_ptr->buckets != NULL);

    return cmap_ptr;
}

/**
 * get_key_addr
 *
 * Returns key address for a given blob pointer.
 * @param blob_ptr pointer to blob
 * @return key address of blob
 */
static char * get_key_addr(void * blob_ptr)
{
    assert(blob_ptr != NULL);
    return (char *) blob_ptr + sizeof(void *);
}

/**
 * get_val_addr
 *
 * Returns value address for a given blob pointer.
 * @param blob_ptr pointer to blob
 * @return value address of blob
 */
static void * get_val_addr(void * blob_ptr)
{
    assert(blob_ptr != NULL);
    char * blob_key_addr = get_key_addr(blob_ptr);
    return blob_key_addr + strlen(blob_key_addr) + 1;
}

/**
 * clean_bucket
 *
 * Deallocates memory allocated in bucket.
 * @param cmap_ptr pointer to cmap
 * @param first_blob_ptr pointer to first blob in bucket
 */
void clean_bucket(cmap * cmap_ptr, void * first_blob_ptr)
{
    void * blob_ptr = first_blob_ptr;

    while (blob_ptr != NULL)
    {
        void * prev_blob_ptr = blob_ptr;
        blob_ptr = * (void **) blob_ptr;

        if (cmap_ptr->cleanup_fn != NULL)
        {
            cmap_ptr->cleanup_fn(get_val_addr(prev_blob_ptr));
        }

        free(prev_blob_ptr);
    }

    first_blob_ptr = NULL;
}

void cmap_dispose(cmap * cmap_ptr)
{
    // Iterate through buckets
    for (int i = 0; i < cmap_ptr->num_buckets; i++)
    {
        if (cmap_ptr->buckets[i] != NULL)
        {
            clean_bucket(cmap_ptr, cmap_ptr->buckets[i]);
        }
    }

    free(cmap_ptr->buckets);
    free(cmap_ptr);
}

int cmap_count(cmap * cmap_ptr)
{
    return cmap_ptr->num_entries;
}

/**
 * hash
 *
 * cstring hashing function
 * @param string cstring
 * @param num_buckets number of buckets
 * @return index of bucket in which to store cstring
 */
static int hash(const char * string, int num_buckets)
{
    const unsigned long MULTIPLIER = 2630849305L;
    unsigned long hashcode = 0;

    for (int i = 0; string[i] != '\0'; i++)
    {
        hashcode = hashcode * MULTIPLIER + string[i];
    }

    return hashcode % num_buckets;
}

/**
 * create_blob
 *
 * Create blob.
 * @param cmap_ptr pointer to cmap
 * @param key key to store
 * @param val_addr address of value to store
 */
static void * create_blob(const cmap * cmap_ptr, const char * key, const void * val_addr)
{
    int num_bytes = sizeof(void *) + strlen(key) + 1 + cmap_ptr->val_size;

    void * blob_ptr = malloc(num_bytes);
    assert(blob_ptr != NULL);

    * (void **) blob_ptr = NULL;
    strcpy(get_key_addr(blob_ptr), key);
    memcpy(get_val_addr(blob_ptr), val_addr, cmap_ptr->val_size);

    return blob_ptr;
}

/**
 * search_bucket
 *
 * Searches bucket linked list for blob containing specified key.
 * @param blob_ptr pointer to blob
 * @param key key for which to search
 * @return pointer to blob containing key
 */
static void * search_bucket(void * blob_ptr, const char * key)
{
    if (blob_ptr == NULL)
    {
        return NULL;
    }

    if (strcmp(get_key_addr(blob_ptr), key) == 0)
    {
        return blob_ptr;
    }
    else
    {
        return search_bucket(* (void **) blob_ptr, key);
    }
}

char * cmap_first(cmap * cmap_ptr)
{
    for (int i = 0; i < cmap_ptr->num_buckets; i++)
    {
        if (cmap_ptr->buckets[i] != NULL)
        {
            return get_key_addr(cmap_ptr->buckets[i]);
        }
    }

    return NULL;
}

void * cmap_get(const cmap * cmap_ptr, const char * key)
{
    assert(key != NULL);

    int index = hash(key, cmap_ptr->num_buckets);

    if (cmap_ptr->buckets[index] != NULL)
    {
        void * match_blob_ptr;
        if ((match_blob_ptr = search_bucket(cmap_ptr->buckets[index], key)) != NULL)
        {
            return get_val_addr(match_blob_ptr);
        }
    }

    return NULL;
}

char * cmap_next(cmap * cmap_ptr, const char * key)
{
    assert(key != NULL);

    int prev_index = hash(key, cmap_ptr->num_buckets);
    void * prev_blob_ptr = search_bucket(cmap_ptr->buckets[prev_index], key);

    assert(prev_blob_ptr != NULL);

    if (* (void **) prev_blob_ptr != NULL)
    {
        return get_key_addr(* (void **) prev_blob_ptr);
    }
    else
    {
        for (int i = prev_index + 1; i < cmap_ptr->num_buckets; i++)
        {
            if (cmap_ptr->buckets[i] != NULL)
            {
                return get_key_addr(cmap_ptr->buckets[i]);
            }
        }

        return NULL;
    }
}

/**
 * insert_blob
 *
 * Insert blob into bucket.
 * @param existing_blob_ptr pointer to blob in bucket
 * @param new_blob_ptr pointer to blob to insert
 */
static void insert_blob(void * existing_blob_ptr, void * new_blob_ptr)
{
    if (* (void **) existing_blob_ptr  == NULL)
    {
        * (void **) existing_blob_ptr = new_blob_ptr;
    }
    else
    {
        insert_blob(* (void **) existing_blob_ptr, new_blob_ptr);
    }
}

void cmap_put(cmap * cmap_ptr, const char * key, const void * val_addr)
{
    int index = hash(key, cmap_ptr->num_buckets);

    if (cmap_ptr->buckets[index] == NULL)
    {
        cmap_ptr->buckets[index] = create_blob(cmap_ptr, key, val_addr);
        cmap_ptr->num_entries++;
    }
    else
    {
        void * match_blob_ptr;
        if ((match_blob_ptr = search_bucket(cmap_ptr->buckets[index], key)) != NULL)
        {   
            if (cmap_ptr->cleanup_fn != NULL)
            {
                cmap_ptr->cleanup_fn(get_val_addr(match_blob_ptr));
            }

            memcpy(get_val_addr(match_blob_ptr), val_addr, cmap_ptr->val_size);
        }
        else
        {
            insert_blob(cmap_ptr->buckets[index], create_blob(cmap_ptr, key, val_addr));
            cmap_ptr->num_entries++;
        }
    }
}

/**
 * remove_blob
 *
 * Removes the blob with the given key from cmap. If no matching key is found, nothing is done.
 * @param cmap_ptr pointer to cmap
 * @param blob_ptr pointer to blob
 * @param key key for which to search
 * @return pointer to first blob in bucket
 */
static void * remove_blob(cmap * cmap_ptr, void * blob_ptr, const char * key)
{
    if (blob_ptr == NULL)
    {
        return NULL;
    }

    if ((strcmp(get_key_addr(blob_ptr), key)) == 0)
    {
        void * next_blob_ptr = * (void **) blob_ptr;

        if (cmap_ptr->cleanup_fn != NULL)
        {
            cmap_ptr->cleanup_fn(get_val_addr(blob_ptr));
        }

        free(blob_ptr);
        cmap_ptr->num_entries--;

        return next_blob_ptr;
    }
    else if (* (void **) blob_ptr == NULL)
    {
        return blob_ptr;
    }
    else
    {
        * (void **) blob_ptr = remove_blob(cmap_ptr, * (void **) blob_ptr, key);
        return blob_ptr;
    }
}

void cmap_remove(cmap * cmap_ptr, const char * key)
{
    int index = hash(key, cmap_ptr->num_buckets);

    if (cmap_ptr->buckets[index] != NULL)
    {
        cmap_ptr->buckets[index] = remove_blob(cmap_ptr, cmap_ptr->buckets[index], key);
    }
}

