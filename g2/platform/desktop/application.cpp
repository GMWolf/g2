//
// Created by felix on 13/04/2021.
//

#include <g2/application.h>
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <iostream>

#include <imgui_impl_glfw.h>

namespace g2 {

    struct Application::Impl {
        GLFWwindow *window;
        int width;
        int height;
    };

    static void glfwErrorCallback(int error, const char *description) {
        std::cerr << "GLFW error: " << description << "\n";
    }

    static bool initialize_glfw() {
        glfwSetErrorCallback(glfwErrorCallback);
        return glfwInit();
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));

        switch (action) {
            case GLFW_PRESS:
                app->inputState.keyPressedFrame[key] = app->inputState.frame;
                break;
            case GLFW_RELEASE:
                app->inputState.keyReleaseFrame[key] = app->inputState.frame;
                break;
            default:
                break;
        }
    }

    Application::Application(const ApplicationConfiguration &appConfig) {
        if (!initialize_glfw()) {
            std::cerr << "Failed to initialize glfw\n";
            return;
        }

        pImpl = std::make_unique<Impl>();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        pImpl->window = glfwCreateWindow(appConfig.width, appConfig.height,
                                         appConfig.title, nullptr, nullptr);

        if (!pImpl->window) {
            std::cerr << "Failed to create window\n";
            return;
        }

        glfwGetWindowSize(pImpl->window, &pImpl->width, &pImpl->height);
        glfwSetKeyCallback(pImpl->window, key_callback);
        glfwSetWindowUserPointer(pImpl->window, this);
    }

    Application::~Application() {
        glfwDestroyWindow(pImpl->window);
        glfwTerminate();
    };

    bool Application::shouldClose() const {
        return glfwWindowShouldClose(pImpl->window);
    }

    void Application::pollEvents() {
        inputState.frame++;
        glfwPollEvents();
    }

    VkSurfaceKHR Application::createSurface(VkInstance instance) {
        VkSurfaceKHR surface{};
        glfwCreateWindowSurface(instance, pImpl->window, nullptr, &surface);
        return surface;
    }

    glm::ivec2 Application::getWindowSize() {
        int width, height;
        glfwGetWindowSize(pImpl->window, &width, &height);
        return glm::ivec2(width, height);
    }

    double Application::getTime() const{
        return glfwGetTime();
    }

    void Application::initImgui() {
        ImGui_ImplGlfw_InitForVulkan(pImpl->window, true);
    }

    void Application::imguiNewFrame() {
        ImGui_ImplGlfw_NewFrame();
    }

    std::span<const char *> getVkExtensions() {
        uint32_t count;
        const char **str = glfwGetRequiredInstanceExtensions(&count);
        return {str, count};
    }

}  // namespace g2