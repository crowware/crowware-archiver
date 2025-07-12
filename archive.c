#include "archive.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/stat.h>

static int file_exists_and_size(const char *path, uint64_t *size_out) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    *size_out = (uint64_t)st.st_size;
    return 1;
}

static int compress_rle(const uint8_t *in, uint64_t in_sz, uint8_t **out, uint64_t *out_sz)
{
    if (!in || !out || !out_sz) return -1;
    uint8_t *buf = malloc(in_sz * 2 + 1);
    if (!buf) return -1;
    uint64_t o = 0;
    for (uint64_t i = 0; i < in_sz;) {
        uint8_t val = in[i];
        uint8_t cnt = 1;
        while (i + cnt < in_sz && in[i + cnt] == val && cnt < 255) cnt++;
        buf[o++] = cnt;
        buf[o++] = val;
        i += cnt;
    }
    *out = buf;
    *out_sz = o;
    return 0;
}

static int decompress_rle(const uint8_t *in, uint64_t in_sz, uint8_t **out, uint64_t expected)
{
    if (!in || !out) return -1;
    uint8_t *buf = malloc(expected);
    if (!buf) return -1;
    uint64_t o = 0;
    for (uint64_t i = 0; i + 1 < in_sz; i += 2) {
        uint8_t cnt = in[i];
        uint8_t val = in[i + 1];
        if (o + cnt > expected) { free(buf); return -1; }
        memset(buf + o, val, cnt);
        o += cnt;
    }    
    *out = buf;
    return 0;
}

static int read_file(const char *p, uint8_t **b, uint64_t *s)
{
    struct stat st;
    if (stat(p, &st) != 0) return -1;
    uint64_t len = (uint64_t)st.st_size;

    FILE *f = fopen(p, "rb");
    if (!f) return -1;

    uint8_t *buf = malloc(len);
    if (!buf) { fclose(f); return -1; }
    if (fread(buf, 1, len, f) != len) { fclose(f); free(buf); return -1; }
    fclose(f);

    *b = buf;
    *s = len;
    return 0;
}

static int write_file(const char *p, const uint8_t *b, uint64_t s)
{
    FILE *f = fopen(p, "wb");
    if (!f) return -1;
    if (fwrite(b, 1, s, f) != s) { fclose(f); return -1; }
    fclose(f);    
    return 0;
}

static void *xmalloc(size_t n) { void *p = malloc(n); if (!p) { perror("malloc"); exit(1);} return p; }

int create_archive(const char *out_path, int n, char *files[])
{
    for (int i = 0; i < n; ++i) {
        uint64_t filesize;
        if (!file_exists_and_size(files[i], &filesize)) {
            fprintf(stderr, "Error: File '%s' does not exist or is inaccessible.\n", files[i]);
            return -1;
        }
    }

    FILE *out = fopen(out_path, "wb");
    if (!out) return -1;
    cwa_header_t h = {{'C','W','A','1'}, (uint32_t)n};
    fwrite(&h, sizeof h, 1, out);
    for (int i = 0; i < n; ++i) {
        uint8_t *raw; uint64_t raw_sz;
        if (read_file(files[i], &raw, &raw_sz)) { fclose(out); return -1; }
        uint8_t *cmp; uint64_t cmp_sz;
        if (compress_rle(raw, raw_sz, &cmp, &cmp_sz)) { free(raw); fclose(out); return -1; }
        cwa_entry_header_t eh = {(uint32_t)strlen(files[i]), raw_sz, cmp_sz};
        fwrite(&eh, sizeof eh, 1, out);
        fwrite(files[i], 1, eh.filename_len, out);
        fwrite(cmp, 1, cmp_sz, out);
        free(raw); free(cmp);
    }
    fclose(out);
    return 0;
}

int list_archive(const char *p)
{
    FILE *in = fopen(p, "rb");
    if (!in) return -1;
    cwa_header_t h; fread(&h, sizeof h, 1, in);
    if (memcmp(h.magic, "CWA1", 4)) { fclose(in); return -1; }
    for (uint32_t i = 0; i < h.entry_count; ++i) {
        cwa_entry_header_t eh; fread(&eh, sizeof eh, 1, in);
        char *name = xmalloc(eh.filename_len + 1);
        fread(name, 1, eh.filename_len, in);
        name[eh.filename_len] = 0;
        fseek(in, (long)eh.compressed_size, SEEK_CUR);
        printf("%-30s %10" PRIu64 " bytes\n", name, eh.original_size);
        free(name);
    }    
    fclose(in);
    return 0;
}

int extract_archive(const char *p, const char *out_dir)
{
    FILE *in = fopen(p, "rb");
    if (!in) return -1;
    cwa_header_t h; fread(&h, sizeof h, 1, in);
    if (memcmp(h.magic, "CWA1", 4)) { fclose(in); return -1; }
    for (uint32_t i = 0; i < h.entry_count; ++i) {
        cwa_entry_header_t eh; fread(&eh, sizeof eh, 1, in);
        char *name = xmalloc(eh.filename_len + 1);
        fread(name, 1, eh.filename_len, in); name[eh.filename_len] = 0;
        uint8_t *cmp = xmalloc(eh.compressed_size);
        fread(cmp, 1, eh.compressed_size, in);
        uint8_t *raw;
        if (decompress_rle(cmp, eh.compressed_size, &raw, eh.original_size)) { free(name); free(cmp); fclose(in); return -1; }
        char out_path[4096];
        snprintf(out_path, sizeof out_path, "%s%c%s", out_dir, PATH_SEP, name);
        write_file(out_path, raw, eh.original_size);
        free(name); free(cmp); free(raw);
    }    
    fclose(in);
    return 0;
}
