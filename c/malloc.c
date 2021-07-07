#include "malloc.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct hsd_alloc_block {

    unsigned                 size;
    struct hsd_alloc_block * next, * prev;

};

/**
 * Free list starts out with a degenerate block. List is stored in order of
 * increasinng memory to make de-fragmentation measues easier.
 */
static struct hsd_alloc_block hsd_free_list_beginning = {
    .size        = 0,
    .next        = &hsd_free_list_beginning,
    .prev        = &hsd_free_list_beginning
};

void * hsd_malloc(unsigned size) {

    struct hsd_alloc_block *hsd_free_search = &hsd_free_list_beginning,
                           *hsd_match_block = NULL;

    /* Search free list to see if anything is big enough. */
    do {
        hsd_free_search = hsd_free_search->next;
        if (hsd_free_search->size >= size) {
            hsd_match_block = hsd_free_search;
        }
    } while (hsd_free_search != &hsd_free_list_beginning);

    if (hsd_match_block == NULL) {
        
        unsigned min_request_size = getpagesize();
        unsigned block_size = min_request_size > size ? min_request_size : size;

        void * newmem = sbrk( sizeof (struct hsd_alloc_block) + block_size );
        if (newmem == NULL) return NULL;

        hsd_match_block         = newmem;
        hsd_match_block->size   = block_size;

        /* Search the free list for a place. */
        hsd_free_search = &hsd_free_list_beginning;
        while (1) {
            if (
                ((hsd_free_search < hsd_match_block)
            &&  (hsd_free_search->next > hsd_match_block))
            ||  (hsd_free_search->next == &hsd_free_list_beginning)
            ) {
                hsd_free_search->prev->next = hsd_match_block;
                hsd_match_block->next->prev = hsd_match_block;
                break;
            }
            hsd_free_search = hsd_free_search->next;
        }

    }

    /* If matching block is big enough, return it and remove it from the list,
    * otherwise chop a chunk of memory off the end. */
    if (hsd_match_block->size == size) {
        hsd_match_block->prev = hsd_match_block->next;
        return hsd_match_block + 1;
    } else {
        hsd_match_block->size -= size;
        return ( (void *) (hsd_match_block + 1) ) + hsd_match_block->size - size;
    }

}

void hsd_free(void * block) {

    struct hsd_alloc_block *hsd_free_less, *hsd_free_more,
    *a_block = ( (struct hsd_alloc_block *) block) - 1;

    hsd_free_less = hsd_free_more = &hsd_free_list_beginning;
    while (1) {
        if (
            ((hsd_free_less < a_block)
        &&  (hsd_free_more > a_block))
        ||  (hsd_free_more == &hsd_free_list_beginning)
        ) {
            a_block->prev = hsd_free_less;
            hsd_free_less->next = a_block;
            a_block->next = hsd_free_more;
            hsd_free_more->prev = a_block;
            break;
        }
        hsd_free_less = hsd_free_more;
        hsd_free_more = hsd_free_more->next;
    }

    /* See if we can combine two blocks. */
    if ( ( (void *) (a_block + 1) ) + a_block->size == a_block->next) {
        a_block->size += a_block->next->size + sizeof (struct hsd_alloc_block);
    }
    if ( ( (void *) (a_block->prev + 1) ) + a_block->prev->size == a_block ) {
        a_block->prev->size += a_block->size + sizeof (struct hsd_alloc_block);
    }

}
