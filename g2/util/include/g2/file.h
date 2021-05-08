//
// Created by felix on 22/04/2021.
//

#ifndef G2_G2_UTIL_INCLUDE_FILE_H_
#define G2_G2_UTIL_INCLUDE_FILE_H_

#include <string>
#include <span>

namespace g2 {

  bool readFile(const char* filename, std::string& str);

  std::span<const char> map(const char* filename);

  void unmap(std::span<const char> data);
}

#endif  // G2_G2_UTIL_INCLUDE_FILE_H_
