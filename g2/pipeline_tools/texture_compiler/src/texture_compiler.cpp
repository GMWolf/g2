//
// Created by felix on 26/04/2021.
//

#include <iostream>
#include <bc7e_ispc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include <filesystem>
#include <vector>

#include <g2/gfx/image_generated.h>
#include <fstream>

#include <zstd.h>
#include <span>

namespace fs = std::filesystem;
namespace fb = flatbuffers;


void getBlock(stbi_uc* data, int bx, int by, int imageWidth, uint32_t* out) {
    for(uint32_t y = 0; y < 4; y++) {
        memcpy(out + y * 4, data + (((by * 4 + y) * imageWidth + (bx * 4)) * 4), 4 * sizeof(uint32_t));
    }
}

std::vector<uint8_t> compressImageData(stbi_uc* imageData, uint32_t imageWidth, uint32_t imageHeight, const ispc::bc7e_compress_block_params& bc7e_params) {

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
                getBlock(imageData, bx + b, by, imageWidth, pixels + b * 16);
            }

            auto* packedBlock = reinterpret_cast<uint64_t*>(packedImage.data() + ((bx + by * blocksX) * blockByteSize));
            ispc::bc7e_compress_blocks(numBlocks, packedBlock, pixels, &bc7e_params);
        }
    }


    //Compress the data with zstd
    std::vector<uint8_t> compressedImage(ZSTD_compressBound(packedImage.size()));
    auto compressedSize = ZSTD_compress(compressedImage.data(), compressedImage.size(), packedImage.data(), packedImage.size(), ZSTD_maxCLevel());
    compressedImage.resize(compressedSize);

    return compressedImage;
}


int main(int argc, char* argv[]) {

    if(argc < 3) {
        std::cerr << "Expected 2 arguments\n";
        return 1;
    }

    fs::path input = argv[1];
    fs::path output = argv[2];

    int imageWidth, imageHeight, comp;

    stbi_uc localImage[16];


    stbi_uc* imageData = stbi_load(input.c_str(), &imageWidth, &imageHeight, &comp, 4);

    if (imageWidth == 1 && imageHeight == 1) {
        for(int i = 0; i < 16; i++) {
            localImage[i] = imageData[i % 4];
        }
        stbi_image_free(imageData);
        imageData = localImage;
        imageWidth = 4;
        imageHeight = 4;
    }

    ispc::bc7e_compress_block_init();

    ispc::bc7e_compress_block_params bc7e_params {} ;

    ispc::bc7e_compress_block_params_init_basic(&bc7e_params, false); //TODO enable perceptual for colour information

    uint32_t numLevels = std::log2(std::max(imageWidth, imageHeight));
    numLevels = std::max(1u, numLevels - 2); //We want to have a 4x4 smallest mip

    uint32_t mipWidth = imageWidth;
    uint32_t mipHeight = imageHeight;

    fb::FlatBufferBuilder fbb(2048);

    unsigned char* outImageData = (unsigned char*)malloc(imageWidth * imageHeight * 4);

    std::vector<fb::Offset<g2::gfx::ImageMip>> mips;
    mips.reserve(numLevels);

    for(int i = 0; i < numLevels; i++) {
        auto compressedData = compressImageData(imageData, mipWidth, mipHeight, bc7e_params);
        assert(ZSTD_getDecompressedSize(compressedData.data(), compressedData.size()));
        mips.push_back(g2::gfx::CreateImageMipDirect(fbb, &compressedData));

        stbir_resize_uint8_srgb(imageData, mipWidth, mipHeight, 0,
                                outImageData,std::max(4u, mipWidth / 2u), std::max(4u, mipHeight / 2u), 0,
                                4, 3, 0);
        mipWidth = std::max(4u, mipWidth / 2u); //we want minimum 4x4 blocks
        mipHeight = std::max(4u, mipHeight / 2u);

        std::swap(imageData, outImageData);
    }

    auto packedImage = compressImageData(imageData, imageWidth, imageHeight, bc7e_params);

    if (outImageData != localImage) {
        free(outImageData);
    }
    if (imageData != localImage) {
        stbi_image_free(imageData);
    }


    auto image = g2::gfx::CreateImageDefDirect(fbb, imageWidth, imageHeight, 1, numLevels, g2::gfx::Format::bc7_srgb, &mips); // TODO non srgb textures

    g2::gfx::FinishImageDefBuffer(fbb, image);

    {
        auto buf = fbb.GetBufferPointer();
        auto buf_size = fbb.GetSize();
        std::ofstream ofs(output.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char*)buf, buf_size);
    }


    return 0;
}