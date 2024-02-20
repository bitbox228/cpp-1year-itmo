#include "Frame.h"

uint32_t ParseSynchsafeInt(std::ifstream& stream) {
    uint8_t current_byte;
    uint32_t result = 0;
    for (uint8_t i = 0; i < kSynchsafeIntSize; i++) {
        current_byte = stream.get();
        uint8_t block = kSynchsafeIntSize - i - 1;
        uint32_t byte = current_byte;
        result += (byte << block * kSizeBlockLen);
    }
    return result;
}

uint8_t DetermineInterpolationMethod(uint8_t byte) {
    if (byte > 1) {
        return 0;
    }
    return byte;
}

Encoding DetermineEncoding(uint8_t byte) {
    Encoding result = UTF_8;
    if (byte == 0) {
        result = ISO_8859_1;
    } else if (byte == 1) {
        result = UTF_16;
    } else if (byte == 2) {
        result = UTF_16BE;
    } else if (byte == 3) {
        result = UTF_8;
    }
    return result;
}

uint8_t DetermineTimeStampFormat(uint8_t byte) {
    if (byte != 1 && byte != 2) {
        return 1;
    }
    return byte;
}

uint8_t DetermineTypeOfEvent(uint8_t byte) {
    if ((23 <= byte) && (byte <= 223) || (240 <= byte) && (byte <= 252)) {
        return 23;
    } else if ((224 <= byte) && (byte <= 239)) {
        return 24;
    }
    return byte;
}

uint8_t DetermineReceivedAs(uint8_t byte) {
    if (byte > 8) {
        return 0;
    }
    return byte;
}

uint8_t DetermineContentType(uint8_t byte) {
    if (byte > 8) {
        return 0;
    }
    return byte;
}

void ParseFramesFlags(std::ifstream& stream, std::vector<bool>& flags) {
    uint8_t current_byte;
    current_byte = stream.get();
    uint16_t two_bytes = current_byte << CHAR_BIT;
    current_byte = stream.get();
    two_bytes += current_byte;

    std::bitset<2 * CHAR_BIT> bit_flags(two_bytes);

    flags[0] = bit_flags[14];
    flags[1] = bit_flags[13];
    flags[2] = bit_flags[12];

    flags[3] = bit_flags[6];
    flags[4] = bit_flags[3];
    flags[5] = bit_flags[2];
    flags[6] = bit_flags[1];
    flags[7] = bit_flags[0];
}

void TextFrame::ParseFrame(std::ifstream& stream) {
    uint8_t current_byte = stream.get();
    encoding_ = DetermineEncoding(current_byte);

    uint32_t size = size_ - kEncodingLen;
    std::string text;

    while (size != 0) {
        text.clear();
        ParseNullString(text, stream, size, encoding_);
        text_.push_back(text);
    }

    stream.seekg(size, std::ios::cur);
}

void TextFrame::Print() const {
    PrintID();
    std::cout << " | ";
    if (text_frames_info.contains(id_)) {
        std::cout << text_frames_info.find(id_)->second;
        std::cout << '\n' << ((text_.size() > 1) ? "Text strings: \n" : "Text string: ");
        for (size_t i = 0; i < text_.size(); i++) {
            std::cout << text_[i] << '\n';
        }
    } else {
        std::cout << "Undefined Text Frame \n";
    }
    std::cout << '\n';
}

