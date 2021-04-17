flatc -b ../../g2/gfx/flatbufferschemas/pipeline.fbs pipeline.json
flatc -b ../../g2/gfx/flatbufferschemas/pipeline.fbs pipeline2.json
glslc -o vert.spv src/shader.vert
glslc -o vert2.spv src/shader2.vert
glslc -o frag.spv src/shader.frag
