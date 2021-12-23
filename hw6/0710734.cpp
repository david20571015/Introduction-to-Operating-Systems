/*
Student No.: 0710734
Student Name: 邱俊耀
Email: david20571015.eed07@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <vector>

struct FileHeader {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

struct TarFile {
  std::string filename;
  fpos_t offset;
  long time;
};

class Tar {
 public:
  Tar(const char *filename) {
    file = fopen(filename, "rb");

    int end_count = 0;
    while (end_count < 2) {
      // get attr for TarFile
      fpos_t pos;
      fgetpos(file, &pos);

      FileHeader header;
      fread(&header, sizeof(header), 1, file);
      if (strcmp(header.name, "") == 0) {
        end_count++;
      } else {
        end_count = 0;
      }

      std::string name = "/" + std::string(header.name);
      if (name.back() == '/') {
        name.pop_back();
      }

      long time = strtol(header.mtime, NULL, 8);

      // if file existed => update
      size_t i;
      for (i = 0; i < headers.size(); i++) {
        if (strcmp(headers[i].filename.c_str(), name.c_str()) == 0) {
          if (headers[i].time < time) {
            headers[i].offset = pos;
            headers[i].time = time;
            break;
          }
        }
      }

      // if file not existed => add
      if (i == headers.size()) {
        TarFile tar_file;
        tar_file.filename = name;
        tar_file.offset = pos;
        tar_file.time = time;
        headers.push_back(tar_file);
      }

      long size = strtol(header.size, nullptr, 8);
      size = ((size + 511) / 512) * 512;
      fseek(file, size, SEEK_CUR);
    }

    headers.pop_back();
    headers.pop_back();
  }

  ~Tar() { fclose(file); }

  FILE *GetTar() { return file; }

  fpos_t *GetHeaderPos(const char *filename) {
    for (size_t i = 0; i < headers.size(); i++) {
      if (headers[i].filename == filename) {
        return &headers[i].offset;
      }
    }
    return nullptr;
  }

  std::vector<std::string> GetChildren(const char *filename) {
    if (strcmp(filename, "/") == 0) {
      filename = "";
    }
    std::vector<std::string> children;

    for (size_t i = 0; i < headers.size(); i++) {
      std::string path = headers[i].filename;
      if (IsChild(filename, path.c_str())) {
        children.push_back(path.substr(strlen(filename) + 1));
      }
    }

    return children;
  }

 private:
  FILE *file;
  std::vector<TarFile> headers;

  bool IsChild(const char *parent, const char *child) {
    if (strlen(parent) >= strlen(child)) {
      return false;
    }

    size_t i;
    for (i = 0; i < strlen(parent); i++) {
      if (parent[i] != child[i]) {
        return false;
      }
    }

    if (child[i++] != '/') {
      return false;
    }

    for (; i < strlen(child); i++) {
      if (child[i] == '/') {
        return false;
      }
    }

    return true;
  }
};

static Tar tar("./test.tar");

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi) {
  std::vector<std::string> children = tar.GetChildren(path);
  for (size_t i = 0; i < children.size(); i++) {
    filler(buffer, children[i].c_str(), nullptr, 0);
  }
  return 0;
}

int my_getattr(const char *path, struct stat *st) {
  if (strcmp(path, "/") == 0) {  // root
    st->st_mode = S_IFDIR | 0444;
    return 0;
  }

  fpos_t *pos = tar.GetHeaderPos(path);

  if (pos == nullptr) {
    return -ENOENT;
  }

  FILE *file = tar.GetTar();
  FileHeader header;
  fsetpos(file, pos);
  fread(&header, sizeof(header), 1, file);

  st->st_uid = strtol(header.uid, nullptr, 8);
  st->st_gid = strtol(header.gid, nullptr, 8);
  st->st_mtime = strtol(header.mtime, nullptr, 8);
  st->st_size = strtol(header.size, nullptr, 8);

  if (header.typeflag[0] == '5') {  // directory
    st->st_mode = S_IFDIR | strtol(header.mode, nullptr, 8);
  } else {  // file
    st->st_mode = S_IFREG | strtol(header.mode, nullptr, 8);
  }

  return 0;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset,
            struct fuse_file_info *fi) {
  FILE *file = tar.GetTar();
  fpos_t *pos = tar.GetHeaderPos(path);
  fsetpos(file, pos);
  FileHeader header;
  fread(&header, sizeof(header), 1, file);

  fseek(file, offset, SEEK_CUR);

  size_t content_size = strtoul(header.size, nullptr, 8);
  size = size < content_size ? size : content_size;

  return fread(buffer, sizeof(char), size, file);
}

static struct fuse_operations op;

int main(int argc, char *argv[]) {
  memset(&op, 0, sizeof(op));

  op.getattr = my_getattr;
  op.readdir = my_readdir;
  op.read = my_read;

  return fuse_main(argc, argv, &op, NULL);
}