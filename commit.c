#include "commit.h"
#include "index.h"
#include "tree.h"
#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ---------- HEAD ----------

int head_read(ObjectID *id_out) {
    FILE *f = fopen(HEAD_FILE, "r");
    if (!f) return -1;

    char line[256];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "ref: ", 5) == 0) {
        char ref_path[512];
        snprintf(ref_path, sizeof(ref_path), "%s/%s", PES_DIR, line + 5);

        f = fopen(ref_path, "r");
        if (!f) return -1;

        if (!fgets(line, sizeof(line), f)) {
            fclose(f);
            return -1;
        }
        fclose(f);

        line[strcspn(line, "\n")] = '\0';
    }

    return hex_to_hash(line, id_out);
}

int head_update(const ObjectID *id) {
    FILE *f = fopen(HEAD_FILE, "r");
    if (!f) return -1;

    char line[256];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    line[strcspn(line, "\n")] = '\0';

    char path[512];

    if (strncmp(line, "ref: ", 5) == 0) {
        snprintf(path, sizeof(path), "%s/%s", PES_DIR, line + 5);
    } else {
        snprintf(path, sizeof(path), "%s", HEAD_FILE);
    }

    f = fopen(path, "w");
    if (!f) return -1;

    char hex[HASH_HEX_SIZE + 1];
    hash_to_hex(id, hex);

    fprintf(f, "%s\n", hex);
    fclose(f);

    return 0;
}

// ---------- SERIALIZE ----------

int commit_serialize(const Commit *c, void **data_out, size_t *len_out) {
    char tree_hex[HASH_HEX_SIZE + 1];
    char parent_hex[HASH_HEX_SIZE + 1];

    hash_to_hex(&c->tree, tree_hex);

    char buffer[4096];
    int n = 0;

    n += snprintf(buffer + n, sizeof(buffer) - n, "tree %s\n", tree_hex);

    if (c->has_parent) {
        hash_to_hex(&c->parent, parent_hex);
        n += snprintf(buffer + n, sizeof(buffer) - n, "parent %s\n", parent_hex);
    }

    n += snprintf(buffer + n, sizeof(buffer) - n,
        "author %s %lu\n"
        "committer %s %lu\n\n"
        "%s\n",   // 👈 IMPORTANT newline fix
        c->author, c->timestamp,
        c->author, c->timestamp,
        c->message
    );

    *data_out = malloc(n);
    memcpy(*data_out, buffer, n);
    *len_out = n;

    return 0;
}

// ---------- PARSE ----------

int commit_parse(const void *data, size_t len, Commit *c) {
    (void)len;

    const char *p = (const char *)data;
    char hex[HASH_HEX_SIZE + 1];

    if (sscanf(p, "tree %64s\n", hex) != 1) return -1;
    if (hex_to_hash(hex, &c->tree) != 0) return -1;

    p = strchr(p, '\n') + 1;

    if (strncmp(p, "parent ", 7) == 0) {
        if (sscanf(p, "parent %64s\n", hex) != 1) return -1;
        if (hex_to_hash(hex, &c->parent) != 0) return -1;
        c->has_parent = 1;
        p = strchr(p, '\n') + 1;
    } else {
        c->has_parent = 0;
    }

    sscanf(p, "author %[^\n]\n", c->author);

    char *last_space = strrchr(c->author, ' ');
    c->timestamp = strtoull(last_space + 1, NULL, 10);
    *last_space = '\0';

    p = strstr(p, "\n\n");
    if (!p) return -1;

    strncpy(c->message, p + 2, sizeof(c->message) - 1);
    c->message[sizeof(c->message) - 1] = '\0';

    // remove trailing newline cleanly
    c->message[strcspn(c->message, "\n")] = '\0';

    return 0;
}

// ---------- WALK ----------

int commit_walk(commit_walk_fn fn, void *ctx) {
    ObjectID id;

    if (head_read(&id) != 0) return -1;

    while (1) {
        ObjectType type;
        void *data;
        size_t len;

        if (object_read(&id, &type, &data, &len) != 0) return -1;

        Commit c;
        if (commit_parse(data, len, &c) != 0) {
            free(data);
            return -1;
        }

        free(data);

        fn(&id, &c, ctx);

        if (!c.has_parent) break;

        id = c.parent;
    }

    return 0;
}

// ---------- CREATE ----------

int commit_create(const char *message, ObjectID *out) {
    Commit c;
    memset(&c, 0, sizeof(c));

    if (tree_from_index(&c.tree) != 0) return -1;

    ObjectID parent;
    if (head_read(&parent) == 0) {
        c.has_parent = 1;
        c.parent = parent;
    }

    strncpy(c.author, pes_author(), sizeof(c.author) - 1);
    c.timestamp = (uint64_t)time(NULL);
    strncpy(c.message, message, sizeof(c.message) - 1);

    void *data;
    size_t len;

    commit_serialize(&c, &data, &len);

    if (object_write(OBJ_COMMIT, data, len, out) != 0) {
        free(data);
        return -1;
    }

    free(data);

    return head_update(out);
}
