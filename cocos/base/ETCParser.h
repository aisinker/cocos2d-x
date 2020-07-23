#ifndef ETCPARSER_H
#define ETCPARSER_H

#include "platform/CCPlatformMacros.h"
#include <GLES3/gl3.h>

NS_CC_BEGIN

/**
     * \brief Class to extract information from the ETC headers of compressed textures.
     */
class ETCParser
{
private:
    uint32_t width;
    uint32_t height;
    GLenum format;
    size_t textureDataOffset;
public:

    static const uint32_t SIZE = 16;

    ETCParser();

    ETCParser(const unsigned char *data);

    bool is_valid();

    uint32_t getWidth();

    uint32_t getHeight();

    GLenum getFormat();

    size_t getTextureDataOffset();
};

NS_CC_END
#endif /* ETCPARSER_H */