/* Copyright (c) 2014-2017, ARM Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ETCHeader.h"
#include <string.h>

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

static const char kMagicPkm10[] = { 'P', 'K', 'M', ' ', '1', '0' };
static const char kMagicPkm20[] = { 'P', 'K', 'M', ' ', '2', '0' };

NS_CC_BEGIN


    ETCHeader::ETCHeader()
    {}

    ETCHeader::ETCHeader(const unsigned char *data)
    {
        /*
         * Load from a ETC compressed pkm image file.
         * First 6 bytes are the name of the file format and version/packing type.
         * Bytes 7 and 8 are blank.
         */
        /* Beware endianess issues with most/least significant bits of the height and width. */
        formatIdentifierMSB = data[6];
        formatIdentifierLSB = data[7];
        paddedWidthMSB  = data[8];
        paddedWidthLSB  = data[9];
        paddedHeightMSB = data[10];
        paddedHeightLSB = data[11];
        widthMSB        = data[12];
        widthLSB        = data[13];
        heightMSB       = data[14];
        heightLSB       = data[15];
    }

    bool ETCHeader::is_valid_pkm(const unsigned char *data) {
        bool is_pkm10 = memcmp(data, kMagicPkm10, sizeof(kMagicPkm10));
        bool is_pkm20 = memcmp(data, kMagicPkm20, sizeof(kMagicPkm20));
        // TODO more check
        return is_pkm10 || is_pkm20;
    }

    unsigned short ETCHeader::getWidth(void)
    {
        return (widthMSB << 8) | widthLSB;
    }

    unsigned short ETCHeader::getHeight(void)
    {
        return (heightMSB << 8) | heightLSB;
    }

    unsigned short ETCHeader::getPaddedWidth(void)
    {
        return (paddedWidthMSB << 8) | paddedWidthLSB;
    }

    unsigned short ETCHeader::getPaddedHeight(void)
    {
        return (paddedHeightMSB << 8) | paddedHeightLSB;
    }

    GLsizei ETCHeader::getSize(GLenum internalFormat)
    {
        if (internalFormat != GL_COMPRESSED_RG11_EAC       && internalFormat != GL_COMPRESSED_SIGNED_RG11_EAC &&
            internalFormat != GL_COMPRESSED_RGBA8_ETC2_EAC && internalFormat != GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)
        {
            return (getPaddedWidth() * getPaddedHeight()) >> 1;
        }
        else
        {
            return (getPaddedWidth() * getPaddedHeight());
        }
    }

    GLenum ETCHeader::getFormat() {
        unsigned short format_identifier = (formatIdentifierMSB << 8) | formatIdentifierLSB;
        if (format_identifier > sizeof(IdentifierToFormat)/sizeof(IdentifierToFormat[0])-1){
            return GL_INVALID_VALUE;
        }else {
            return IdentifierToFormat[format_identifier];
        }
    }


NS_CC_END