Frame* ParseFrameHeader(std::ifstream& stream) {
    uint8_t current_byte;
    char id[kFrameIDLen + kNullLen];
    id[4] = 0;
    for (uint8_t i = 0; i < kFrameIDLen; i++) {
        current_byte = stream.get();
        id[i] = static_cast<char>(current_byte);
    }

    Frame* result = nullptr;

    uint32_t size = ParseSynchsafeInt(stream);

    std::vector<bool> flags(kFrameHeaderFlagsCount);

    ParseFramesFlags(stream, flags);

    if (IsUndefinedFrame(id)) {
        auto* new_frame = new UndefinedFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "TXXX") || CompareID(id, "WXXX")) {
        auto* new_frame = new WXXXAndTXXXFrame(id, size, flags);
        result = new_frame;
    } else if (IsTextFrame(id)) {
        auto* new_frame = new TextFrame(id, size, flags);
        result = new_frame;
    } else if (IsURLFrame(id)) {
        auto* new_frame = new URLFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "POPM")) {
        auto* new_frame = new POPMFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "COMM") || CompareID(id, "USLT")) {
        auto* new_frame = new USLTAndCOMMFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "ETCO")) {
        auto* new_frame = new ETCOFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "RVA2")) {
        auto* new_frame = new RVA2Frame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "EQU2")) {
        auto* new_frame = new EQU2Frame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "PCNT")) {
        auto* new_frame = new PCNTFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "RBUF")) {
        auto* new_frame = new RBUFFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "POSS")) {
        auto* new_frame = new POSSFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "USER")) {
        auto* new_frame = new USERFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "OWNE")) {
        auto* new_frame = new OWNEFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "ENCR") || CompareID(id, "GRID")) {
        auto* new_frame = new ENCRAndGRIDFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "PRIV") || CompareID(id, "UFID")) {
        auto* new_frame = new UFIDAndPRIVFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "COMR")) {
        auto* new_frame = new COMRFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "SEEK")) {
        auto* new_frame = new SEEKFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "SYLT")) {
        auto* new_frame = new SYLTFrame(id, size, flags);
        result = new_frame;
    } else if (CompareID(id, "LINK")) {
        auto* new_frame = new LINKFrame(id, size, flags);
        result = new_frame;
    }

    return result;
}

bool IsTextFrame(const char* id) {
    if (id[0] != 'T') {
        return false;
    }
    for (uint8_t i = 1; i < kFrameIDLen; i++) {
        if (!(isdigit(id[i]) || isupper(id[i]))) {
            return false;
        }
    }
    return true;
}

bool IsURLFrame(const char* id) {
    if (id[0] != 'W') {
        return false;
    }
    for (uint8_t i = 1; i < kFrameIDLen; i++) {
        if (!(isdigit(id[i]) || isupper(id[i]))) {
            return false;
        }
    }
    return true;
}

bool IsUndefinedFrame(const char* id) {
    return (undefined_frames.contains(id));
}

bool CompareID(const char* lhs, const char* rhs) {
    for (uint8_t i = 0; i < kFrameIDLen; i++) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

void ParseNullString(std::string& text, std::ifstream& stream, uint32_t& size, Encoding encoding = UTF_8) {
    if (size == 0) {
        return;
    }
    if (encoding == UTF_16 || encoding == UTF_16BE) {
        if (encoding == UTF_16) { // skip BOM
            if (size == 1) {
                return;
            }
            stream.seekg(kBomLen, std::ios::cur);
            size -= kBomLen;
        }
        if (size == 0) {
            return;
        }
        std::wstring sw;
        uint16_t lhs;
        uint16_t rhs;
        do {
            lhs = stream.get() & 0xFF;
            rhs = stream.get() & 0xFF;
            size -= 2;
            if (lhs == 0 && rhs == 0 || size == 0) {
                break;
            }
            sw.push_back((lhs << CHAR_BIT) | rhs);
        } while (lhs != 0 || rhs != 0 && size != 0);

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
        text = convert.to_bytes(sw);
    } else {
        uint8_t byte;
        if (size == 0) {
            return;
        }
        do {
            byte = stream.get();
            size--;
            if (byte == 0) {
                break;
            }
            text.push_back(static_cast<char>(byte));
        } while (byte != 0 && size != 0);
    }
}

void ParseSizedString(char* text, std::ifstream& stream, uint32_t& size, uint32_t size_of_string) {
    if (size_of_string > size) {
        return;
    }
    uint8_t current_byte;
    for (size_t i = 0; i < size_of_string; i++) {
        current_byte = stream.get();
        text[i] = static_cast<char>(current_byte);
    }
    size -= size_of_string;
}

uint16_t CharsToUint16(const char* data, size_t size) {
    uint16_t result = 0;
    for (size_t i = 0; i < std::min(sizeof(uint16_t), size); i++) {
        result <<= CHAR_BIT;
        result += static_cast<uint8_t>(data[i]);
    }
    return result;
}

uint32_t CharsToUint32(const char* data, size_t size) {
    uint32_t result = 0;
    for (size_t i = 0; i < std::min(sizeof(uint32_t), size); i++) {
        result <<= CHAR_BIT;
        result += static_cast<uint8_t>(data[i]);
    }
    return result;
}

uint64_t CharsToUint64(const char* data, size_t size) {
    uint64_t result = 0;
    for (size_t i = 0; i < std::min(sizeof(uint64_t), size); i++) {
        result <<= CHAR_BIT;
        result += static_cast<uint8_t>(data[i]);
    }
    return result;
}

uint64_t CharsToUint64(const std::string& data, size_t size) {
    uint64_t result = 0;
    for (size_t i = 0; i < std::min(sizeof(uint64_t), size); i++) {
        result <<= CHAR_BIT;
        result += static_cast<uint8_t>(data[i]);
    }
    return result;
}

uint8_t DetermineTypeOfChannel(uint8_t byte) {
    if (byte > 8) {
        return 0;
    }
    return byte;
}

RVA2Channel ParseRVA2Channel(uint32_t& size, std::ifstream& stream) {
    RVA2Channel result;
    uint8_t current_byte = stream.get();
    size--;
    result.type_of_channel_ = DetermineTypeOfChannel(current_byte);
    ParseSizedString(result.volume_adj_, stream, size, RVA2Channel::kVolumeAdjLen);
    result.peak_bits_ = stream.get();
    size--;
    for (uint8_t i = 0; i < result.peak_bits_ && size != 0; i++) {
        size--;
        current_byte = stream.get();
        result.peak_volume_.push_back(static_cast<char>(current_byte));
    }

    return result;
}

Date ParseDate(char* data) {
    auto* result = new Date;
    uint8_t size = 0;
    for (uint8_t i = size; i < size + Date::kYearLen; i++) {
        result->year_ *= 10;
        result->year_ += char_to_number.find(data[i])->second;
    }
    size += Date::kYearLen;

    for (uint8_t i = size; i < size + Date::kMonthLen; i++) {
        result->month_ *= 10;
        result->month_ += char_to_number.find(data[i])->second;
    }
    size += Date::kMonthLen;

    for (uint8_t i = size; i < size + Date::kDayLen; i++) {
        result->day_ *= 10;
        result->day_ += char_to_number.find(data[i])->second;
    }
    size += Date::kDayLen;

    return *result;
}

void UndefinedFrame::ParseFrame(std::ifstream& stream) {
    uint8_t current_byte;
    for (uint32_t i = 0; i < size_; i++) {
        current_byte = stream.get();
        frame_content_[i] = static_cast<char>(current_byte);
    }
}

void URLFrame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    ParseNullString(url_, stream, size);

    stream.seekg(size, std::ios::cur);
}

