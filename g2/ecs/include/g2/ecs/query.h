//
// Created by felix on 28/08/2021.
//

#ifndef G2_QUERY_H
#define G2_QUERY_H

#include "type.h"
#include "registry.h"

#include <vector>

namespace g2::ecs {

    struct Query {
        std::vector<id_t> components;
    };


    bool queryMatch(const Query& query, const Type& type);

    struct QueryIterator {
        Query query;
        std::vector<Archetype>::iterator archetypeIterator;
        std::vector<Archetype>::iterator archetypeEnd;
        std::vector<Chunk*>::iterator chunkIterator;

        bool operator==(const QueryIterator& other) const {
            return archetypeIterator == other.archetypeIterator
                && chunkIterator == other.chunkIterator;
        }

        bool operator!=(const QueryIterator& other) const {
            return !this->operator==(other);
        }

        QueryIterator& operator++() {
            chunkIterator++;
            if (chunkIterator == archetypeIterator->chunks.end()) {
                do {
                    archetypeIterator++;
                } while(archetypeIterator != archetypeEnd && !queryMatch(query, archetypeIterator->type));
                chunkIterator = archetypeIterator == archetypeEnd ? typeof(chunkIterator){} : archetypeIterator->chunks.begin();
            }
            return *this;
        }

    };



}

#endif //G2_QUERY_H
