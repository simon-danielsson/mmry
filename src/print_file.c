#include "parse_file.h"
#include <stdio.h>
#include <time.h>

#define SEC_PER_DAY 86400

// for event and todo items
int days_until_due_or_overdue(MmryItem *item) {
    time_t now = time(NULL);
    time_t next_occ = item->date;
    double diff_seconds = difftime(next_occ, now);
    return abs((int)(diff_seconds / SEC_PER_DAY));
}

// returns -1 if not a repeatable item
int days_until_next_repeat(MmryItem *item) {
    if (item->mit.t != REPEAT) {
        return -1;
    }
    time_t now = time(NULL);
    time_t next_occ = item->date;
    while (next_occ < now) {
        next_occ += (item->mit.repeat * SEC_PER_DAY);
    }
    double diff_seconds = difftime(next_occ, now);
    return (int)((diff_seconds / SEC_PER_DAY));
}

time_t next_repeat(MmryItem *item) {
    time_t now = time(NULL);
    time_t next_occ = item->date;
    while (next_occ < now) {
        next_occ += (item->mit.repeat * SEC_PER_DAY);
    }
    return next_occ;
}

bool within_lead_time(MmryItem *item) {
    time_t now = time(NULL);
    if (item->mit.t == REPEAT) {
        time_t start_showing = next_repeat(item) - (item->lead_time * SEC_PER_DAY);
        return now >= start_showing;
    } else {
        time_t start_showing = item->date - (item->lead_time * SEC_PER_DAY);
        return now >= start_showing; // true if lead_time reached or passed
    }
}

void trim_str(char *str) {
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    size_t len = (end - start) + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

// accounting for utf-8 widths
size_t append_column(char *dst, size_t dstsize, const char *text, int width) {
    size_t used = strlen(dst);

    if (used >= dstsize)
        return used;

    int n = snprintf(dst + used, dstsize - used, "%s", text);
    if (n < 0)
        return used;

    used += (size_t)n;

    mbstate_t st = {0};
    const char *p = text;
    size_t wlen = mbsrtowcs(NULL, &p, 0, &st);

    int cols = 0;
    if (wlen != (size_t)-1) {
        wchar_t wbuf[wlen + 1];

        st = (mbstate_t){0};
        p = text;
        mbsrtowcs(wbuf, &p, wlen + 1, &st);

        cols = wcswidth(wbuf, wlen);
        if (cols < 0)
            cols = (int)strlen(text);
    }

    while (cols++ < width && used + 1 < dstsize)
        dst[used++] = ' ';

    if (used + 1 < dstsize)
        dst[used++] = ' ';

    dst[used] = '\0';

    return used;
}

// builder
void print_file(MmryFile *mf) {
    setlocale(LC_ALL, "");

    char print_buffer[4000] = "";
    int active_items_counter = 0;

    // column headers
    {
        int count = 74;
        char div[256] = {0};
        for (int i = 0; i < count; i++) {
            strcat(div, "┄");
        }
        char buff[512] = {0};
        snprintf(buff, sizeof(buff), "%-9s%-33s%-21s%s\n%s\n",
                "Type:", "Title:", "Date:", "Due:", div);
        strcat(print_buffer, buff);
    }

    for (size_t i = 0; i < mf->count; i++) {

        if (!within_lead_time(&mf->items[i])) {
            continue;
        }

        char line_builder[256] = {0};

        // type
        {
            char type[32] = {0};
            switch (mf->items[i].mit.t) {
                case REPEAT:
                    snprintf(type, sizeof(type), "REPEAT");
                    break;
                case TODO:
                    if (mf->items[i].mit.todo) {
                        snprintf(type, sizeof(type), "TODO");
                    } else {
                        snprintf(type, sizeof(type), "DONE");
                    }
                    break;
                case EVENT:
                    snprintf(type, sizeof(type), "EVENT");
                    break;
            }
            append_column(line_builder, sizeof(line_builder), type, 8);
        }

        // header
        {
            trim_str(mf->items[i].header);
            append_column(line_builder, sizeof(line_builder), mf->items[i].header,
                    32);
        }

        // date
        {
            struct tm *t = localtime(&mf->items[i].date);
            char date_buff[100];
            strftime(date_buff, sizeof(date_buff), "%a %Y-%m-%d", t);
            char *d = date_buff;

            append_column(line_builder, sizeof(line_builder), d, 20);
        }

        // todo status (done items aren't printed)
        {
            if (mf->items[i].mit.t == TODO) {
                if (mf->items[i].mit.todo) {
                    char buff[64] = {0};
                    int due = days_until_due_or_overdue(&mf->items[i]);
                    if (due == 0) {
                        snprintf(buff, sizeof(buff), "Today!");
                    } else if (mf->items[i].date < (time_t)time(NULL)) {
                        snprintf(buff, sizeof(buff), "%dd ago", due);
                    } else {
                        snprintf(buff, sizeof(buff), "%dd left", due);
                    }
                    append_column(line_builder, sizeof(line_builder), buff, 20);
                } else {
                    continue;
                }
            }
        }

        // event
        {
            if (mf->items[i].mit.t == EVENT) {
                char buff[64] = {0};
                int due = days_until_due_or_overdue(&mf->items[i]);
                if (due == 0) {
                    snprintf(buff, sizeof(buff), "Today!");
                } else if (mf->items[i].date < (time_t)time(NULL)) {
                    continue;
                } else {
                    snprintf(buff, sizeof(buff), "%dd left", due);
                }
                append_column(line_builder, sizeof(line_builder), buff, 15);
            }
        }

        // next occur if repeatable item
        {
            int next_occur = days_until_next_repeat(&mf->items[i]);
            if (next_occur != -1) {
                char buff[64] = {0};
                snprintf(buff, sizeof(buff), "%dd left", next_occur);
                append_column(line_builder, sizeof(line_builder), buff, 15);
            }
        }

        if (line_builder[0] != '\0') {
            active_items_counter++;
            strcat(print_buffer, line_builder);
            strcat(print_buffer, "\n");
        }
    }

    if (active_items_counter != 0) {
        print_buffer[3999] = '\0';
        puts(print_buffer);
    } else {
        printf("There's nothing to remember today!\n");
    }
}
