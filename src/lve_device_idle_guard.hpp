#pragma once

#include "lve_device.hpp"

namespace lve {
// Declare after resources referenced by frame submissions, before starting draws.
class LveDeviceIdleGuard {
 public:
  explicit LveDeviceIdleGuard(LveDevice &device) : device{device} {}
  ~LveDeviceIdleGuard() { vkDeviceWaitIdle(device.device()); }
  LveDeviceIdleGuard(const LveDeviceIdleGuard &) = delete;
  LveDeviceIdleGuard &operator=(const LveDeviceIdleGuard &) = delete;

 private:
  LveDevice &device;
};
}  // namespace lve
