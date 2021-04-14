//
// Created by felix on 14/04/2021.
//

#include "vk.h"
#include "validation.h"


#include <algorithm>
#include <cstring>

namespace g2::gfx
{

    static const char* validationLayers[] =
    {
#ifndef NDEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
    };


    bool checkValidationSupport()
    {
        auto[result, availableLayers] = vk::enumerateInstanceLayerProperties();

        for(const char* layerName : validationLayers)
        {
            auto f = std::find_if(availableLayers.begin(), availableLayers.end(),[=](const auto& layer) {
                return strcmp(layer.layerName, layerName) == 0;
            });

            if (f == availableLayers.end())
            {
                return false;
            }
        }

        return true;
    }

    std::span<const char *> getValidationLayerNames() {
#ifndef NDEBUG
        return validationLayers;
#else
        return {};
#endif
    }


}