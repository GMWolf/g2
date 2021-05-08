//
// Created by felix on 22/04/2021.
//
#include <file.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

bool g2::readFile(const char *filename, std::string &str) {

    str.clear();

    std::ifstream ifs(filename);
    if (!ifs) {
        return false;
    }

    ifs.seekg(0, std::ios_base::end);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios_base::beg);

    str.resize(size, '\0');
    ifs.read(str.data(), size);

    return true;
}

std::span<const char> g2::map(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return {};
    }

    struct stat sb{};
    if (fstat(fd, &sb) == -1) {
        return {};
    }

    size_t length = sb.st_size;
    const void *ptr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0u);

    if (ptr == MAP_FAILED) {
        return {};
    }
    return {(const char *) ptr, length};
}

void g2::unmap(std::span<const char> data) {
    munmap((void *) data.data(), data.size_bytes());
}
