#include <bitset>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

static const uint8_t kFrameHeaderFlagsCount = 8;
static const uint8_t kFrameIDLen = 4;
static const uint8_t kFrameFlagsLen = 8;
static const uint8_t kSynchsafeIntSize = 4;
static const uint8_t kSizeBlockLen = 7;
static const uint8_t kLangLen = 3;
static const uint8_t kEncodingLen = 1;
static const uint8_t kBomLen = 2;
static const uint8_t kTimeStampFormatLen = 1;
static const uint8_t kTimeStampLen = 4;
static const uint8_t kTypeOfEventLen = 1;
static const uint8_t kDateLen = 8;
static const uint8_t kNullLen = 1;

static const std::unordered_set<std::string> undefined_frames = {
        "MCDI", "MLLT", "SYTC", "SIGN", "AENC", "APIC", "ASPI", "GEOB", "RVRB", "TFLT", "TMED"
};

static const std::unordered_map<std::string, std::string> text_frames_info = {
        {"TALB", "Album/Movie/Show title"},
        {"TBPM", "BPM (beats per minute)"},
        {"TCOM", "Composer"},
        {"TCON", "Content type"},
        {"TCOP", "Copyright message"},
        {"TDEN", "Encoding time"},
        {"TDLY", "Playlist delay"},
        {"TDOR", "Original release time"},
        {"TDRC", "Recording time"},
        {"TDRL", "Release time"},
        {"TDTG", "Tagging time"},
        {"TENC", "Encoded by"},
        {"TEXT", "Lyricist/Text writer"},
        {"TFLT", "File type"},
        {"TIPL", "Involved people list"},
        {"TIT1", "Content group description"},
        {"TIT2", "Title/songname/content description"},
        {"TIT3", "Subtitle/Description refinement"},
        {"TKEY", "Initial key"},
        {"TLAN", "Language(s)"},
        {"TLEN", "Length"},
        {"TMCL", "Musician credits list"},
        {"TMED", "Media type"},
        {"TMOO", "Mood"},
        {"TOAL", "Original album/movie/show title"},
        {"TOFN", "Original filename"},
        {"TOLY", "Original lyricist(s)/text writer(s)"},
        {"TOPE", "Original artist(s)/performer(s)"},
        {"TOWN", "File owner/licensee"},
        {"TPE1", "Lead performer(s)/Soloist(s)"},
        {"TPE2", "Band/orchestra/accompaniment"},
        {"TPE3", "Conductor/performer refinement"},
        {"TPE4", "Interpreted, remixed, or otherwise modified by"},
        {"TPOS", "Part of a set"},
        {"TPRO", "Produced notice"},
        {"TPUB", "Publisher"},
        {"TRCK", "Track number/Position in set"},
        {"TRSN", "Internet radio station name"},
        {"TRSO", "Internet radio station owner"},
        {"TSOA", "Album sort order"},
        {"TSOP", "Performer sort order"},
        {"TSOT", "Title sort order"},
        {"TSRC", "ISRC (international standard recording code)"},
        {"TSSE", "Software/Hardware and settings used for encoding"},
        {"TSST", "Set subtitle"}
};

static const std::unordered_map<std::string, std::string> url_frames_info = {
        {"WCOM", "Commercial information"},
        {"WCOP", "Copyright/Legal information"},
        {"WOAF", "Official audio file webpage"},
        {"WOAR", "Official artist/performer webpage"},
        {"WOAS", "Official audio source webpage"},
        {"WORS", "Official Internet radio station homepage"},
        {"WPAY", "Payment"},
        {"WPUB", "Publishers official webpage"}
};

uint32_t ParseSynchsafeInt(std::ifstream& stream);

void ParseFramesFlags(std::ifstream& stream, std::vector<bool>& flags);

bool IsTextFrame(const char* id);

bool IsURLFrame(const char* id);

bool IsUndefinedFrame(const char* id);

