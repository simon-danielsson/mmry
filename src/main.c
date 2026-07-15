#include "main.h"
#include "parse_file.h"
#include "print_file.h"
#include "utils.h"
#include <time.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        err("no arguments were provided");
    }

    // get markdown file from args and resolve home directory
    char md_path[256] = {0};
    if (argv[1][0] == '~') {
        char *h = get_home_dir();
        char *ptr = argv[1];
        ptr++;
        snprintf(md_path, 255, "%s%s", h, ptr);
        free(h);
    } else {
        strcpy(md_path, argv[1]);
    }
    md_path[255] = '\0';

    // print program info
    {
        printf("%s %s (%s)\n", ENV_NAME, ENV_GITTAG, ENV_REPO);
        printf("File: %s\n", md_path);
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char temp[100] = {0};
        strftime(temp, sizeof(temp), "%A %h %d %Y, Week %W", t);
        printf("Today: %s\n\n", temp);
    }

    MmryFile mf = parse_file(md_path);

    print_file(&mf);

    MmryFile_free(&mf);

    return 0;
}
