//
// Created by felix on 14/04/2021.
//

#ifndef G2_VALIDATION_H
#define G2_VALIDATION_H

#include <span>

namespace g2::gfx {
std::span<const char *> getValidationLayerNames();
bool checkValidationSupport();
}  // namespace g2::gfx

#endif  // G2_VALIDATION_H
