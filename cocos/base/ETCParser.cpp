#include "ETCParser.h"
#include <string.h>

#define GL_ETC1_RGB8_OES    0x8D64

NS_CC_BEGIN

    static bool is_valid_pkm(const unsigned char *data) {
        static const char kMagicPkm10[] = { 'P', 'K', 'M', ' ', '1', '0' };
        static const char kMagicPkm20[] = { 'P', 'K', 'M', ' ', '2', '0' };
        bool is_pkm10 = memcmp(data, kMagicPkm10, sizeof(kMagicPkm10)) == 0;
        bool is_pkm20 = memcmp(data, kMagicPkm20, sizeof(kMagicPkm20)) == 0;
        // TODO more check
        return is_pkm10 || is_pkm20;
    }

    static bool is_valid_ktx(const unsigned char *data) {
        static const uint8_t fileIdentifier[12] = {
                0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
        };
        // TODO more check
        return memcmp(data, fileIdentifier, sizeof(fileIdentifier)) == 0;
    }

    static uint32_t reverseBytesOfUint32(uint32_t n) {
        return ((n & 0x000000FF) << 24)|((n & 0x0000FF00) << 8)
        |((n & 0x00FF0000) >> 8)|((n & 0xFF000000) >> 24);
    }

    ETCParser::ETCParser():
    width(0),
    height(0),
    textureDataOffset(0),
    format(GL_INVALID_VALUE)
    {}

    ETCParser::ETCParser(const uint8_t *data)
    {
        if (is_valid_pkm(data)){
            uint8_t formatIdentifierMSB = data[6];
            uint8_t formatIdentifierLSB = data[7];
            uint8_t widthMSB            = data[12];
            uint8_t widthLSB            = data[13];
            uint8_t heightMSB           = data[14];
            uint8_t heightLSB           = data[15];

            width = (widthMSB << 8) | widthLSB;
            height = (heightMSB << 8) | heightLSB;
            textureDataOffset = 16;

            // From: https://github.com/Ericsson/ETCPACK/blob/master/source/etcpack.cxx
            // In a PKM-file, the codecs are stored using the following identifiers
            //
            // identifier                         value               codec
            // --------------------------------------------------------------------
            // ETC1_RGB_NO_MIPMAPS                  0                 GL_ETC1_RGB8_OES
            // ETC2PACKAGE_RGB_NO_MIPMAPS           1                 GL_COMPRESSED_RGB8_ETC2
            // ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD      2, not used       -
            // ETC2PACKAGE_RGBA_NO_MIPMAPS          3                 GL_COMPRESSED_RGBA8_ETC2_EAC
            // ETC2PACKAGE_RGBA1_NO_MIPMAPS         4                 GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
            // ETC2PACKAGE_R_NO_MIPMAPS             5                 GL_COMPRESSED_R11_EAC
            // ETC2PACKAGE_RG_NO_MIPMAPS            6                 GL_COMPRESSED_RG11_EAC
            // ETC2PACKAGE_R_SIGNED_NO_MIPMAPS      7                 GL_COMPRESSED_SIGNED_R11_EAC
            // ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS     8                 GL_COMPRESSED_SIGNED_RG11_EAC
            static const GLenum IdentifierToFormat[] = {
                    GL_COMPRESSED_RGB8_ETC2,
                    GL_COMPRESSED_RGB8_ETC2,
                    GL_INVALID_VALUE,
                    GL_COMPRESSED_RGBA8_ETC2_EAC,
                    GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                    GL_COMPRESSED_R11_EAC,
                    GL_COMPRESSED_RG11_EAC,
                    GL_COMPRESSED_SIGNED_R11_EAC,
                    GL_COMPRESSED_SIGNED_RG11_EAC
            };

            unsigned short format_identifier = (formatIdentifierMSB << 8) | formatIdentifierLSB;
            if (format_identifier > sizeof(IdentifierToFormat)/sizeof(IdentifierToFormat[0])-1){
                format = GL_INVALID_VALUE;
            }else {
                format = IdentifierToFormat[format_identifier];
            }
        } else if (is_valid_ktx(data)) {
            int32_t endianness  = *((int32_t*)&data[12]);
            if (endianness == 0x04030201) {
                format = *((uint32_t*)&data[28]);
                width = *((uint32_t*)&data[36]);
                height = *((uint32_t*)&data[40]);
                textureDataOffset = 64 + *((uint32_t*)&data[60]) + 4;
            }else{
                format = reverseBytesOfUint32(*((uint32_t*)&data[28]));
                width = reverseBytesOfUint32(*((uint32_t*)&data[36]));
                height = reverseBytesOfUint32(*((uint32_t*)&data[40]));
                textureDataOffset = 64 + reverseBytesOfUint32(*((uint32_t*)&data[60])) + 4;
            }

            if (format == GL_ETC1_RGB8_OES) {
                format = GL_COMPRESSED_RGB8_ETC2;
            }
        }else {
            width = 0;
            height = 0;
            textureDataOffset = 0;
            format = GL_INVALID_VALUE;
        }
    }

    bool ETCParser::is_valid() {
        switch (getFormat()) {
            case GL_COMPRESSED_R11_EAC:
            case GL_COMPRESSED_SIGNED_R11_EAC:
            case GL_COMPRESSED_RG11_EAC:
            case GL_COMPRESSED_SIGNED_RG11_EAC:
            case GL_COMPRESSED_RGB8_ETC2:
            case GL_COMPRESSED_SRGB8_ETC2:
            case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            case GL_COMPRESSED_RGBA8_ETC2_EAC:
            case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
                return true;
            default:
                return false;
        }
    }

    uint32_t ETCParser::getWidth()
    {
        return width;
    }

    uint32_t ETCParser::getHeight()
    {
        return height;
    }

    GLenum ETCParser::getFormat() {
        return format;
    }

    size_t ETCParser::getTextureDataOffset() {
        return textureDataOffset;
    }


NS_CC_END