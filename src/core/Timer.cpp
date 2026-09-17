#include "core/Timer.h"

#include <algorithm>
#include <chrono>

static double submitOnce(const Device& dev,
                         const std::function<void(VkCommandBuffer)>& record,
                         double* cpuMs = nullptr) {
  std::chrono::steady_clock::time_point begun =
      std::chrono::steady_clock::now();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  check(vkBeginCommandBuffer(dev.cmd, &beginInfo), "vkBeginCommandBuffer");

  vkCmdResetQueryPool(dev.cmd, dev.queries, 0, 2);
  vkCmdWriteTimestamp(dev.cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dev.queries,
                      0);
  record(dev.cmd);
  vkCmdWriteTimestamp(dev.cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      dev.queries, 1);
  check(vkEndCommandBuffer(dev.cmd), "vkEndCommandBuffer");

  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &dev.cmd;
  check(vkQueueSubmit(dev.gfxQueue, 1, &submit, VK_NULL_HANDLE),
        "vkQueueSubmit");
  check(vkQueueWaitIdle(dev.gfxQueue), "vkQueueWaitIdle");

  std::uint64_t stamps[2] = {0, 0};
  VkResult read = vkGetQueryPoolResults(
      dev.device, dev.queries, 0, 2, sizeof(stamps), stamps,
      sizeof(std::uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
  if (cpuMs != nullptr) {
    std::chrono::duration<double, std::milli> elapsed =
        std::chrono::steady_clock::now() - begun;
    *cpuMs = elapsed.count();
  }
  if (read != VK_SUCCESS) return 0.0;

  double ns = static_cast<double>(stamps[1] - stamps[0]) *
              dev.props.limits.timestampPeriod;
  return ns / 1.0e6;
}

Timing timePass(const Device& dev, std::uint32_t warmup, std::uint32_t frames,
                const std::function<void(VkCommandBuffer)>& record) {
  for (std::uint32_t i = 0; i < warmup; ++i) submitOnce(dev, record);

  std::vector<double> samples;
  std::vector<double> cpuSamples;
  samples.reserve(frames);
  cpuSamples.reserve(frames);
  for (std::uint32_t i = 0; i < frames; ++i) {
    double cpu = 0.0;
    samples.push_back(submitOnce(dev, record, &cpu));
    cpuSamples.push_back(cpu);
  }
  std::sort(samples.begin(), samples.end());
  std::sort(cpuSamples.begin(), cpuSamples.end());

  Timing timing{};
  if (samples.empty()) return timing;

  timing.medianMs = samples[samples.size() / 2];
  timing.cpuMs = cpuSamples[cpuSamples.size() / 2];
  timing.p95Ms = samples[std::min(
      samples.size() - 1, static_cast<std::size_t>(samples.size() * 0.95))];
  return timing;
}