void URLFrame::Print() const {
    PrintID();
    std::cout << " | ";
    if (url_frames_info.contains(id_)) {
        std::cout << url_frames_info.find(id_)->second;
        std::cout << "\nURL: " << url_ << '\n';
    } else {
        std::cout << "Undefined Text Frame \n";
    }
    std::cout << '\n';
}

void WXXXAndTXXXFrame::ParseFrame(std::ifstream& stream) {
    uint8_t current_byte = stream.get();
    encoding_ = DetermineEncoding(current_byte);
    uint32_t size = size_ - kEncodingLen;

    ParseNullString(description_, stream, size, encoding_);
    std::string text;

    ParseNullString(text_, stream, size, encoding_);

    stream.seekg(size, std::ios::cur);
}

void WXXXAndTXXXFrame::Print() const {
    PrintID();
    if (CompareID(id_, "TXXX")) {
        std::cout << " | User defined text information frame \nDescription: " << description_;
        std::cout << " \nText string: " << text_;
    } else {
        std::cout << " | User defined URL link frame \nDescription: " << description_;
        std::cout << " \nURL: " << text_ << '\n';
    }

    std::cout << '\n';
}

void USLTAndCOMMFrame::ParseFrame(std::ifstream& stream) {
    uint8_t current_byte = stream.get();
    encoding_ = DetermineEncoding(current_byte);
    uint32_t size = size_ - kEncodingLen;

    ParseSizedString(language_, stream, size, kLangLen);

    ParseNullString(description_, stream, size, encoding_);

    ParseNullString(text_, stream, size, encoding_);

    stream.seekg(size, std::ios::cur);
}

void USLTAndCOMMFrame::Print() const {
    PrintID();
    if (CompareID(id_, "COMM")) {
        std::cout << " | Comments \nLanguage: ";
        for (uint8_t i = 0; i < kLangLen; i++) {
            std::cout << language_[i];
        }
        std::cout << " \nShort content description: " << description_ << " \nThe actual text: " << text_ << '\n';
    } else {
        std::cout << " | Unsynchronised lyrics/text transcription \nLanguage: ";
        for (uint8_t i = 0; i < kLangLen; i++) {
            std::cout << language_[i];
        }
        std::cout << " \nContent descriptor: " << description_ << " \nLyrics/text " << text_ << '\n';
    }
    std::cout << '\n';
}

