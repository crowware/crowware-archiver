# 🗜️ Crowware Archiver

A small, cross-platform archiver using RLE (Run-Length Encoding) compression. Built in C, designed for portability and minimalism. Supports archive creation, listing, and extraction.

# ⚠️ Warning

  This tool was built with cross-platform support in mind, but has not been tested on Linux.\
  Use on Linux at your own risk — contributions and fixes are welcome.

## ⚙️ Features

- Create `.cwa` (Crowware Archive) files
- Basic RLE compression
- Simple file format with custom headers
- Cross-platform support (Windows & POSIX)
- Pure C, no external dependencies

## 🔧 Usage

```
cwa c <archive.cwa> <files...>   Create archive
cwa l <archive.cwa>             List archive contents
cwa x <archive.cwa> [outdir]    Extract archive (default outdir is `.`)
```

## 📦 Archive Format

- Header:
  - Magic: `CWA1`
  - File count (uint32)

- For each file:
  - Filename length (uint32)
  - Original size (uint64)
  - Compressed size (uint64)
  - Filename bytes
  - Compressed data

Compression is done via RLE.

## 🧪 Example

```
cwa c mydata.cwa file1.txt image.bmp
cwa l mydata.cwa
cwa x mydata.cwa output_folder
```

## 💻 Build

```bash
gcc -o cwa.exe main.c archive.c
```

Windows:

```bash
cl /Fe:cwa.exe.exe main.c archive.c
```

## 🗂️ File Structure

- `main.c` — CLI interface
- `archive.c` — Archive logic and compression
- `archive.h` — Format structures and function declarations

## 🔐 License

Apache-2.0 License

## 🐦 Crowware

Made by [Crowware](https://github.com/crowware), a collective focused on tooling and software experimentation.
