//
// Created by felix on 28/08/2021.
//

#include "query.h"
#include <algorithm>

bool g2::ecs::queryMatch(const g2::ecs::Query &query, const g2::ecs::Type &type) {
    return std::ranges::all_of(query.components, [&type](id_t c) {
        return std::ranges::binary_search(type.components, c);
    });
}
