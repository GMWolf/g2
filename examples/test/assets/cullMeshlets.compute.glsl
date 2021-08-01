#version 450

struct VkDrawIndexedIndirectCommand {
    uint32_t    indexCount;
    uint32_t    instanceCount;
    uint32_t    firstIndex;
    int32_t     vertexOffset;
    uint32_t    firstInstance;
};

layout(location = 0) writeonly buffer OutputCommands
{
    VkDrawIndexedIndirectCommand commands[];
};

struct Meshlet {
    uint indexCount;
    uint firstIndex;
    uint vertexOffset;
};

layout(location = 1) readonly buffer Meshlets
{
    Meshlet meshlets[];
};


layout (local_size_x = 64) in;
void main() {

    uint index = gl_GlobalInvocationID.x;

    Meshlet meshlet = meshlets[index];

    commands[index].indexCount = meshlet.indexCount;
    commands[index].instanceCount = 1;
    commands[index].firstIndex = meshlet.firstIndex;
    commands[index].vertexOffset = meshlet.vertexOffset;
    commands[index].firstInstance = index;

}
