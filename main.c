#include "archive.h"
#include <stdio.h>

static void usage(void)
{
    puts("Crowware File Archiver (cross‑platform)\n"\
         "Usage:\n"\
         "  cwa c <archive.cwa> <files...>   Create archive\n"\
         "  cwa l <archive.cwa>             List archive\n"\
         "  cwa x <archive.cwa> [outdir]    Extract archive (default .)\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3) { usage(); return 1; }
    char m = argv[1][0];
    if (m == 'c') {
        if (argc < 4) { usage(); return 1; }
        return create_archive(argv[2], argc - 3, &argv[3]);
    } else if (m == 'l') {
        return list_archive(argv[2]);
    } else if (m == 'x') {
        const char *outdir = argc >= 4 ? argv[3] : ".";
        return extract_archive(argv[2], outdir);
    } else { usage(); return 1; }
}
