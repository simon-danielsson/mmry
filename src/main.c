#include "main.h"

void err(const char *msg) {
    printf("Error: %s", msg);
    exit(1);
}

char *get_home_dir() {
    char buff[256] = {0};
    char *d = getcwd(buff, 256);
    int brack_encount = 0;
    char *part1 = (char *)malloc(256);
    for (int i = 0; d[i] != '\0'; i++) {
        if (d[i] == '/') {
            brack_encount++;
        }
        if (brack_encount == 3) {
            strncpy(part1, d, i);
            part1[i] = '\0';
            break;
        }
    }
    return part1;
}

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
    if (!BUILD_RELEASE) {
        printf("File at path: %s\n", md_path);
    }

    FILE *file = fopen(md_path, "r");
    if (file == NULL) {
        err("failed to open file\n");
        return 1;
    }

    char buffer[2048];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    if (feof(file)) {
    } else if (ferror(file)) {
        printf("An error occurred.\n");
    }

    fclose(file);
    return 0;
}
