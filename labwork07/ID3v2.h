#include "Frame.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

static const uint8_t kHeaderSize = 10;
static const uint8_t kHeaderFlagsCount = 4;

uint32_t Pow(uint32_t lhs, uint32_t rhs);

class FileHeader {

    static const std::string id_;

    uint8_t version_;

    uint8_t subversion_;

    std::vector<bool> flags;

    uint32_t size_;

public:

    FileHeader() : flags(kHeaderFlagsCount), size_(0), version_(0), subversion_(0) {
    }

    static const std::string& id() {
        return id_;
    }

    uint16_t version() const {
        return version_;
    }

    uint16_t revision() const {
        return subversion_;
    }

    uint32_t size() const {
        return size_;
    }

    bool Parse(std::ifstream& stream);

    bool ext_header() const {
        return flags[1];
    }
};

class ID3v2 {
public:
    FileHeader header_;

    std::map<std::string, std::vector<Frame*>> frames_;

    ID3v2() : header_() {}

    ~ID3v2() {
        for (auto& [frame_key, frame_vector] : frames_) {
            for (auto & current_frame : frame_vector) {
                delete current_frame;
            }
        }
    }

    void ParseFile(const std::string& file_name);

    void PrintFrames();

    void PrintFrames(const std::vector<std::string>& frames);
};