bool CompareID(const char* lhs, const char* rhs);

enum Encoding {
    ISO_8859_1 = 0,
    UTF_16,
    UTF_16BE,
    UTF_8
};

static const std::unordered_map<uint8_t, std::string> time_stamp_format = {
        {1, "Absolute time, 32 bit sized, using MPEG [MPEG] frames as unit"},
        {2, "Absolute time, 32 bit sized, using milliseconds as unit"},
};

static const std::unordered_map<uint8_t, std::string> type_of_event{
        {0,   "padding (has no meaning)"},
        {1,   "end of initial silence"},
        {2,   "intro start"},
        {3,   "main part start"},
        {4,   "outro start"},
        {5,   "outro end"},
        {6,   "verse start"},
        {7,   "refrain start"},
        {8,   "interlude start"},
        {9,   "theme start"},
        {10,  "variation start"},
        {11,  "key change"},
        {12,  "time change"},
        {13,  "momentary unwanted noise (Snap, Crackle & Pop)"},
        {14,  "sustained noise"},
        {15,  "sustained noise end"},
        {16,  "intro end"},
        {17,  "main part end"},
        {18,  "verse end"},
        {19,  "refrain end"},
        {20,  "theme end"},
        {21,  "profanity"},
        {22,  "profanity end"},

        {23,  "reserved for future use"},

        {24,  "not predefined synch 0-F"},

        {253, "audio end (start of silence)"},
        {254, "audio file ends"},
        {255, "one more byte of events follows (all the following bytes with the value $FF have the same function)"}
};

static const std::unordered_map<uint8_t, std::string> received_as{
        {0, "Other"},
        {1, "Standard CD album with other songs"},
        {2, "Compressed audio on CD"},
        {3, "File over the Internet"},
        {4, "Stream over the Internet"},
        {5, "As note sheets"},
        {6, "As note sheets in a book with other sheets"},
        {7, "Music on other media"},
        {8, "Non-musical merchandise"}
};

static const std::unordered_map<uint8_t, std::string> content_type{
        {0, "is other"},
        {1, "is lyrics"},
        {2, "is text transcription"},
        {3, "is movement/part name (e.g. \"Adagio\")"},
        {4, "is events (e.g. \"Don Quijote enters the stage\")"},
        {5, "is chord (e.g. \"Bb F Fsus\")"},
        {6, "is trivia/'pop up' information"},
        {7, "is URLs to webpages"},
        {8, "is URLs to images"}
};

static const std::unordered_map<char, uint8_t> char_to_number{
        {'0', 0},
        {'1', 1},
        {'2', 2},
        {'3', 3},
        {'4', 4},
        {'5', 5},
        {'6', 6},
        {'7', 7},
        {'8', 8},
        {'9', 9}
};

struct Date {

    static const uint8_t kYearLen = 4;
    static const uint8_t kMonthLen = 2;
    static const uint8_t kDayLen = 2;

    uint16_t year_;
    uint8_t month_;
    uint8_t day_;

    Date() : year_(0), month_(0), day_(0) {
    }
};

Date ParseDate(char* data);

Encoding DetermineEncoding(uint8_t byte);

uint8_t DetermineTimeStampFormat(uint8_t byte);

uint8_t DetermineTypeOfEvent(uint8_t byte);

uint8_t DetermineReceivedAs(uint8_t byte);

uint8_t DetermineContentType(uint8_t byte);

void ParseNullString(std::string& text, std::ifstream& stream, uint32_t& size, Encoding encoding);

void ParseSizedString(char* text, std::ifstream&, uint32_t& size, uint32_t size_of_string);

uint16_t CharsToUint16(const char* data, size_t size);

uint32_t CharsToUint32(const char* data, size_t size);

uint64_t CharsToUint64(const char* data, size_t size);

uint64_t CharsToUint64(const std::string& data, size_t size);

class Frame {
protected:
    char id_[kFrameIDLen + kNullLen];