void POPMFrame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    ParseNullString(email_, stream, size);

    rating_ = stream.get();
    size--;

    ParseNullString(counter_, stream, size);

    stream.seekg(size, std::ios::cur);
}

void POPMFrame::Print() const {
    PrintID();
    std::cout << " | Popularimeter \nEmail to user: " << email_ << " \nRating: " << static_cast<uint16_t>(rating_);
    if (counter_.size() <= sizeof(uint64_t)) {
        uint64_t counter_number = CharsToUint64(counter_, counter_.size());
        std::cout << " \nCounter: " << counter_number;
    }
    std::cout << '\n' << '\n';
}

void ETCOFrame::ParseFrame(std::ifstream& stream) {
    uint8_t current_byte = stream.get();
    time_stamp_format_ = DetermineTimeStampFormat(current_byte);

    char time_stamp[kTimeStampLen];
    uint32_t size = size_ - kTimeStampFormatLen;
    while (size != 0) {
        current_byte = stream.get();
        uint8_t event = DetermineTypeOfEvent(current_byte);
        size -= kTypeOfEventLen;
        ParseSizedString(time_stamp, stream, size, kTimeStampLen);
        uint32_t time_stamp_number = CharsToUint32(time_stamp, kTimeStampLen);
        events_.emplace_back(event, time_stamp_number);
    }

    stream.seekg(size, std::ios::cur);
}

void ETCOFrame::Print() const {
    PrintID();
    std::cout << " | Event timing codes \nTime stamp format: " << time_stamp_format.find(time_stamp_format_)->second
              << " \nEvents: \n";
    for (auto& event: events_) {
        std::cout << "Type of event: " << type_of_event.find(event.first)->second << " \nTime stamp: " << event.second
                  << '\n';
    }
    std::cout << '\n';
}

void RVA2Frame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    ParseNullString(identification_, stream, size);

    while (size != 0) {
        RVA2Channel channel = ParseRVA2Channel(size, stream);
        channels_.push_back(channel);
    }

    stream.seekg(size, std::ios::cur);
}

