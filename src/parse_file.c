#include "main.h"
#include <stdio.h>

typedef enum { REPEAT, TODO, EVENT } MmryItemTypeEnum;

typedef struct {
    MmryItemTypeEnum t;
    union {
        u32 repeat;
        bool todo;
    };
} MmryItemType;

MmryItemType MmryItemType_default() { return (MmryItemType){.t = EVENT}; }

typedef struct {
    MmryItemType mit;
    char *header;
    char *content;
    int lead_time;
    time_t date;
    int id;
} MmryItem;

void MmryItem_init(MmryItem *item, char *header, int id) {
    item->header = strdup(header);
    item->id = id;
    item->date = time(NULL);
    item->content = strdup("");
    item->lead_time = 7;
}

typedef struct {
    MmryItem *items;
    size_t count;
    size_t cap;
} MmryFile;

void MmryFile_push(MmryItem item, MmryFile *file) {
    if (file->count == file->cap) {
        // double cap, or start at 4 if empty
        file->cap = (file->cap == 0) ? 4 : file->cap * 2;
        MmryItem *temp = realloc(file->items, file->cap * sizeof(MmryItem));

        if (temp == NULL) {
            err("allocation failure when pushing to MmryFile");
        }
        file->items = temp;
    }

    file->items[file->count] = item;
    file->count++;
}

void MmryFile_free(MmryFile *file) {
    for (size_t i = 0; i < file->count; i++) {
        free(file->items[i].header);
        free(file->items[i].content);
    }
    free(file->items);
}

MmryFile parse_file(char *md_path) {

    // read -------------------------------------------------------------------

    FILE *file = fopen(md_path, "r");
    if (file == NULL) {
        err("failed to open file\n");
    }

    // parse ------------------------------------------------------------------

    char buffer[512];
    int line, id_counter = 0;
    MmryItem mit = {0};
    MmryFile mf = {0};
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (buffer[0] == '#') {
            // push if not null
            if (mit.header != NULL && mit.header[0] != '\0') {
                MmryFile_push(mit, &mf);
            }

            char *h = buffer + 2; // offset '#'

            MmryItem_init(&mit, h, id_counter);
            id_counter++;
            printf("%d: %s", line, buffer);

        } else if (buffer[0] == '{') { // metadata
            printf("%d: %s", line, buffer);
        } else {
        }
        line++;
    }

    if (feof(file)) {
    } else if (ferror(file)) {
        fclose(file);
        err("while reading file\n");
    }

    fclose(file);

    return mf;
}
