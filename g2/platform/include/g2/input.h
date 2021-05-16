//
// Created by felix on 16/05/2021.
//

#ifndef G2_INPUT_H
#define G2_INPUT_H
#include "keyboard.h"
#include <cstdint>

namespace g2 {

    struct InputState {
        uint64_t keyPressedFrame[static_cast<int>(KEYS::COUNT)]{0};
        uint64_t keyReleaseFrame[static_cast<int>(KEYS::COUNT)]{0};
        uint64_t frame = 0;

        [[nodiscard]] bool keyDown(KEYS key) const {
            return keyReleaseFrame[static_cast<int>(key)] < keyPressedFrame[static_cast<int>(key)];
        }

        [[nodiscard]] bool keyUp(KEYS key) const {
            return !keyDown(key);
        }

        [[nodiscard]] bool keyPressed(KEYS key) const {
            return keyPressedFrame[static_cast<int>(key)] == frame;
        }

        [[nodiscard]] bool keyReleased(KEYS key) const {
            return keyReleaseFrame[static_cast<int>(key)] == frame;
        }

    };

}
#endif //G2_INPUT_H
