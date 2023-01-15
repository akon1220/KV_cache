
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "kv_cache.h"


// Lookup the provided key in the KV cache.
// If the key is not found, fetch it from storage by calling read_key
// and place the KVPAIR in the cache.
// If placing a KVPAIR from memory into the cache causes the number of elements
// in the bin to exceed MAX_BIN_ELEMENTS, remove a KVPAIR using the LRU
// replacement policy and call write_key if the removed KVPAIR has
// been modified and changes need to be written back to memory.
// If the key is found in the cache, update LRU order for the
// key's bin appropriately.
// Input: the KV cache hash table and key to lookup
// Return: pointer to KVPAIR with the desired key

KVPAIR *lookup(KVCACHE *kv_cache, long key, unsigned int bins) {
    KVPAIR *my_key = kv_cache[key%bins].list;
    KVPAIR *head = kv_cache[key%bins].list;

    //initialize iterator
    int i = 0;

    while (i < kv_cache[key%bins].num_keys) {
        //break statement for null
        if (my_key == NULL) {
            break;
        }
        //key is found
        if(key == my_key->key){ 
            //increment hits
            hits++;
            // implement least recently used by removing key from its current position in the list and putting it at the front of the list
            KVPAIR *temp = my_key;
            // if first node, return since it's already in least recently used order 
            if (i < 1) {
                return my_key;
            }
            // if last node, remove it and add to front of list
            if (i == kv_cache[key%bins].num_keys - 1) {
                KVPAIR *p = my_key->prev;
                temp->next = head;
                temp->prev = NULL;
                head->prev = temp;
                p->next = NULL;
                kv_cache[key%bins].list = my_key;
                return my_key;
            }
            // if not first/last node, remove it and add to front of list
            if (i > 0 && i < kv_cache[key%bins].num_keys - 1) {
                //pointers for next and previous 
                KVPAIR *n = my_key->next;
                KVPAIR *p = my_key->prev;
                temp->prev = NULL;
                temp->next = NULL;
                n->prev = p;
                p->next = n;
                temp->next = head;
                head->prev = temp;
                kv_cache[key%bins].list = my_key;
                return my_key;
            }
        }
        //increment key and iterator
        my_key = my_key->next;
        i++;
    }

    //key not found
    KVPAIR *kv_new;
    // check if all empty, then add to the list
    if(kv_cache[key%bins].num_keys == 0) { 
        //increment misses
        misses++;
        // try new kvpair
        kv_new = read_key(key); 
        kv_cache[key%bins].list = kv_new;
        kv_new->next = NULL;
        kv_new->prev = NULL;
        kv_cache[key%bins].num_keys++;
        return kv_new;

    } 
    // if not yet full, then add to front of the list
    else {  
        //increment misses
        misses++;
        // read in new kvpair
        kv_new = read_key(key);
        kv_new->next = head;
        kv_new->prev = NULL;
        head->prev = kv_new;
        kv_cache[key%bins].list = kv_new;
        kv_cache[key%bins].num_keys++;
        if(kv_cache[key%bins].num_keys <= MAX_BIN_ELEMENTS) {
            return kv_new;
        }
    }
    // if full cache, free least recently used and add new key to front of list
    if(kv_cache[key%bins].num_keys > MAX_BIN_ELEMENTS) { 
        KVPAIR *current = kv_new;
        //Look at KVPAIR
        KVPAIR *node_last = current->next->next->next->next;
        KVPAIR *node_four = current->next->next->next;
        node_last->prev = NULL;
        node_last->next = NULL;
        node_four->next = NULL;
        //check if modified
        if(node_last->modified) {
            write_key(node_last);
            writebacks++;
        } 
        free(node_last->val);
        free(node_last);
        kv_cache[key%bins].num_keys--;
        return kv_new;
    }
    return NULL;
}
