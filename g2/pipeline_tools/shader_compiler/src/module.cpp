//
// Created by felix on 24/06/2021.
//

#include "module.h"
#include <shaderc/shaderc.hpp>
#include <fstream>
#include <memory>
#include <iostream>

namespace fs = std::filesystem;

struct ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {

    std::vector<fs::path>& includes;

    ShaderIncluder(std::vector<fs::path>& includes) : includes(includes) {
    }

    struct ResultContainer {
        shaderc_include_result result;
        std::string sourceName;
        std::string contents;
    };

    shaderc_include_result *
    GetInclude(const char *requested_source, shaderc_include_type type, const char *requesting_source,
               size_t include_depth) override {
        auto path = fs::path(requesting_source).parent_path() / requested_source;
        includes.push_back(path);
        std::ifstream stream(path);

        auto result = new ResultContainer;
        result->sourceName = path.filename().string();

        if (stream) {
            result->contents = std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            result->result.content = result->contents.c_str();
            result->result.content_length = result->contents.length();
            result->result.source_name = result->sourceName.c_str();
            result->result.source_name_length = result->sourceName.length();
        } else {
            result->result.content = nullptr;
            result->result.content_length = 0;
            result->result.source_name = nullptr;
            result->result.source_name_length = 0;
        }

        result->result.user_data = result;


        return &result->result;
    }

    void ReleaseInclude(shaderc_include_result *data) override {
        delete (ResultContainer *) data->user_data;
    }
};


shaderc_shader_kind getShaderKind(g2::gfx::ShaderStage stage) {
    switch (stage) {
        case g2::gfx::ShaderStage::vertex:
            return shaderc_vertex_shader;
        case g2::gfx::ShaderStage::fragment:
            return shaderc_fragment_shader;
        case g2::gfx::ShaderStage::compute:
            return shaderc_compute_shader;
    }
    std::cerr << "Error, unknown shader stage " << (int)stage << std::endl;
    exit(1);
}

std::vector <uint32_t> compileModule(const char *text, const char* filename, g2::gfx::ShaderStage shaderStage, std::vector <fs::path> &depsVec) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetIncluder(std::make_unique<ShaderIncluder>(depsVec));
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
    //options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto vertexPreprocessResult = compiler.PreprocessGlsl(text, getShaderKind(shaderStage), filename, options);
    if (vertexPreprocessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << vertexPreprocessResult.GetErrorMessage() << "\n";
        return {};
    }
    auto vertexResult = compiler.CompileGlslToSpv({vertexPreprocessResult.cbegin(), vertexPreprocessResult.cend()},
                                                  getShaderKind(shaderStage), filename, options);
    if (vertexResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << vertexResult.GetErrorMessage() << "\n";
        return {};
    }

    return std::vector<uint32_t>(vertexResult.cbegin(), vertexResult.cend());
}
