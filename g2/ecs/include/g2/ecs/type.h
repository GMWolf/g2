//
// Created by felix on 28/08/2021.
//

#ifndef G2_TYPE_H
#define G2_TYPE_H

#include <cstdint>
#include <vector>
#include <initializer_list>

namespace g2::ecs {

    using id_t = uint64_t;

    struct Type {
        Type(std::initializer_list<id_t> l) : components(l){
            std::sort(components.begin(), components.end());
        }

        std::vector<id_t> components;
    };

    inline auto operator<=>(const Type& a, const Type& b) {
        return a.components <=> b.components;
    }

    inline auto operator==(const Type& a, const Type& b) {
        return a.components == b.components;
    }

    struct Component {
        size_t size;
        size_t align;
    };

}

#endif //G2_TYPE_H