void RVA2Frame::Print() const {
    PrintID();
    std::cout << " | Relative volume adjustment (2) \n" << "Identification: " << identification_ << '\n';
    for (size_t i = 0; i < channels_.size(); i++) {
        std::cout << "Type of channel: " << type_of_channel.find(channels_[i].type_of_channel_)->second
                  << " \nVolume adjustment: ";
        for (uint8_t j = 0; j < RVA2Channel::kVolumeAdjLen; j++) {
            std::cout << channels_[i].volume_adj_[j];
        }
        if (channels_[i].peak_volume_.size() <= sizeof(uint64_t)) {
            uint64_t counter_number = CharsToUint64(channels_[i].peak_volume_, channels_[i].peak_volume_.size());
            std::cout << " \nPeak volume: " << counter_number;
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void EQU2Frame::ParseFrame(std::ifstream& stream) {
    uint8_t current_byte = stream.get();
    interpolation_method_ = DetermineInterpolationMethod(current_byte);

    uint32_t size = size_ - kInterpolationLen;
    ParseNullString(identification_, stream, size);

    while (size != 0) {
        char lhs[kFreqLen];
        char rhs[kVolumeAdjLen];
        ParseSizedString(lhs, stream, size, kFreqLen);
        ParseSizedString(rhs, stream, size, kVolumeAdjLen);
        adj_points_.emplace_back(CharsToUint16(lhs, kFreqLen), CharsToUint16(rhs, kVolumeAdjLen));
    }

    stream.seekg(size, std::ios::cur);
}

void EQU2Frame::Print() const {
    PrintID();
    std::cout << " | Equalisation (2) \nInterpolation method: " << interpolation.find(interpolation_method_)->second;
    std::cout << " \nIdentification: " << identification_ << '\n';
    std::cout << '\n';
}

void PCNTFrame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    ParseNullString(counter_, stream, size);

    stream.seekg(size, std::ios::cur);
}

void PCNTFrame::Print() const {
    PrintID();
    std::cout << " | Play counter";
    if (counter_.size() <= sizeof(uint64_t)) {
        uint64_t counter_number = CharsToUint64(counter_, counter_.size());
        std::cout << " \nCounter: " << counter_number;
    }
    std::cout << '\n';
    std::cout << '\n';
}

void RBUFFrame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    char buffer_size[kBufferSizeLen];
    ParseSizedString(buffer_size, stream, size, kBufferSizeLen);
    buffer_size_ = CharsToUint32(buffer_size, kBufferSizeLen);

    std::bitset<CHAR_BIT> flag(stream.get());
    flag_ = flag[0];
    size--;

    char offset[kOffsetLen];
    ParseSizedString(offset, stream, size, kOffsetLen);
    offset_to_next_tag_ = CharsToUint32(offset, kOffsetLen);

    stream.seekg(size, std::ios::cur);
}

void RBUFFrame::Print() const {
    PrintID();
    std::cout << " | Recommended buffer size \nBuffer size: " << buffer_size_;
    std::cout << " \nEmbedded info flag: " << flag_ << " \nOffset to next tag: " << offset_to_next_tag_ << '\n';
    std::cout << '\n';
}

void POSSFrame::ParseFrame(std::ifstream& stream) {
    time_stamp_format_ = DetermineTimeStampFormat(stream.get());

    uint32_t size = size_ - kTimeStampFormatLen;
    ParseNullString(position_, stream, size);

    stream.seekg(size, std::ios::cur);
}

void POSSFrame::Print() const {
    PrintID();
    std::cout << " | Position synchronisation frame \nTime stamp: "
              << time_stamp_format.find(time_stamp_format_)->second;
    std::cout << " \nPosition: " << position_ << '\n';
    std::cout << '\n';
}

void USERFrame::ParseFrame(std::ifstream& stream) {
    encoding_ = DetermineEncoding(stream.get());

    uint32_t size = size_ - kEncodingLen;
    ParseSizedString(language_, stream, size, kLangLen);

    ParseNullString(text_, stream, size, encoding_);

    stream.seekg(size, std::ios::cur);
}

void USERFrame::Print() const {
    PrintID();
    std::cout << " | Terms of use frame \nLanguage: ";
    for (uint8_t i = 0; i < kLangLen; i++) {
        std::cout << language_[i];
    }
    std::cout << " \nThe actual text: " << text_ << '\n';
    std::cout << '\n';
}

void OWNEFrame::ParseFrame(std::ifstream& stream) {
    encoding_ = DetermineEncoding(stream.get());

    uint32_t size = size_ - kEncodingLen;
    ParseNullString(price_, stream, size);

    char date[kDateLen];
    ParseSizedString(date, stream, size, kDateLen);

    ParseNullString(seller_, stream, size, encoding_);

    stream.seekg(size, std::ios::cur);
}

void OWNEFrame::Print() const {
    PrintID();
    std::cout << " | Ownership frame ";
    if (price_.size() <= sizeof(uint64_t)) {
        uint64_t price_paid_number = CharsToUint64(price_, price_.size());
        std::cout << " \nPrice paid: " << price_paid_number;
    }
    std::cout << " \nDate of purch" << date_.year_ << '.' << date_.month_ << '.'
              << date_.day_;
    std::cout << " \nSeller: " << seller_ << '\n';
    std::cout << '\n';
}

void ENCRAndGRIDFrame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    ParseNullString(owner_identifier_, stream, size);

    symbol_ = stream.get();
    size--;

    ParseNullString(data_, stream, size);

    stream.seekg(size, std::ios::cur);
}

void ENCRAndGRIDFrame::Print() const {
    PrintID();
    if (CompareID(id_, "ENCR")) {
        std::cout << " | Encryption method registration" << " \nOwner identifier: " << owner_identifier_;
        std::cout << " \nMethod symbol: " << symbol_ << " \nEncryption data: " << data_;

    } else {

        std::cout << " | Group identification registration" << " \nOwner identifier: " << owner_identifier_;
        std::cout << " \nGroup symbol: " << symbol_ << " \nEncryption data: " << data_;
    }
    std::cout << '\n';
}

void UFIDAndPRIVFrame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    ParseNullString(owner_identifier_, stream, size);
    ParseNullString(data_, stream, size);

    stream.seekg(size, std::ios::cur);
}

void UFIDAndPRIVFrame::Print() const {
    if (CompareID(id_, "PRIV")) {
        std::cout << id_ << " | Private frame \nOwner identifier: " << owner_identifier_;
        std::cout << " \nThe private data: " << data_;
    } else {
        std::cout << " | Unique file identifier \nOwner identifier: " << owner_identifier_;
        std::cout << " \nIdentifier: " << data_ << '\n';
    }
    std::cout << '\n';
}

