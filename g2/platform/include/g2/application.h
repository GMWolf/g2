//
// Created by felix on 13/04/2021.
//

#ifndef G2_APPLICATION_H
#define G2_APPLICATION_H

#include <glm/glm.hpp>
#include <memory>
#include <span>
#include "input.h"

struct VkSurfaceKHR_T;
typedef VkSurfaceKHR_T *VkSurfaceKHR;

struct VkInstance_T;
typedef VkInstance_T *VkInstance;

namespace g2 {
    struct ApplicationConfiguration {
        int width;
        int height;
        const char *title;
    };

    class Application {
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    public:
        InputState inputState;

        double getTime() const;

        explicit Application(const ApplicationConfiguration &appConfig);

        ~Application();

        [[nodiscard]] bool shouldClose() const;

        void pollEvents();

        glm::ivec2 getWindowSize();

        VkSurfaceKHR createSurface(VkInstance instance);
    };

    std::span<const char *> getVkExtensions();
}  // namespace g2

#endif  // G2_APPLICATION_H
