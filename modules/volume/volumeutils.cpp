#include "volumeutils.h"

namespace openspace {
namespace volumeutils {
    
size_t coordsToIndex(const glm::uvec3& coords, const glm::uvec3& dims) {
    size_t w = dims.x;
    size_t h = dims.y;
    size_t d = dims.z;
    
    size_t x = coords.x;
    size_t y = coords.y;
    size_t z = coords.z;
    
    return coords.z * (h * w) + coords.y * w + coords.x;
}

glm::uvec3 indexToCoords(size_t index, const glm::uvec3& dims) {
    size_t w = dims.x;
    size_t h = dims.y;
    size_t d = dims.z;
    
    size_t x = index % w;
    size_t y = (index / w) % h;
    size_t z = index / w / h;
    
    return glm::uvec3(x, y, z);
}
    
}
}
