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

        inline bool operator==(const QueryIterator& other) const {
            return archetypeIterator == other.archetypeIterator
                && chunkIterator == other.chunkIterator;
        }

        inline bool operator!=(const QueryIterator& other) const {
            return !this->operator==(other);
        }

        inline QueryIterator& operator++() {
            chunkIterator++;
            if (chunkIterator == archetypeIterator->chunks.end()) {
                do {
                    archetypeIterator++;
                } while(archetypeIterator != archetypeEnd && !queryMatch(query, archetypeIterator->type));
                chunkIterator = archetypeIterator == archetypeEnd ? typeof(chunkIterator){} : archetypeIterator->chunks.begin();
            }
            return *this;
        }

        inline Chunk& operator*() const {
            return **chunkIterator;
        }

    };

    struct QueryResult {
        QueryIterator beginIterator;
        QueryIterator endIterator;

        QueryIterator begin() {
            return beginIterator;
        }

        QueryIterator end() {
            return endIterator;
        }
    };

    QueryResult query(Registry& registry, const Query& query) {
        QueryIterator begin;
        begin.query = query;
        begin.archetypeIterator = registry.archetypes.begin();
        begin.archetypeEnd = registry.archetypes.end();

        while(begin.archetypeIterator != begin.archetypeEnd && !queryMatch(query, begin.archetypeIterator->type)) {
            begin.archetypeIterator++;
        }

        if (begin.archetypeIterator == begin.archetypeEnd) {
            begin.chunkIterator = {};
        } else {
            begin.chunkIterator = begin.archetypeIterator->chunks.begin();
        }

        QueryIterator end;
        end.query = query;
        end.archetypeIterator = registry.archetypes.end();
        end.archetypeEnd = registry.archetypes.end();
        end.chunkIterator = {};

        return {
            .beginIterator = begin,
            .endIterator = end
        };

    }


}

#endif //G2_QUERY_H
