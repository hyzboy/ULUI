#include "InputStream.h"
#include <algorithm>

namespace ului {

int64_t InputStream::Skip(int64_t n) {
    if (n <= 0) {
        return 0;
    }
    
    int64_t remaining = n;
    const size_t BUFFER_SIZE = 2048;
    uint8_t buffer[BUFFER_SIZE];
    
    while (remaining > 0) {
        size_t toRead = static_cast<size_t>(std::min(remaining, static_cast<int64_t>(BUFFER_SIZE)));
        int bytesRead = Read(buffer, 0, toRead);
        
        if (bytesRead < 0) {
            break;
        }
        
        remaining -= bytesRead;
        
        if (bytesRead < static_cast<int>(toRead)) {
            break;
        }
    }
    
    return n - remaining;
}

int InputStream::Available() {
    return 0;
}

} // namespace ului