    uint32_t size_;

    std::vector<bool> flags_;

public:

    virtual ~Frame() = default;

    Frame() : size_(0), flags_(kFrameFlagsLen, false) {
        id_[4] = 0;
    }

    Frame(const char* id, uint32_t size, const std::vector<bool>& flags) : Frame() {
        for (uint8_t i = 0; i < kFrameIDLen; i++) {
            id_[i] = id[i];
        }
        size_ = size;
        for (uint8_t i = 0; i < kFrameHeaderFlagsCount; i++) {
            flags_[i] = flags[i];
        }
    }

    uint32_t size() const {
        return size_;
    }

    std::string_view id() const {
        return id_;
    }

    virtual void ParseFrame(std::ifstream& file) = 0;

    virtual void Print() const = 0;

    void PrintID() const {
        for (uint8_t i = 0; i < kFrameIDLen; i++) {
            std::cout << id_[i];
        }
    }
};

class EncodedFrame : public Frame {
protected:
    Encoding encoding_;
public:

    EncodedFrame(const char* id,
                 uint32_t size,
                 const std::vector<bool>& flags) : Frame(id, size, flags) {
        encoding_ = UTF_8;
    }
};

class TextFrame : public EncodedFrame {
protected:
    std::vector<std::string> text_;
public:

    TextFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : EncodedFrame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class WXXXAndTXXXFrame : public EncodedFrame {
protected:
    std::string description_;
    std::string text_;
public:
    WXXXAndTXXXFrame(const char* id,
                     uint32_t size,
                     const std::vector<bool>& flags) : EncodedFrame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class USLTAndCOMMFrame : public WXXXAndTXXXFrame {
protected:
    char language_[kLangLen];
public:
    USLTAndCOMMFrame(const char* id,
                     uint32_t size,
                     const std::vector<bool>& flags) : WXXXAndTXXXFrame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class UndefinedFrame : public Frame {
protected:
    std::string frame_content_;
public:
    UndefinedFrame(const char* id,
                   uint32_t size,
                   const std::vector<bool>& flags) : Frame(id, size, flags), frame_content_(size_, 0) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override {
        PrintID();
        std::cout << " | Undefined Frame";
        std::cout << '\n' << '\n';
    }
};

class URLFrame : public Frame {
protected:
    std::string url_;
public:
    URLFrame(const char* id,
             uint32_t size,
             const std::vector<bool>& flags) : Frame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class PCNTFrame : public Frame {
protected:
    std::string counter_;
public:
    PCNTFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : Frame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class POPMFrame : public PCNTFrame {
protected:
    std::string email_;
    uint8_t rating_;
public:
    POPMFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : PCNTFrame(id, size, flags), rating_(0) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class TimeStampedFrame : public Frame {
protected:
    uint8_t time_stamp_format_;
public:
    TimeStampedFrame(const char* id,
                     uint32_t size,
                     const std::vector<bool>& flags) : Frame(id, size, flags) {
        time_stamp_format_ = 1;
    }
};

class ETCOFrame : public TimeStampedFrame {
protected:
    std::vector<std::pair<uint8_t, uint32_t>> events_;
public:
    ETCOFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : TimeStampedFrame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class IdentificationFrame : public Frame {
protected:
    std::string identification_;
public:
    IdentificationFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : Frame(id, size, flags) {
    }
};

static const std::unordered_map<uint8_t, std::string> type_of_channel = {
        {0, "Other"},
        {1, "Master volume"},
        {2, "Front right"},
        {3, "Front left"},
        {4, "Back right"},
        {5, "Back left"},
        {6, "Front centre"},
        {7, "Back centre"},
        {8, "Subwoofer"}
};

uint8_t DetermineTypeOfChannel(uint8_t byte);

struct RVA2Channel {
    static const uint8_t kVolumeAdjLen = 2;

    uint8_t type_of_channel_;
    char volume_adj_[kVolumeAdjLen];
    uint8_t peak_bits_;
    std::string peak_volume_;

    RVA2Channel() : peak_bits_(0) {
        type_of_channel_ = 0;
    }
};

RVA2Channel ParseRVA2Channel(uint32_t& size, std::ifstream& stream);

class RVA2Frame : public IdentificationFrame {
protected:
    std::vector<RVA2Channel> channels_;
public:
    RVA2Frame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : IdentificationFrame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

static const std::unordered_map<uint8_t, std::string> interpolation = {
        {0, "Band"},
        {1, "Linear"}
};

uint8_t DetermineInterpolationMethod(uint8_t byte);

class EQU2Frame : public IdentificationFrame {
protected:
    static const uint8_t kInterpolationLen = 1;
    static const uint8_t kFreqLen = 2;
    static const uint8_t kVolumeAdjLen = 2;

    uint8_t interpolation_method_;
    std::string identification_;

    std::vector<std::pair<uint16_t, int16_t>> adj_points_;

public:
    EQU2Frame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : IdentificationFrame(id, size, flags) {
        interpolation_method_ = 0;
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class POSSFrame : public TimeStampedFrame {
protected:
    std::string position_;
public:
    POSSFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : TimeStampedFrame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class USERFrame : public EncodedFrame {
protected:
    char language_[kLangLen];
    std::string text_;
public:
    USERFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : EncodedFrame(id, size, flags) {
        encoding_ = UTF_8;
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class UFIDAndPRIVFrame : public Frame {
protected:
    std::string owner_identifier_;
    std::string data_;
public:
    UFIDAndPRIVFrame(const char* id,
                     uint32_t size,
                     const std::vector<bool>& flags) : Frame(id, size, flags) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class ENCRAndGRIDFrame : public UFIDAndPRIVFrame {
protected:
    uint8_t symbol_;
public:
    ENCRAndGRIDFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : UFIDAndPRIVFrame(id, size, flags), symbol_(0) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class OWNEFrame : public Frame {
protected:
    Encoding encoding_;
    std::string price_;
    Date date_;
    std::string seller_;
public:
    OWNEFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : Frame(id, size, flags) {
        encoding_ = UTF_8;
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class COMRFrame : public OWNEFrame {
protected:
    std::string contact_url_;
    uint8_t received_as_;
    std::string description_;
    std::string picture_mime_type_;
    std::string seller_logo_;
public:
    COMRFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : OWNEFrame(id, size, flags) {
        received_as_ = 0;
        encoding_ = UTF_8;
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class SEEKFrame : public Frame {
protected:
    static const uint8_t kOffsetLen = 4;

    uint32_t offset_to_next_tag_;
public:
    SEEKFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : Frame(id, size, flags), offset_to_next_tag_(0) {
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class RBUFFrame : public SEEKFrame {
protected:
    static const uint8_t kBufferSizeLen = 3;
    static const uint8_t kFlagLen = 1;

    uint32_t buffer_size_;
    bool flag_;
public:
    RBUFFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : SEEKFrame(id, size, flags), buffer_size_(0) {
        flag_ = false;
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class SYLTFrame : public USERFrame {
protected:
    uint8_t time_stamp_format_;
    uint8_t content_type_;
    std::vector<std::pair<std::string, uint32_t>> time_stamps_;

public:
    SYLTFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : USERFrame(id, size, flags) {
        content_type_ = 0;
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

class LINKFrame : public URLFrame {
protected:
    char frame_identifier_[kFrameIDLen + kNullLen];
    std::vector<std::string> id_and_additional_data_;

public:

    LINKFrame(const char* id,
              uint32_t size,
              const std::vector<bool>& flags) : URLFrame(id, size, flags) {
        frame_identifier_[4] = 0;
    }

    void ParseFrame(std::ifstream& stream) override;

    void Print() const override;
};

Frame* ParseFrameHeader(std::ifstream& stream);