#include "core/Device.h"
#include "utils/Vertex.h"

#include <cstdlib>
#include <iostream>

int main() {
  try {
    Device dev = createDevice(nullptr);

    std::cout << "  device      " << dev.props.deviceName << "\n";
    std::cout << "  api         " << VK_API_VERSION_MAJOR(dev.props.apiVersion) << "."
              << VK_API_VERSION_MINOR(dev.props.apiVersion) << "."
              << VK_API_VERSION_PATCH(dev.props.apiVersion) << "\n";
    std::cout << "  subgroup    " << dev.subgroupSize << " lanes\n";

    if (dev.hasTimestamps)
      std::cout << "  timestamps  " << dev.props.limits.timestampPeriod << " ns per tick\n";
    else
      std::cout << "  timestamps  UNSUPPORTED: this driver cannot measure Parts II to VI,\n"
                << "              see 'If it does not work' in the handout\n";

    std::cout << "  statistics  pipeline statistics "
              << (dev.hasPipelineStats ? "supported" : "unsupported") << "\n";
    std::cout << "  student.cpp " << TRIANGLE.size() << " vertices, stride "
              << Vertex::bindingDescription().stride << ", "
              << Vertex::attributeDescriptions().size() << " attributes\n";

    if (TRIANGLE.empty())
      std::cout << "              TASK 1 incomplete: TRIANGLE is empty\n";

    destroyDevice(dev);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
