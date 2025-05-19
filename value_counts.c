#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_LINE 4096
#define INITIAL_CAPACITY 16384  // Increased initial size

// Structure to hold count for each string
typedef struct Entry {
    char *key;
    int count;
    struct Entry *next;  // For hash table chaining
} Entry;

// Structure for our "hash table"
typedef struct {
    Entry **buckets;
    size_t num_buckets;
    size_t size;
} Counter;

// Simple hash function for strings
unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash;
}

// Initialize counter
Counter* counter_create() {
    Counter *c = malloc(sizeof(Counter));
    c->num_buckets = INITIAL_CAPACITY;
    c->buckets = calloc(c->num_buckets, sizeof(Entry*));
    c->size = 0;
    return c;
}

// Compare function for qsort
int compare_entries(const void *a, const void *b) {
    const Entry *ea = *(const Entry **)a;
    const Entry *eb = *(const Entry **)b;
    return strcmp(ea->key, eb->key);
}

// Find or create entry for a key
Entry* counter_get(Counter *c, const char *key) {
    unsigned long h = hash(key) % c->num_buckets;
    
    // Look for existing entry
    Entry *entry = c->buckets[h];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    
    // Create new entry
    entry = malloc(sizeof(Entry));
    entry->key = strdup(key);
    entry->count = 0;
    entry->next = c->buckets[h];
    c->buckets[h] = entry;
    c->size++;
    
    return entry;
}

// Print progress to stderr
void print_progress(Counter *c) {
    // Clear screen (ANSI escape codes)
    fprintf(stderr, "\033[2J\033[H");
    fprintf(stderr, "value_counts intermediate results:\n");
    
    // Convert hash table to array for sorting
    Entry **entries = malloc(c->size * sizeof(Entry*));
    size_t idx = 0;
    for (size_t i = 0; i < c->num_buckets; i++) {
        Entry *entry = c->buckets[i];
        while (entry) {
            entries[idx++] = entry;
            entry = entry->next;
        }
    }
    
    // Sort entries
    qsort(entries, c->size, sizeof(Entry*), compare_entries);
    
    // Print counts
    for (size_t i = 0; i < c->size; i++) {
        fprintf(stderr, "%s,%d\n", entries[i]->key, entries[i]->count);
    }
    
    free(entries);
    fflush(stderr);
}

void counter_free(Counter *c) {
    for (size_t i = 0; i < c->num_buckets; i++) {
        Entry *entry = c->buckets[i];
        while (entry) {
            Entry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
    free(c->buckets);
    free(c);
}

int main(int argc, char *argv[]) {
    int progress_interval = 0;
    char delimiter = '\t';  // Default to tab
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--progress") == 0 && i + 1 < argc) {
            progress_interval = atoi(argv[i + 1]);
            i++;
        } else if ((strcmp(argv[i], "--delim") == 0 || strcmp(argv[i], "-d") == 0) && i + 1 < argc) {
            delimiter = argv[i + 1][0];  // Take first character of delimiter argument
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
    
    // Convert hash table to array for sorting
    Entry **entries = malloc(counts->size * sizeof(Entry*));
    size_t idx = 0;
    for (size_t i = 0; i < counts->num_buckets; i++) {
        Entry *entry = counts->buckets[i];
        while (entry) {
            entries[idx++] = entry;
            entry = entry->next;
        }
    }
    
    // Sort and print results
    qsort(entries, counts->size, sizeof(Entry*), compare_entries);
    
    // Update progress output format
    for (size_t i = 0; i < counts->size; i++) {
        fprintf(stderr, "%s%c%d\n", entries[i]->key, delimiter, entries[i]->count);
    }
    
    // Update final output format
    for (size_t i = 0; i < counts->size; i++) {
        printf("%s%c%d\n", entries[i]->key, delimiter, entries[i]->count);
    }
    
    free(entries);
    counter_free(counts);
    return 0;
} 