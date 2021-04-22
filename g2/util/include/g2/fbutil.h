//
// Created by felix on 22/04/2021.
//

#ifndef G2_G2_UTIL_INCLUDE_G2_FBUTIL_H_
#define G2_G2_UTIL_INCLUDE_G2_FBUTIL_H_

#include <flatbuffers/flatbuffers.h>
#include <optional>

namespace g2 {


std::optional<int64_t> enumLookup(const char* str, const flatbuffers::TypeTable* table);

}

#endif  // G2_G2_UTIL_INCLUDE_G2_FBUTIL_H_
