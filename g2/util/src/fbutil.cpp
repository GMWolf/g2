//
// Created by felix on 22/04/2021.
//

#include <fbutil.h>
#include <cstring>

std::optional<int64_t> g2::enumLookup(const char *str, const flatbuffers::TypeTable *table) {

  for(int i = 0; i < table->num_elems; i++) {
    if(strcmp(table->names[i], str) == 0) {
      return table->values ? table->values[i] : i;
    }
  }

  return {};
}
