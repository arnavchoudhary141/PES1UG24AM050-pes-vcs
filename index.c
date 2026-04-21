#include "index.h"
#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Find entry
IndexEntry* index_find(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0)
            return &index->entries[i];
    }
    return NULL;
}

// Load index from file
int index_load(Index *index) {
    index->count = 0;

    FILE *f = fopen(INDEX_FILE, "r");
    if (!f) return 0;

    char hex[HASH_HEX_SIZE + 1];

    while (index->count < MAX_INDEX_ENTRIES) {
        IndexEntry *e = &index->entries[index->count];

        int rc = fscanf(f, "%o %64s %u %u %255s\n",
                        &e->mode,
                        hex,
                        &e->mtime_sec,
                        &e->size,
                        e->path);

        if (rc != 5) break;

        if (hex_to_hash(hex, &e->hash) != 0) break;

        index->count++;
    }

    fclose(f);
    return 0;
}

// Save index to file
int index_save(const Index *index) {
    FILE *f = fopen(INDEX_FILE, "w");
    if (!f) return -1;

    char hex[HASH_HEX_SIZE + 1];

    for (int i = 0; i < index->count; i++) {
        hash_to_hex(&index->entries[i].hash, hex);

        fprintf(f, "%o %s %u %u %s\n",
                index->entries[i].mode,
                hex,
                index->entries[i].mtime_sec,
                index->entries[i].size,
                index->entries[i].path);
    }

    fclose(f);
    return 0;
}

// Add file to index
int index_add(Index *index, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("error: cannot open %s\n", path);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *buf = malloc(size);
    if (!buf) {
        fclose(f);
        return -1;
    }

    fread(buf, 1, size, f);
    fclose(f);

    ObjectID id;
    if (object_write(OBJ_BLOB, buf, size, &id) != 0) {
        free(buf);
        return -1;
    }
    free(buf);

    struct stat st;
    stat(path, &st);

    IndexEntry *e = index_find(index, path);

    if (!e) {
        if (index->count >= MAX_INDEX_ENTRIES) return -1;
        e = &index->entries[index->count++];
    }

    strcpy(e->path, path);
    e->hash = id;
    e->mode = 0100644;
    e->mtime_sec = st.st_mtime;
    e->size = st.st_size;

    return index_save(index);
}

// Status display
int index_status(const Index *index) {
    printf("Staged changes:\n");

    if (index->count == 0) {
        printf("    (nothing to show)\n\n");
        return 0;
    }

    for (int i = 0; i < index->count; i++) {
        printf("    staged: %s\n", index->entries[i].path);
    }

    printf("\n");
    return 0;
}
