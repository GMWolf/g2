//
// Created by felix on 13/04/2021.
//

#ifndef G2_GFX_INSTANCE_H
#define G2_GFX_INSTANCE_H
#include <g2/application.h>

#include <memory>
#include <span>

namespace g2::gfx {
struct InstanceConfig {
  Application *application;
  std::span<const char *> vkExtensions;
};

class Instance {
  struct Impl;
  std::unique_ptr<Impl> pImpl;

 public:
  explicit Instance(const InstanceConfig &config);
  ~Instance();

  void drawFrame();
};
}  // namespace g2::gfx
#endif  // G2_GFX_INSTANCE_H
