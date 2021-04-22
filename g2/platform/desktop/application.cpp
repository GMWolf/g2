//
// Created by felix on 13/04/2021.
//

#include <g2/application.h>
#include <volk.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

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
  glfwSetWindowUserPointer(pImpl->window, pImpl.get());
}

Application::~Application() {
  glfwDestroyWindow(pImpl->window);
  glfwTerminate();
};

bool Application::shouldClose() const {
  return glfwWindowShouldClose(pImpl->window);
}

void Application::pollEvents() const { glfwPollEvents(); }

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

std::span<const char *> getVkExtensions() {
  uint32_t count;
  const char **str = glfwGetRequiredInstanceExtensions(&count);
  return {str, count};
}

}  // namespace g2