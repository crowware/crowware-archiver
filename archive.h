#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <stdint.h>
#ifdef _WIN32
  #include <direct.h>
  #define PATH_SEP '\\'
#else
  #include <sys/stat.h>
  #define PATH_SEP '/'
#endif

#ifdef _MSC_VER
  #pragma pack(push,1)
  #define PACKED
#else
  #define PACKED __attribute__((packed))
#endif

typedef struct PACKED {
    char magic[4];             /* "CWA1" */
    uint32_t entry_count;
} cwa_header_t;

typedef struct PACKED {
    uint32_t filename_len;
    uint64_t original_size;
    uint64_t compressed_size;
} cwa_entry_header_t;

#ifdef _MSC_VER
  #pragma pack(pop)
#endif

int create_archive(const char *archive_path, int file_count, char *files[]);
int list_archive(const char *archive_path);
int extract_archive(const char *archive_path, const char *out_dir);

#endif