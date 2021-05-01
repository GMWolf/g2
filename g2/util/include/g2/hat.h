//
// Created by felix on 01/05/2021.
//

#ifndef G2_HAT_H
#define G2_HAT_H

#include <vector>
#include <array>
#include <bitset>
#include <type_traits>
#include <memory>

namespace g2 {

    template<class T, size_t CHUNK_SIZE = 64>
    class hat {
        struct chunk {
            std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, CHUNK_SIZE> array;
            std::bitset<CHUNK_SIZE> initialized;
        };

        std::vector<std::unique_ptr<chunk>> chunks {};
        size_t indexChunkIndex(size_t index);
        size_t indexItemIndex(size_t index);
    public:

        bool contains(size_t index);

        template<typename... Args>
        bool emplace(size_t index, Args&&... t);

        T& operator[](size_t index);

    };

    template<class T, size_t CHUNK_SIZE>
    bool hat<T, CHUNK_SIZE>::contains(size_t index) {
        const size_t chunkIndex = indexChunkIndex(index);
        const size_t itemIndex = indexItemIndex(index);
        if (chunks.size() <= chunkIndex || !chunks[chunkIndex]) {
            return false;
        }

        return chunks[chunkIndex]->initialized(itemIndex);
    }

    template<class T, size_t CHUNK_SIZE>
    size_t hat<T, CHUNK_SIZE>::indexChunkIndex(size_t index) {
        return index / CHUNK_SIZE;
    }

    template<class T, size_t CHUNK_SIZE>
    size_t hat<T, CHUNK_SIZE>::indexItemIndex(size_t index) {
        return index % CHUNK_SIZE;
    }

    template<class T, size_t CHUNK_SIZE>
    template<typename... Args>
    bool hat<T, CHUNK_SIZE>::emplace(size_t index, Args&&... args) {
        const size_t chunkIndex = indexChunkIndex(index);
        const size_t itemIndex = indexItemIndex(index);

        if (chunks.size() <= chunkIndex) {
            chunks.resize(chunkIndex + 1);
        }

        if (!chunks[chunkIndex]) {
            //TODO add freelist
            //if (freelist) {
            //    chunks[chunkIndex] = std::move(freelist);
            //    freelist = std::move(chunks[chunkIndex]->next);
            //} else {
                chunks[chunkIndex] = std::make_unique<chunk>();
            //}
        }

        if (chunks[chunkIndex]->initialized.test(itemIndex)){
            return false;
        }

        new(&chunks[chunkIndex]->array[itemIndex]) T(std::forward<Args>(args)...);
        chunks[chunkIndex]->initialized.set(itemIndex);

        return true;
    }

    template<class T, size_t CHUNK_SIZE>
    T &hat<T, CHUNK_SIZE>::operator[](size_t index) {
        const size_t chunkIndex = indexChunkIndex(index);
        const size_t itemIndex = indexItemIndex(index);
        return *reinterpret_cast<T*>(&chunks[chunkIndex]->array[itemIndex]);
    }


}

#endif //G2_HAT_H
