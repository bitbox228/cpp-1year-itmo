#include "ID3v2.h"

const std::string FileHeader::id_ = "ID3v2";

bool FileHeader::Parse(std::ifstream& stream) {
    uint8_t current_byte;
    for (uint8_t i = 0; i < kHeaderSize - kSynchsafeIntSize; i++) {
        current_byte = stream.get();
        if (i <= 2 && current_byte != id_[i]) {
            return false;
        } else if (i == 3) {
            version_ = current_byte;
        } else if (i == 4) {
            subversion_ = current_byte;
        } else if (i == 5) {
            std::bitset<CHAR_BIT> byte(current_byte);
            for (uint8_t j = 0; j < kHeaderFlagsCount; j++) {
                flags[j] = byte[CHAR_BIT - j - 1];
            }
        }
    }
    size_ = ParseSynchsafeInt(stream);
    return true;
}

uint32_t Pow(uint32_t lhs, uint32_t rhs) {
    uint32_t result = 1;
    for (size_t i = 0; i < rhs; i++) {
        result *= lhs;
    }
    return result;
}

void ID3v2::ParseFile(const std::string& file_name) {
    std::ifstream stream;
    stream.open(file_name, std::ios::binary);

    header_.Parse(stream);

    uint32_t current_size = header_.size();

    if (header_.ext_header()) {
        uint32_t ext_header_size = ParseSynchsafeInt(stream);
        current_size -= ext_header_size;
        stream.seekg(ext_header_size - kSynchsafeIntSize, std::ios::cur);
    }

    Frame* current_frame = nullptr;

    while (current_size != 0) {
        current_frame = ParseFrameHeader(stream);
        if (current_frame == nullptr) {
            return;
        }
        current_size -= current_frame->size() + kSynchsafeIntSize;
        current_frame->ParseFrame(stream);
        frames_[static_cast<std::string>(current_frame->id())].push_back(current_frame);
    }
}

void ID3v2::PrintFrames() {
    for (const auto &[frame_key, frame_vector] : frames_) {
        for (size_t i = 0; i < frame_vector.size(); i++) {
            frame_vector[i]->Print();
        }
    }
}

void ID3v2::PrintFrames(const std::vector<std::string>& frames) {
    for (size_t i = 0; i < frames.size(); i++) {
        if (frames_.contains(frames[i])) {
            for (size_t j = 0; j < frames_.find(frames[i])->second.size(); j++) {
                frames_.find(frames[i])->second[j]->Print();
            }
        }
    }
}