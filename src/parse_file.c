#include "main.h"
#include <assert.h>
#include <stdio.h>

typedef enum { REPEAT, TODO, EVENT } MmryItemTypeEnum;

typedef struct {
    MmryItemTypeEnum t;
    union {
        int repeat;
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
    item->content = (char *)malloc(sizeof(char) * 1024);
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

void append_char_to_null_term_str(char *s, char c) {
    int len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}

void get_from_within_parens(char *dest, char *s) {
    bool collect = false;
    int dest_idx = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] == '(') {
            collect = true;
            continue;
        } else if (s[i] == ')') {
            break;
        }
        if (collect) {
            dest[dest_idx++] = s[i];
        }
    }
    dest[dest_idx] = '\0';
    // printf("Extracted: %s\n", dest);
}

// source:
// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char **str_split(char *a_str, const char a_delim) {
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result) {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

// takes ISO date as c-string: "YYYY-MM-DD"
time_t create_time_t_from_str(char *date_as_str) {
    char **s = str_split(date_as_str, '-');

    int year = atoi(s[0]);
    int month = atoi(s[1]);
    int day = atoi(s[2]);

    return ({
            struct tm t = {0};
            t.tm_year = year - 1900; // years since 1900
            t.tm_mon = month - 1;    // months since January (0-11)
            t.tm_mday = day;         // day of the month (1-31)
            mktime(&t);
            });
}

MmryFile parse_file(char *md_path) {

    // read -------------------------------------------------------------------

    FILE *file = fopen(md_path, "r");
    if (file == NULL) {
        err("failed to open file\n");
    }

    // parse ------------------------------------------------------------------

    char buffer[1024];
    char content[1024] = {0};
    int line = 0;
    int id_counter = 0;
    MmryItem mit = {0};
    MmryFile mf = {0};
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // skip html style comments
        if (strstr(buffer, "<!--")) {
            continue;
        }
        if (buffer[0] == '#') {
            // push if not null
            if (mit.header != NULL && mit.header[0] != '\0') {
                // printf("\n%s", mit.header);
                MmryFile_push(mit, &mf);
            }

            char *h = buffer + 2; // offset '#'

            MmryItem_init(&mit, h, id_counter);
            id_counter++;
            // printf("%d: %s", line, buffer);

        } else if (buffer[0] == '{') { // metadata

            char *h = buffer + 1; // offset '{'
            char **words = str_split(buffer, ' ');
            if (words) {
                int i;
                for (i = 0; *(words + i); i++) {
                    if (strstr(*(words + i), "todo")) {
                        char result[256] = {0};
                        get_from_within_parens(result, *(words + i));
                        mit.mit.t = TODO;
                        mit.mit.todo = false;
                        if (result[0] == 't') {
                            mit.mit.todo = true;
                        }
                    }

                    if (strstr(*(words + i), "event")) {
                        mit.mit.t = EVENT;

                    } else if (strstr(*(words + i), "repeat")) {
                        char result[256] = {0};
                        get_from_within_parens(result, *(words + i));
                        mit.mit.t = REPEAT;
                        mit.mit.repeat = atoi(result);

                    } else if (strstr(*(words + i), "date")) {
                        char result[256] = {0};
                        get_from_within_parens(result, *(words + i));
                        mit.date = create_time_t_from_str(result);

                    } else if (strstr(*(words + i), "lead")) {
                        char result[256] = {0};
                        get_from_within_parens(result, *(words + i));
                        mit.lead_time = atoi(result);
                    }
                    free(*(words + i));
                }
                free(words);
            }

            // printf("%d: %s", line, buffer);

        } else {
            if (buffer[0] == '\0' || buffer[0] == '\n') {
                continue;
            }
            // TODO: will need to improve this. perhaps add some realloc action
            memset(content, 0, sizeof(content));
            strcat(content, buffer);
            content[1023] = '\0';
            free(mit.content);
            mit.content = strdup(content);
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
