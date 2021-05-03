//
// Created by felix on 03/05/2021.
//

#include <g2archive.h>
#include <flatbuffers/hash.h>

g2::archive::ArchiveWriter::ArchiveWriter() : fbb(1000){
}

void g2::archive::ArchiveWriter::addEntry(const char* path, std::span<uint8_t> data) {
    auto dataOffset = fbb.CreateVector(data.data(), data.size());
    auto pathOffset = fbb.CreateString(path);
    auto id = flatbuffers::HashFnv1<uint64_t>(path);

    entries.push_back(CreateEntry(fbb, id, pathOffset, dataOffset));
}

std::span<uint8_t> g2::archive::ArchiveWriter::finish() {
    auto a = CreateArchiveDirect(fbb, &entries);
    FinishArchiveBuffer(fbb, a);

    return {fbb.GetBufferPointer(), fbb.GetSize()};
}


