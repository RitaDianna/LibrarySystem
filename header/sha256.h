#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

class SHA256 {
public:
    static std::string hash(const std::string& input);

private:
    static uint32_t rotr(const uint32_t x, const uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }
};

#endif //SHA256_H