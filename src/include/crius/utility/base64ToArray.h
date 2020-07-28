#pragma once

#include <stdexcept>
#include <vector>

#include <QByteArray>

#include <agz/utility/misc.h>

// decode data is assumed to be of little endian
inline std::vector<float> base64ToFloatArray(const char *base64)
{
    const auto byteArray = QByteArray::fromBase64(base64);
    if(byteArray.size() % sizeof(float) != 0)
        throw std::runtime_error("invalid float byte array size");

    std::vector<float> ret;
    constexpr int BYTE_STEP = sizeof(float);
    for(int i = 0; i < byteArray.size(); i += BYTE_STEP)
    {
        const char chs[4] = {
            byteArray.at(i),
            byteArray.at(i + 1),
            byteArray.at(i + 2),
            byteArray.at(i + 3)
        };

        float value;
        std::memcpy(&value, chs, sizeof(float));

        ret.push_back(
            agz::misc::to_local_endian<
            agz::misc::endian_type::little>(value));
    }

    return ret;
}
