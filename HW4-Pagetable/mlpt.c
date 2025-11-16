#define _POSIX_C_SOURCE 200112L

#include "mlpt.h"
#include "config.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// valid bit at LSB, 1 as unsigned long
#define PTE_VALID 1UL

size_t ptbr = 0;

// helper constants and macros from config.h 
static const size_t PAGE_SIZE_BYTES = (1UL << POBITS);
static const size_t OFFSET_MASK = ((1UL << POBITS) - 1);

// bit count and mask for one level
static const unsigned LEVEL_INDEX_BITS = (unsigned)(POBITS - 3); 
static const size_t LEVEL_INDEX_MASK = ((1UL << LEVEL_INDEX_BITS) - 1);

// allocates one aliged page based on the page size in bytes
static void *alloc_aligned_page(void) {
    void *ptr = NULL;
    posix_memalign(&ptr, PAGE_SIZE_BYTES, PAGE_SIZE_BYTES);
    memset(ptr, 0, PAGE_SIZE_BYTES);
    return ptr;
}

size_t translate(size_t va) {
    // check if no page table
    if (ptbr == 0) {
        return (size_t) -1; 
    }

    size_t offset = va & OFFSET_MASK;
    size_t vpn = va >> POBITS;
    size_t *page_table = (size_t*)(uintptr_t)ptbr;

    // iterate through each level to extract index and follow PTE
    for (int level = 0; level < LEVELS; ++level) {
        // compute index bits for specific level
        int levels_below = LEVELS - 1 - level;
        size_t index = (vpn >> (levels_below * LEVEL_INDEX_BITS) & LEVEL_INDEX_MASK);

        // get the page table entry and check valid bit
        size_t pte = page_table[index];
        if ((pte & PTE_VALID) == 0) {
            return (size_t) -1; 
        }

        // mask offset to extract the base address of the next page 
        size_t next_page_base = pte & ~OFFSET_MASK;

        // if at final level, return physical address (page base + offset)
        if (level == LEVELS - 1) {
            return next_page_base | offset;
        }
        // otherwise update page table 
        else {
            page_table = (size_t*)(uintptr_t)next_page_base;
        }
    }
    return (size_t) -1;
}



int allocate_page(size_t start_va) {
    // check if start_va is not the address at start of page
    if ((start_va & OFFSET_MASK) != 0) {
        return -1;
    }

    size_t vpn = start_va >> POBITS;
    int new_allocation = 0;

    // Ensure top-level page table exists 
    if (ptbr == 0) {
        void *page = alloc_aligned_page();
        ptbr = (size_t)(uintptr_t)page;
    }

    size_t *page_table = (size_t*)(uintptr_t)ptbr;

    // iterate over levels, allocate page tables if needed
    for (int level = 0; level < LEVELS; ++level) {
        int levels_below = LEVELS - 1 - level;
        size_t index = (vpn >> (levels_below * LEVEL_INDEX_BITS)) & LEVEL_INDEX_MASK;
        size_t pte = page_table[index];
        
        // need to allocate a page, either a page table or data page if were at final level
        if ((pte & PTE_VALID) == 0) {
            void *newpage = alloc_aligned_page();
            size_t pte_val = ((size_t)(uintptr_t)newpage) | PTE_VALID;
            // get new page table entry
            page_table[index] = pte_val;
            pte = pte_val;
            // use to see if we allocated a new page later
            new_allocation = 1;
        }

        // final level
        if (level == LEVELS - 1) {
            // new page or page table was allocated return 1, otherwise return 0
            if (new_allocation) {return 1;} 
            else {return 0;}   
        } 
        // descend to next level page table
        else {
            size_t next_page_base = pte & ~OFFSET_MASK;
            page_table = (size_t*)(uintptr_t)next_page_base;
        }
    }
    return 0;
}



/**
 * Proposed function for de-allocating memory
 * 
 * Fuction behavior described in README
 */

int deallocate_page(size_t start_va) {
    // check for non-page aligned start_va
    if ((start_va & ((PTE_VALID << POBITS) - 1)) != 0) {
        return -1; 
    }

    size_t vpn = start_va >> POBITS;
    size_t *tables[LEVELS];
    size_t indices[LEVELS];

    size_t *page_table = (size_t *)(uintptr_t)ptbr;

    // iterate through levels similar to other 2 functions
    for (int level = 0; level < LEVELS; ++level) {
        int levels_below = LEVELS - 1 - level;
        size_t index = (vpn >> (levels_below * (POBITS - 3))) & ((PTE_VALID << (POBITS - 3)) - 1);
        size_t pte = page_table[index];

        tables[level] = page_table;
        indices[level] = index;

        // page not allocated
        if ((pte & PTE_VALID) == 0) {
            return 0; 
        }

        // found data page, free it
        if (level == LEVELS - 1) {
            void *phys_page = (void *)(uintptr_t)(pte & ~((PTE_VALID << POBITS) - 1));
            free(phys_page);
            page_table[index] = 0;

            // clean up any newly empty page tables
            for (int l = LEVELS - 1; l > 0; --l) {
                size_t *tbl = tables[l];
                int empty = 1;
                for (size_t i = 0; i < (PTE_VALID << (POBITS - 3)); ++i) {
                    if (tbl[i] & PTE_VALID) {
                        empty = 0;
                        break;
                    }
                }
                if (empty) {
                    free(tbl);
                    tables[l - 1][indices[l - 1]] = 0;
                } else {
                    break;
                }
            }
            return 1;
        } else {
            page_table = (size_t *)(uintptr_t)(pte & ~((PTE_VALID << POBITS) - 1));
        }
    }
    return 0;
}