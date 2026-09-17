#ifndef __TIMER_H_INCLUDED__
#define __TIMER_H_INCLUDED__

#include <functional>

#include "core/Device.h"

struct Timing {
  double medianMs = 0.0;
  double p95Ms = 0.0;
  double cpuMs = 0.0;
};

Timing timePass(const Device& dev, std::uint32_t warmup, std::uint32_t frames,
                const std::function<void(VkCommandBuffer)>& record);

#endif
