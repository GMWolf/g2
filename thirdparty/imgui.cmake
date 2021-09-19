add_library(imgui
        imgui/imgui.h
        imgui/imgui.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_tables.cpp)
target_include_directories(imgui PUBLIC imgui)

add_library(imgui_impl
        imgui/backends/imgui_impl_glfw.h
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_vulkan.h
        imgui/backends/imgui_impl_vulkan.cpp
        )
target_include_directories(imgui PUBLIC imgui/backends)
target_link_libraries(imgui_impl PUBLIC imgui vulkan glfw)