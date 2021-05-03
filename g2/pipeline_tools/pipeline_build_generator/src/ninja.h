//
// Created by felix on 01/05/2021.
//

#ifndef G2_NINJA_H
#define G2_NINJA_H

#include <ostream>
#include <span>
#include <filesystem>
#include <utility>

struct ninjactx {
    std::ostream& out;
    std::filesystem::path buildDir;
    std::filesystem::path sourceDir;
};

void write_rule(ninjactx& ctx, const char* name, const char* command);
void write_build(ninjactx& ctx,  const char* ruleName, std::span<const char*> outputs, std::span<const char*> inputs,
                 std::span<std::pair<const char*, const char*>> flags = {});


#endif //G2_NINJA_H
