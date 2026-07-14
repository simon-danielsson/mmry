#include "parse_file.h"
#include <stdio.h>
#include <time.h>

#define SEC_PER_DAY 86400
bool within_lead_time(MmryItem *item) {
    time_t now = time(NULL);
    time_t start_showing = item->date - (item->lead_time * SEC_PER_DAY);
    return now >= start_showing; // true if lead_time reached or passed
}

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
    return (int)(diff_seconds / SEC_PER_DAY);
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

// builder
void print_file(MmryFile *mf) {

    // column headers
    {
        printf("%-9s%-31s%-21s%s\n", "Type:", "Title:", "Date:", "Due:");
        int count = 70;
        for (int i = 0; i < count; i++) {
            printf("┄");
        }
        putchar('\n');
    }

    for (size_t i = 0; i < mf->count; i++) {
        if (!within_lead_time(&mf->items[i])) {
            continue;
        }

        char line_builder[256] = {0};

        // icon
        {
            char icon[32] = {0};
            switch (mf->items[i].mit.t) {
                case REPEAT:
                    snprintf(icon, sizeof(icon), "REPEAT");
                    break;
                case TODO:
                    if (mf->items[i].mit.todo) {
                        snprintf(icon, sizeof(icon), "TODO");
                    } else {
                        snprintf(icon, sizeof(icon), "DONE");
                    }
                    break;
                case EVENT:
                    snprintf(icon, sizeof(icon), "EVENT");
                    break;
            }
            char buff[64] = {0};
            snprintf(buff, sizeof(buff), "%-9s", icon);
            strncat(line_builder, buff, 64);
        }

        // id
        // {
        //     char buff[64] = {0};
        //     snprintf(buff, sizeof(buff), "%-4d ", mf->items[i].id);
        //     strncat(line_builder, buff, 64);
        // }

        // header
        {
            trim_str(mf->items[i].header);
            char buff[64] = {0};
            snprintf(buff, sizeof(buff), "%-30s ", mf->items[i].header);
            strncat(line_builder, buff, 64);
        }

        // date
        {
            struct tm *t = localtime(&mf->items[i].date);
            char date_buff[100];
            strftime(date_buff, sizeof(date_buff), "%a %Y-%m-%d", t);
            char *d = date_buff;

            char buff[64] = {0};
            snprintf(buff, sizeof(buff), "%-20s ", d);
            strncat(line_builder, buff, 64);
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
                        snprintf(buff, sizeof(buff), "%dd overdue", due);
                    } else {
                        snprintf(buff, sizeof(buff), "%dd left", due);
                    }
                    strncat(line_builder, buff, 64);
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
                    snprintf(buff, sizeof(buff), "%dd overdue", due);
                } else {
                    snprintf(buff, sizeof(buff), "%dd left", due);
                }
                strncat(line_builder, buff, 64);
            }
        }

        // next occur if repeatable item
        {
            int next_occur = days_until_next_repeat(&mf->items[i]);
            if (next_occur != -1) {
                char buff[64] = {0};
                snprintf(buff, sizeof(buff), "%dd left", next_occur);
                strncat(line_builder, buff, 64);
                // }
        }

        printf("%s\n", line_builder);
    }
}
}
