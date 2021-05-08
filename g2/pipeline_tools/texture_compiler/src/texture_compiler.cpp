//
// Created by felix on 26/04/2021.
//

#include <iostream>
#include <bc7e_ispc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <vector>

#include <g2/gfx/image_generated.h>
#include <fstream>

#include <zstd.h>

namespace fs = std::filesystem;
namespace fb = flatbuffers;


void getBlock(stbi_uc* data, int bx, int by, int imageWidth, uint32_t* out) {
    for(uint32_t y = 0; y < 4; y++) {
        memcpy(out + y * 4, data + (((by * 4) * imageWidth + (bx * 4)) * 4), 4 * sizeof(uint32_t));
    }
}


int main(int argc, char* argv[]) {

    if(argc < 3) {
        std::cerr << "Expected 2 arguments\n";
        return 1;
    }

    fs::path input = argv[1];
    fs::path output = argv[2];

    int imageWidth, imageHeight, comp;
    stbi_uc* data = stbi_load(input.c_str(), &imageWidth, &imageHeight, &comp, 4);

    ispc::bc7e_compress_block_init();

    ispc::bc7e_compress_block_params bc7e_params {} ;

    ispc::bc7e_compress_block_params_init_basic(&bc7e_params, false); //TODO enable perceptual for colour information

    uint32_t blocksX = imageWidth / 4;
    uint32_t blocksY = imageHeight / 4;

    const size_t blockByteSize = 16;

    std::vector<uint8_t> packedImage(blocksX * blocksY * blockByteSize);

    for(int32_t by = 0; by < blocksY; by++) {

        const int N = 64; //64 blocks at a time for better simd

        for(uint32_t bx = 0; bx < blocksX; bx += N) {

            uint32_t numBlocks = std::min<uint32_t>(blocksX - bx, N);

            uint32_t pixels[16 * N];

            for(uint32_t b = 0; b < numBlocks; b++) {
                getBlock(data, bx + b, by, imageWidth, pixels + b * 16);
            }

            uint64_t* packedBlock = reinterpret_cast<uint64_t*>(packedImage.data() + ((bx + by * blocksX) * blockByteSize));
            ispc::bc7e_compress_blocks(numBlocks, packedBlock, pixels, &bc7e_params);
        }
    }

    stbi_image_free(data);

    //Compress the data with zstd
    std::vector<uint8_t> compressedImage(ZSTD_compressBound(packedImage.size()));
    auto compressedSize = ZSTD_compress(compressedImage.data(), compressedImage.size(), packedImage.data(), packedImage.size(), ZSTD_maxCLevel());
    compressedImage.resize(compressedSize);

    fb::FlatBufferBuilder fbb(packedImage.size() * blockByteSize + 2048);

    auto image = g2::gfx::CreateImageDefDirect(fbb, imageWidth, imageHeight, 1, 1, g2::gfx::Format::bc7_srgb, &compressedImage); // TODO non srgb textures

    g2::gfx::FinishImageDefBuffer(fbb, image);

    {
        auto buf = fbb.GetBufferPointer();
        auto buf_size = fbb.GetSize();
        std::ofstream ofs(output.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char*)buf, buf_size);
    }


    return 0;
}