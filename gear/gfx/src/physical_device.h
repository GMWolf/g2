//
// Created by felix on 14/04/2021.
//

#ifndef G2_PHYSICAL_DEVICE_H
#define G2_PHYSICAL_DEVICE_H

#include "vk.h"
#include <optional>

namespace g2::gfx
{

    std::optional<vk::PhysicalDevice> pickPhysicalDevice(vk::Instance& instance);

}


#endif //G2_PHYSICAL_DEVICE_H
