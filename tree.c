#include "tree.h"
#include "index.h"
#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compare_tree_entries(const void *a, const void *b) {
    return strcmp(((const TreeEntry *)a)->name,
                  ((const TreeEntry *)b)->name);
}

// Parse tree object
int tree_parse(const void *data, size_t len, Tree *tree_out) {
    tree_out->count = 0;

    const uint8_t *ptr = (const uint8_t *)data;
    const uint8_t *end = ptr + len;

    while (ptr < end && tree_out->count < MAX_TREE_ENTRIES) {
        TreeEntry *entry = &tree_out->entries[tree_out->count];

        const uint8_t *space = memchr(ptr, ' ', end - ptr);
        if (!space) return -1;

        char mode_str[16] = {0};
        memcpy(mode_str, ptr, space - ptr);
        entry->mode = strtol(mode_str, NULL, 8);

        ptr = space + 1;

        const uint8_t *null_byte = memchr(ptr, '\0', end - ptr);
        if (!null_byte) return -1;

        size_t name_len = null_byte - ptr;
        memcpy(entry->name, ptr, name_len);
        entry->name[name_len] = '\0';

        ptr = null_byte + 1;

        if (ptr + HASH_SIZE > end) return -1;
        memcpy(entry->hash.hash, ptr, HASH_SIZE);

        ptr += HASH_SIZE;
        tree_out->count++;
    }

    return 0;
}

// Serialize tree
int tree_serialize(const Tree *tree, void **data_out, size_t *len_out) {
    Tree sorted = *tree;

    qsort(sorted.entries, sorted.count,
          sizeof(TreeEntry), compare_tree_entries);

    size_t buffer_size = sorted.count * 300;
    uint8_t *buffer = malloc(buffer_size);
    if (!buffer) return -1;

    size_t offset = 0;

    for (int i = 0; i < sorted.count; i++) {
        const TreeEntry *e = &sorted.entries[i];

        int written = sprintf((char *)buffer + offset,
                              "%o %s", e->mode, e->name);

        offset += written + 1;
        memcpy(buffer + offset, e->hash.hash, HASH_SIZE);
        offset += HASH_SIZE;
    }

    *data_out = buffer;
    *len_out = offset;

    return 0;
}

// Build tree from index
int tree_from_index(ObjectID *id_out) {
    Index index;
    if (index_load(&index) != 0) return -1;
    if (index.count == 0) return -1;

    Tree tree;
    tree.count = index.count;

    for (int i = 0; i < index.count; i++) {
        tree.entries[i].mode = index.entries[i].mode;
        strcpy(tree.entries[i].name, index.entries[i].path);
        tree.entries[i].hash = index.entries[i].hash;
    }

    void *data;
    size_t len;

    if (tree_serialize(&tree, &data, &len) != 0) return -1;

    int rc = object_write(OBJ_TREE, data, len, id_out);
    free(data);

    return rc;
}
