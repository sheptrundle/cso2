#include "tlb.h"
#include <stdbool.h>
#include <stddef.h>

#define SET_COUNT 16
#define WAY_COUNT 4

// parallel arrays for each set and way
static bool tlb_valid[SET_COUNT][WAY_COUNT];
static size_t tlb_vpn[SET_COUNT][WAY_COUNT];
static size_t tlb_pa[SET_COUNT][WAY_COUNT];
static unsigned tlb_lru[SET_COUNT][WAY_COUNT];


// Clear all arrays
void tlb_clear() {
    for (int set = 0; set < SET_COUNT; set++) {
        for (int way = 0; way < WAY_COUNT; way++) {
            tlb_valid[set][way] = false;
            tlb_vpn[set][way] = 0;
            tlb_pa[set][way] = 0;
            tlb_lru[set][way] = 0;
        }
    }
}

int tlb_peek(size_t va) {
    size_t vpn = va >> POBITS;
    int set = vpn % SET_COUNT;

    for (int way = 0; way < WAY_COUNT; way++) {
        // check if valid and vpn matches 
        if (tlb_valid[set][way] && tlb_vpn[set][way] == vpn) {
            // found, return least recently used status
            return (int) tlb_lru[set][way];
        }
    }
    // return 0 if va has no valid mapping in tlb
    return 0;
}

size_t tlb_translate(size_t va) {
    size_t vpn = va >> POBITS;
    int set = vpn % SET_COUNT;
    size_t offset = va & ((1UL << POBITS) - 1);

    // check each way for a hit
    for (int way = 0; way < WAY_COUNT; way++) {
        if (tlb_valid[set][way] && tlb_vpn[set][way] == vpn) {
            // hit, update LRU
            unsigned old_rank = tlb_lru[set][way];
            for (int i = 0; i < WAY_COUNT; i++) {
                if (!tlb_valid[set][i]) {
                    continue;
                }
                if (tlb_lru[set][i] < old_rank) {
                    tlb_lru[set][i]++;
                }
            }
            // set current way to most recently used
            tlb_lru[set][way] = 1;
            // return cached PA
            return (tlb_pa[set][way] | offset);
        }
    }

    // miss, now translate
    size_t pa = translate(va & ~((1UL << POBITS) - 1));

    // invalid translation
    if (pa == (size_t) -1) {
        return (size_t) -1;  
    }

    // find replacement slot, either take first invalid or LRU
    int replace_slot = -1;
    for (int way = 0; way < WAY_COUNT; way++) {
        if (!tlb_valid[set][way]) {
            replace_slot = way;
            break;
        }
    }
    if (replace_slot == -1) {
        for (int way = 0; way < WAY_COUNT; way++) {
            if (tlb_lru[set][way] == WAY_COUNT) {
                replace_slot = way;
                break;
            }
        }
    } 

    // update the arrays
    tlb_valid[set][replace_slot] = true;
    tlb_vpn[set][replace_slot] = vpn;
    tlb_pa[set][replace_slot] = pa & ~((1UL << POBITS) - 1);
;

    // update LRU
    for (int way = 0; way < WAY_COUNT; way++) {
        if (!tlb_valid[set][way]) {
            continue;
        }
        if (way != replace_slot && tlb_lru[set][way] < WAY_COUNT)
            tlb_lru[set][way]++;
    }
    tlb_lru[set][replace_slot] = 1;

    return (tlb_pa[set][replace_slot] | offset);
}

