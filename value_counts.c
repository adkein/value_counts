#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_LINE 4096
#define INITIAL_CAPACITY 1024

// Structure to hold count for each string
typedef struct {
    char *key;
    int count;
} Entry;

// Structure for our "hash table"
typedef struct {
    Entry *entries;
    size_t size;
    size_t capacity;
} Counter;

// Initialize counter
Counter* counter_create() {
    Counter *c = malloc(sizeof(Counter));
    c->entries = malloc(INITIAL_CAPACITY * sizeof(Entry));
    c->size = 0;
    c->capacity = INITIAL_CAPACITY;
    return c;
}

// Compare function for qsort
int compare_entries(const void *a, const void *b) {
    return strcmp(((Entry*)a)->key, ((Entry*)b)->key);
}

// Find or create entry for a key
Entry* counter_get(Counter *c, const char *key) {
    // Linear search (simplified implementation)
    for (size_t i = 0; i < c->size; i++) {
        if (strcmp(c->entries[i].key, key) == 0) {
            return &c->entries[i];
        }
    }
    
    // Grow array if needed
    if (c->size >= c->capacity) {
        c->capacity *= 2;
        c->entries = realloc(c->entries, c->capacity * sizeof(Entry));
    }
    
    // Add new entry
    c->entries[c->size].key = strdup(key);
    c->entries[c->size].count = 0;
    return &c->entries[c->size++];
}

// Print progress to stderr
void print_progress(Counter *c) {
    // Clear screen (ANSI escape codes)
    fprintf(stderr, "\033[2J\033[H");
    fprintf(stderr, "sortuniq intermediate results:\n");
    
    // Create temporary sorted array
    Entry *sorted = malloc(c->size * sizeof(Entry));
    memcpy(sorted, c->entries, c->size * sizeof(Entry));
    qsort(sorted, c->size, sizeof(Entry), compare_entries);
    
    // Print counts
    for (size_t i = 0; i < c->size; i++) {
        fprintf(stderr, "%s,%d\n", sorted[i].key, sorted[i].count);
    }
    
    free(sorted);
    fflush(stderr);
}

void counter_free(Counter *c) {
    for (size_t i = 0; i < c->size; i++) {
        free(c->entries[i].key);
    }
    free(c->entries);
    free(c);
}

int main(int argc, char *argv[]) {
    int progress_interval = 0;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--progress") == 0 && i + 1 < argc) {
            progress_interval = atoi(argv[i + 1]);
            i++;
        }
    }
    
    Counter *counts = counter_create();
    time_t last_update = time(NULL);
    char line[MAX_LINE];
    
    // Process input
    while (fgets(line, sizeof(line), stdin)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Update count
        Entry *entry = counter_get(counts, line);
        entry->count++;
        
        // Show progress if requested
        if (progress_interval > 0 && (time(NULL) - last_update) >= progress_interval) {
            print_progress(counts);
            last_update = time(NULL);
        }
    }
    
    // Clear progress output if needed
    if (progress_interval > 0) {
        fprintf(stderr, "\033[2J\033[H");
        fflush(stderr);
    }
    
    // Print final results
    qsort(counts->entries, counts->size, sizeof(Entry), compare_entries);
    for (size_t i = 0; i < counts->size; i++) {
        printf("%s,%d\n", counts->entries[i].key, counts->entries[i].count);
    }
    
    counter_free(counts);
    return 0;
} 