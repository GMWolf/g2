//
// Created by felix on 01/05/2021.
//

#include "ninja.h"

void write_rule(ninjactx &ctx, const char *name, const char *command) {
    ctx.out << "rule " << name << "\n  command = " << command << "\n";
}

void write_build(ninjactx &ctx, const char *ruleName, std::span<const char *> outputs, std::span<const char *> inputs, std::span<std::pair<const char*, const char*>> flags ) {

    ctx.out << "build";
    for(auto output : outputs) {
        ctx.out << " " << output;
    }

    ctx.out << ": " << ruleName;
    for(auto input : inputs) {
        ctx.out << " " << input;
    }
    ctx.out << "\n";
    for(auto flag : flags) {
        ctx.out << "  " << flag.first << " = " << flag.second << "\n";
    }

    ctx.out << "\n";

}
