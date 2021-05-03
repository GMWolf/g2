//
// Created by felix on 03/05/2021.
//

#ifndef G2_G2ARCHIVE_H
#define G2_G2ARCHIVE_H

#include <flatbuffers/flatbuffers.h>
#include <g2/archive/archive_generated.h>
#include <span>


namespace g2::archive {

    class ArchiveWriter {
        flatbuffers::FlatBufferBuilder fbb;
        std::vector<flatbuffers::Offset<Entry>> entries;
    public:
        ArchiveWriter();
        void addEntry(const char* path, std::span<uint8_t> data);

        std::span<uint8_t> finish();

    };
}

#endif //G2_G2ARCHIVE_H
