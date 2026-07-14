#include "main.h"

typedef enum { REPEAT, TODO, EVENT } MmryItemTypeEnum;

typedef struct {
  MmryItemTypeEnum t;
  union {
    u32 repeat;
    bool todo;
  };
} MmryItemType;

typedef struct {
  MmryItemType mit;
  char *header;
  char *content;
  int lead_time;
  time_t date;
  int id;
} MmryItem;

typedef struct {
  MmryItem *items;
  size_t count;
  size_t cap;
} MmryFile;

MmryFile parse_file(char *md_path);

void MmryFile_free(MmryFile *file);
