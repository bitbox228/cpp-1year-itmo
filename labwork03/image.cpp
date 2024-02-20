#include "image.h"

struct Color {
    unsigned char r = 0, g = 0, b = 0;

    Color() {
        r = g = b = 0;
    }

    Color(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {
    }
};

const Color kColorBlack = Color(0,0,0);
const Color kColorYellow = Color(252, 209, 22);
const Color kColorPurple = Color(102, 0, 153);
const Color kColorGreen = Color(0, 128, 0);
const Color kColorWhite = Color(255, 255, 255);

const uint64_t kFileHeaderSize = 14;
const uint64_t kInformationHeaderSize = 40;
const uint64_t kTotalColors = 5;
const uint64_t kBitsPerPixel = 4;
const uint64_t kColorBytes = 4;
const uint64_t kColorPalleteSize = kColorBytes * kTotalColors;


void ToImage(const std::deque<std::deque<uint64_t>>& matrix, const std::string& path) {
    uint16_t width = matrix[0].size();
    uint16_t height = matrix.size();

    std::ofstream f;
    f.open(path, std::ios::out | std::ios::binary);

    const uint64_t padding = (8 - (width % 8)) % 8;
    const uint64_t full_width = width + padding;

    const uint64_t fileSize = kFileHeaderSize + kInformationHeaderSize + kColorPalleteSize + (full_width * height) / 2;
    uint8_t fileHeader[kFileHeaderSize] = {};
    uint8_t informationHeader[kInformationHeaderSize] = {};

    //file type
    fileHeader[0] = 'B';
    fileHeader[1] = 'M';
    //file size
    fileHeader[2] = fileSize;
    fileHeader[3] = fileSize >> 8;
    fileHeader[4] = fileSize >> 16;
    fileHeader[5] = fileSize >> 24;
    //reserved
    fileHeader[6] = 0;
    fileHeader[7] = 0;
    fileHeader[8] = 0;
    fileHeader[9] = 0;
    //number of bytes between start of file and the first byte of pixel data
    fileHeader[10] = kFileHeaderSize + kInformationHeaderSize + kColorPalleteSize;
    fileHeader[11] = 0;
    fileHeader[12] = 0;
    fileHeader[13] = 0;
    //information_header size
    informationHeader[0] = kInformationHeaderSize;
    informationHeader[1] = 0;
    informationHeader[2] = 0;
    informationHeader[3] = 0;
    //width of image
    informationHeader[4] = width;
    informationHeader[5] = width >> 8;
    informationHeader[6] = width >> 16;
    informationHeader[7] = width >> 24;
    //height of image
    informationHeader[8] = height;
    informationHeader[9] = height >> 8;
    informationHeader[10] = height >> 16;
    informationHeader[11] = height >> 24;
    //planes
    informationHeader[12] = 1;
    informationHeader[13] = 0;
    //bits per pixel
    informationHeader[14] = kBitsPerPixel;
    informationHeader[15] = 0;
    //total colors: black, white, green, purple, yellow
    informationHeader[32] = kTotalColors;
    informationHeader[33] = 0;
    informationHeader[34] = 0;
    informationHeader[35] = 0;

    uint8_t colorPallete[kColorPalleteSize] = {};
    //0 - white
    colorPallete[0] = kColorWhite.b;
    colorPallete[1] = kColorWhite.g;
    colorPallete[2] = kColorWhite.r;
    colorPallete[3] = 0;
    //1 - green
    colorPallete[4] = kColorGreen.b;
    colorPallete[5] = kColorGreen.g;
    colorPallete[6] = kColorGreen.r;
    colorPallete[7] = 0;
    //2 - purple
    colorPallete[8] = kColorPurple.b;
    colorPallete[9] = kColorPurple.g;
    colorPallete[10] = kColorPurple.r;
    colorPallete[11] = 0;
    //3 - yellow
    colorPallete[12] = kColorYellow.b;
    colorPallete[13] = kColorYellow.g;
    colorPallete[14] = kColorYellow.r;
    colorPallete[15] = 0;
    //4 - black
    colorPallete[16] = kColorBlack.b;
    colorPallete[17] = kColorBlack.g;
    colorPallete[18] = kColorBlack.r;
    colorPallete[19] = 0;

    f.write(reinterpret_cast<char*>(fileHeader), kFileHeaderSize);
    f.write(reinterpret_cast<char*>(informationHeader), kInformationHeaderSize);
    f.write(reinterpret_cast<char*>(colorPallete), kColorPalleteSize);

    uint64_t first_color, second_color;
    uint8_t total_color;
    for (int64_t x = height - 1; x >= 0; x--) {
        for (int64_t y = 0; y < full_width; y += 2) {
            if (y >= width) {
                first_color = 0;
                second_color = 0;
            } else if (y + 1 >= width) {
                first_color = matrix[x][y];
                second_color = 0;
            } else {
                first_color = matrix[x][y];
                second_color = matrix[x][y + 1];
            }

            if (first_color >= 4) {
                first_color = 4;
            }
            if (second_color >= 4) {
                second_color = 4;
            }

            total_color = (first_color << kBitsPerPixel);
            total_color += second_color;

            f << total_color;
        }
    }

    f.close();

}