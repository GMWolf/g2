//
// Created by felix on 22/04/2021.
//
#include <file.h>
#include <fstream>
bool g2::readFile(const char *filename, std::string &str) {

  str.clear();

  std::ifstream ifs(filename);
  if(!ifs) {
    return false;
  }

  ifs.seekg(0, std::ios_base::end);
  size_t size = ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);

  str.resize(size, '\0');
  ifs.read(str.data(), size);

  return true;
}
