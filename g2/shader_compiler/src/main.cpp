//
// Created by felix on 19/04/2021.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <flatbuffers/flatbuffers.h>
#include <g2/gfx/pipeline_generated.h>
#include <rapidjson/document.h>
#include <g2/file.h>
#include <shaderc/shaderc.hpp>
#include <g2/fbutil.h>
#include <flatbuffers/util.h>
#include <regex>

namespace fs = std::filesystem;
namespace json = rapidjson;





int main(int argc, char* argv[]) {

  if(argc != 3) {
    std::cerr << "Wrong number of arguments received. 2 expected.\n";
    return 1;
  }

  fs::path inputPath(argv[1]);
  fs::path outputPath(argv[2]);

  fs::path inputDir = inputPath.parent_path();

  json::Document doc;
  std::string docText;
  g2::readFile(inputPath.c_str(), docText);
  doc.ParseInsitu(docText.data());

  shaderc::Compiler compiler;
  shaderc::CompileOptions options;


  auto& shader = doc["shader"];
  auto vertexFile =  inputDir / shader["vertex"].GetString();
  std::string vertexSource;
  g2::readFile(vertexFile.c_str(), vertexSource);
  auto vertexResult = compiler.CompileGlslToSpv(vertexSource, shaderc_vertex_shader, vertexFile.c_str(), options);
  if(vertexResult.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::cerr << vertexResult.GetErrorMessage() << "\n";
    return -1;
  }
  size_t vertexWordSize = std::distance(vertexResult.cbegin(), vertexResult.cend());

  auto fragmentFile = inputDir / shader["fragment"].GetString();
  std::string fragmentSource;
  g2::readFile(fragmentFile.c_str(), fragmentSource);
  auto fragmentResult = compiler.CompileGlslToSpv(fragmentSource, shaderc_fragment_shader, fragmentFile.c_str(), options);
  if(fragmentResult.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::cerr << fragmentResult.GetErrorMessage() << "\n";
    return -1;
  }
  size_t fragmentWordSize = std::distance(fragmentResult.cbegin(), fragmentResult.cend());


  size_t shaderByteSize = sizeof(uint32_t) * (vertexWordSize + fragmentWordSize);

  flatbuffers::FlatBufferBuilder fbb(shaderByteSize + 1000);


  auto fbVertexText = fbb.CreateVector(vertexResult.cbegin(), vertexWordSize);
  auto fbVertexModule = g2::gfx::CreateShaderModule(fbb, fbVertexText, g2::gfx::ShaderStage::vertex);
  auto fbFragmentText = fbb.CreateVector(fragmentResult.cbegin(), fragmentWordSize);
  auto fbFragmentModule = g2::gfx::CreateShaderModule(fbb, fbFragmentText, g2::gfx::ShaderStage::fragment);


  std::vector<flatbuffers::Offset<g2::gfx::ShaderModule>> fbmodules {
      fbVertexModule, fbFragmentModule
  };


  auto& blending = doc["blending"];
  assert(blending.IsArray());
  auto& attachmentsValue = doc["attachments"];
  assert(attachmentsValue.IsArray());

  std::vector<g2::gfx::BlendAttachment> blend_attachments(blending.Size());
  for(size_t i = 0; i < blending.Size(); i++) {

    auto& b = blending[i];

    g2::gfx::BlendAttachment blend_attachment(
      b["blend_enable"].GetBool(),
      static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["src_blend_factor"].GetString(), g2::gfx::BlendFactorTypeTable()).value()),
      static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["dst_blend_factor"].GetString(), g2::gfx::BlendFactorTypeTable()).value()),
      static_cast<g2::gfx::BlendOp>(g2::enumLookup(b["color_blend_op"].GetString(), g2::gfx::BlendOpTypeTable()).value()),
      static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["src_alpha_factor"].GetString(), g2::gfx::BlendFactorTypeTable()).value()),
      static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["dst_alpha_factor"].GetString(), g2::gfx::BlendFactorTypeTable()).value()),
      static_cast<g2::gfx::BlendOp>(g2::enumLookup(b["alpha_blend_op"].GetString(), g2::gfx::BlendOpTypeTable()).value())
    );

    blend_attachments[i] = blend_attachment;
  }


  std::vector<g2::gfx::Attachment> attachments(attachmentsValue.Size());
  for(size_t i = 0; i < attachmentsValue.Size(); i++) {
    auto formatStr = attachmentsValue[i]["format"].GetString();

    g2::gfx::Format format = static_cast<g2::gfx::Format>(g2::enumLookup(formatStr, g2::gfx::FormatTypeTable()).value());

    attachments[i] = g2::gfx::Attachment(format);
  }

  auto fbpipeline = g2::gfx::CreatePipelineDefDirect(fbb, &fbmodules, &blend_attachments, &attachments);
  fbb.Finish(fbpipeline);

  {
    auto buf = fbb.GetBufferPointer();
    auto buf_size = fbb.GetSize();
    std::ofstream ofs(outputPath.c_str(), std::ios::out | std::ios::binary);
    ofs.write((char*)buf, buf_size);
  }

  //Write dep file
  {
    std::ofstream ofs(outputPath.string() + ".d" );
    std::regex whitespaceRegex("\\s");
    auto escapedVertexPath = std::regex_replace(vertexFile.string(), whitespaceRegex, "\\$&");
    auto escapedFragmentPath = std::regex_replace(fragmentFile.string(), whitespaceRegex, "\\$&");
    ofs << outputPath.c_str() << " : " << escapedVertexPath << " " << escapedFragmentPath;
  }

}