//
// Created by felix on 13/04/2021.
//

#ifndef G2_APPLICATION_H
#define G2_APPLICATION_H
#include <memory>
#include <span>
namespace g2
{
    struct ApplicationConfiguration
    {
        int width;
        int height;
        const char* title;
    };

    class Application
    {
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    public:

        explicit Application(const ApplicationConfiguration& appConfig);
        ~Application();

        [[nodiscard]] bool shouldClose() const;
        void pollEvents() const;
    };

    std::span<const char*> getVkExtensions();

}

#endif //G2_APPLICATION_H