void COMRFrame::ParseFrame(std::ifstream& stream) {
    encoding_ = DetermineEncoding(stream.get());
    uint32_t size = size_ - kEncodingLen;
    ParseNullString(price_, stream, size);
    char date[kDateLen];
    ParseSizedString(date, stream, size, kDateLen);
    date_ = ParseDate(date);
    ParseNullString(contact_url_, stream, size);
    received_as_ = DetermineReceivedAs(stream.get());
    size--;
    ParseNullString(seller_, stream, size, encoding_);
    ParseNullString(description_, stream, size, encoding_);
    ParseNullString(picture_mime_type_, stream, size);
    ParseNullString(seller_logo_, stream, size);

    stream.seekg(size, std::ios::cur);
}

void COMRFrame::Print() const {
    PrintID();
    std::cout << " | Commercial frame \nPrice string: " << price_ << " \nValid until: ";
    std::cout << date_.year_ << '.' << date_.month_ << '.' << date_.day_;
    std::cout << " \nContact URL: " << contact_url_ << " \nReceived as: " << received_as.find(received_as_)->second;
    std::cout << " \nName of seller: " << seller_ << " \nDescription: " << description_;
    std::cout << " \nPicture MIME type: " << picture_mime_type_ << '\n';
    std::cout << '\n';
}

void SEEKFrame::ParseFrame(std::ifstream& stream) {
    char offset[kOffsetLen];
    uint32_t size = size_;
    ParseSizedString(offset, stream, size, kOffsetLen);
    offset_to_next_tag_ = CharsToUint32(offset, kOffsetLen);

    stream.seekg(size, std::ios::cur);
}

void SEEKFrame::Print() const {
    PrintID();
    std::cout << " | Seek frame \nMinimum offset to next tag: " << offset_to_next_tag_ << '\n';
    std::cout << '\n';
}

void SYLTFrame::ParseFrame(std::ifstream& stream) {
    uint32_t size = size_;
    encoding_ = DetermineEncoding(stream.get());
    size--;

    ParseSizedString(language_, stream, size, kLangLen);

    time_stamp_format_ = DetermineTimeStampFormat(stream.get());
    size--;

    content_type_ = DetermineContentType(stream.get());
    size--;

    ParseNullString(text_, stream, size, encoding_);

    uint8_t break_point = stream.get();
    size--;

    while (size != 0) {
        std::string text;
        char char_number[kTimeStampLen];
        ParseNullString(text, stream, size, encoding_);
        ParseSizedString(char_number, stream, size, kTimeStampLen);
        uint8_t cur_break_point = 0;
        if (size > 0) {
            cur_break_point = stream.get();
            size--;
        }
        uint32_t number = CharsToUint32(char_number, kTimeStampLen);
        time_stamps_.emplace_back(text, number);
        if (cur_break_point != break_point) {
            break;
        }
    }

    stream.seekg(size, std::ios::cur);
}

void SYLTFrame::Print() const {
    PrintID();
    std::cout << " | Synchronised lyrics/text \nLanguage: ";
    for (uint8_t i = 0; i < kLangLen; i++) {
        std::cout << language_[i];
    }
    std::cout << " \nTime stamp format: " << time_stamp_format.find(time_stamp_format_)->second
              << " \nContent type: " << content_type.find(content_type_)->second;
    std::cout << " \nContent descriptor: " << text_;
    std::cout << " \nTime stamps: \n";
    for (size_t i = 0; i < time_stamps_.size(); i++) {
        std::cout << time_stamps_[i].second << ' ' << time_stamps_[i].first << '\n';
    }
    std::cout << '\n';
}

void LINKFrame::ParseFrame(std::ifstream& stream) {
    uint8_t current_byte;
    for (uint8_t i = 0; i < kFrameIDLen; i++) {
        current_byte = stream.get();
        frame_identifier_[i] = static_cast<char>(current_byte);
    }
    uint32_t size = size_ - kFrameIDLen;
    ParseNullString(url_, stream, size);
    while (size != 0) {
        std::string data;
        ParseNullString(data, stream, size);
        id_and_additional_data_.push_back(data);
    }
}

void LINKFrame::Print() const {
    PrintID();
    std::cout << " | Linked information \nFrame identifier: " << frame_identifier_;
    std::cout << " \nURL: " << url_ << '\n';
    if (!id_and_additional_data_.empty()) {
        std::cout << "ID and additional data: \n";
        for (size_t i = 0; i < id_and_additional_data_.size(); i++) {
            std::cout << id_and_additional_data_[i] << '\n';
        }
    }
    std::cout << '\n';
}
