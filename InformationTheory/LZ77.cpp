// #include <array>
// #include <cstdint>
// #include <cstring>
// #include <iomanip>
// #include <iostream>
// #include <map>
// #include <stdexcept>
// #include <string>
// #include <sstream>
// #include <tuple>
// #include <unordered_map>
// #include <utility>
// #include <vector>

// #include <zlib.h>

// using namespace std;

// namespace {

// struct LZ77Token {
//     int offset;
//     int length;
//     char next;
// };

// int reverseBits(int value, int bitCount) {
//     int result = 0;
//     for (int i = 0; i < bitCount; ++i) {
//         result = (result << 1) | ((value >> i) & 1);
//     }
//     return result;
// }

// class BitReader {
// public:
//     explicit BitReader(const vector<unsigned char>& data) : data_(data), bytePos_(0), bitPos_(0) {}

//     int readBits(int count) {
//         int value = 0;
//         for (int i = 0; i < count; ++i) {
//             if (bytePos_ >= data_.size()) {
//                 throw runtime_error("unexpected end of compressed stream");
//             }
//             int bit = (data_[bytePos_] >> bitPos_) & 1;
//             value |= (bit << i);
//             ++bitPos_;
//             if (bitPos_ == 8) {
//                 bitPos_ = 0;
//                 ++bytePos_;
//             }
//         }
//         return value;
//     }

// private:
//     const vector<unsigned char>& data_;
//     size_t bytePos_;
//     int bitPos_;
// };

// struct HuffmanEntry {
//     int symbol;
//     int bitLength;
// };

// class HuffmanDecoder {
// public:
//     void addCode(int symbol, int code, int bitLength) {
//         entries_.push_back({symbol, bitLength, code});
//         if (bitLength > maxBitLength_) {
//             maxBitLength_ = bitLength;
//         }
//     }

//     int decode(BitReader& reader) const {
//         int code = 0;
//         for (int bitLength = 1; bitLength <= maxBitLength_; ++bitLength) {
//             code |= (reader.readBits(1) << (bitLength - 1));
//             for (const auto& entry : entries_) {
//                 if (entry.bitLength == bitLength && entry.code == code) {
//                     return entry.symbol;
//                 }
//             }
//         }
//         throw runtime_error("invalid Huffman code in fixed block");
//     }

// private:
//     struct Entry {
//         int symbol;
//         int bitLength;
//         int code;
//     };

//     vector<Entry> entries_;
//     int maxBitLength_ = 0;
// };

// HuffmanDecoder buildFixedLiteralLengthDecoder() {
//     HuffmanDecoder decoder;
//     int code = 0;

//     for (int symbol = 256; symbol <= 279; ++symbol) {
//         decoder.addCode(symbol, reverseBits(code, 7), 7);
//         ++code;
//     }

//     code <<= 1;
//     for (int symbol = 0; symbol <= 143; ++symbol) {
//         decoder.addCode(symbol, reverseBits(code, 8), 8);
//         ++code;
//     }

//     for (int symbol = 280; symbol <= 287; ++symbol) {
//         decoder.addCode(symbol, reverseBits(code, 8), 8);
//         ++code;
//     }

//     code <<= 1;
//     for (int symbol = 144; symbol <= 255; ++symbol) {
//         decoder.addCode(symbol, reverseBits(code, 9), 9);
//         ++code;
//     }

//     return decoder;
// }

// HuffmanDecoder buildFixedDistanceDecoder() {
//     HuffmanDecoder decoder;
//     for (int symbol = 0; symbol < 32; ++symbol) {
//         decoder.addCode(symbol, reverseBits(symbol, 5), 5);
//     }
//     return decoder;
// }

// const array<int, 29> LENGTH_BASE = {
//     3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
//     15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
//     67, 83, 99, 115, 131, 163, 195, 227, 258,
// };

// const array<int, 29> LENGTH_EXTRA = {
//     0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
//     1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
//     4, 4, 4, 4, 5, 5, 5, 5, 0,
// };

// const array<int, 30> DIST_BASE = {
//     1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
//     33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
//     1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577,
// };

// const array<int, 30> DIST_EXTRA = {
//     0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
//     4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
//     9, 9, 10, 10, 11, 11, 12, 12, 13, 13,
// };

// int decodeLength(int symbol, BitReader& reader) {
//     if (symbol < 257 || symbol > 285) {
//         throw runtime_error("invalid length symbol");
//     }
//     if (symbol == 285) {
//         return 258;
//     }
//     int index = symbol - 257;
//     return LENGTH_BASE[index] + reader.readBits(LENGTH_EXTRA[index]);
// }

// int decodeDistance(int symbol, BitReader& reader) {
//     if (symbol < 0 || symbol > 29) {
//         throw runtime_error("invalid distance symbol");
//     }
//     return DIST_BASE[symbol] + reader.readBits(DIST_EXTRA[symbol]);
// }

// vector<unsigned char> compressFixedZlib(const string& input) {
//     z_stream stream{};
//     if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS, 8, Z_FIXED) != Z_OK) {
//         throw runtime_error("deflateInit2 failed");
//     }

//     vector<unsigned char> output;
//     output.resize(deflateBound(&stream, static_cast<uLong>(input.size())));

//     stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
//     stream.avail_in = static_cast<uInt>(input.size());
//     stream.next_out = output.data();
//     stream.avail_out = static_cast<uInt>(output.size());

//     int ret = deflate(&stream, Z_FINISH);
//     if (ret != Z_STREAM_END) {
//         deflateEnd(&stream);
//         throw runtime_error("deflate failed to finish correctly");
//     }

//     output.resize(stream.total_out);
//     deflateEnd(&stream);
//     return output;
// }

// vector<LZ77Token> parseFixedDeflateToTokens(const vector<unsigned char>& zlibData) {
//     if (zlibData.size() < 6) {
//         throw runtime_error("zlib payload too short");
//     }

//     unsigned int cmf = zlibData[0];
//     unsigned int flg = zlibData[1];
//     if ((cmf & 0x0F) != 8) {
//         throw runtime_error("not a deflate-based zlib stream");
//     }
//     if (((cmf << 8) + flg) % 31 != 0) {
//         throw runtime_error("invalid zlib header check bits");
//     }
//     if (flg & 0x20) {
//         throw runtime_error("preset dictionary is not supported");
//     }

//     vector<unsigned char> deflateData(zlibData.begin() + 2, zlibData.end() - 4);
//     BitReader reader(deflateData);
//     HuffmanDecoder literalLengthDecoder = buildFixedLiteralLengthDecoder();
//     HuffmanDecoder distanceDecoder = buildFixedDistanceDecoder();

//     vector<LZ77Token> tokens;
//     bool finalBlock = false;
//     while (!finalBlock) {
//         finalBlock = reader.readBits(1) != 0;
//         int blockType = reader.readBits(2);
//         if (blockType != 1) {
//             throw runtime_error("only fixed-Huffman deflate blocks are supported");
//         }

//         while (true) {
//             int symbol = literalLengthDecoder.decode(reader);
//             if (symbol < 256) {
//                 tokens.push_back({0, 0, static_cast<char>(symbol)});
//             } else if (symbol == 256) {
//                 break;
//             } else {
//                 int length = decodeLength(symbol, reader);
//                 int distanceSymbol = distanceDecoder.decode(reader);
//                 int distance = decodeDistance(distanceSymbol, reader);
//                 tokens.push_back({distance, length, '\0'});
//             }
//         }
//     }

//     return tokens;
// }

// string bytesToHex(const vector<unsigned char>& data) {
//     ostringstream oss;
//     oss << hex << setfill('0');
//     for (unsigned char byte : data) {
//         oss << setw(2) << static_cast<int>(byte);
//     }
//     return oss.str();
// }

// }  // namespace

// /* =======================
//    LZ77 解码
//    ======================= */
// string decode77(const vector<LZ77Token>& tokens) {
//     string output;
//     for (const auto& token : tokens) {
//         if (token.offset == 0 && token.length == 0) {
//             output.push_back(token.next);
//             continue;
//         }

//         if (token.offset <= 0 || token.offset > static_cast<int>(output.size())) {
//             throw runtime_error("invalid LZ77 backward distance");
//         }

//         int start = static_cast<int>(output.size()) - token.offset;
//         for (int i = 0; i < token.length; ++i) {
//             output.push_back(output[start + i]);
//         }
//         if (token.next != '\0') {
//             output.push_back(token.next);
//         }
//     }
//     return output;
// }

// /* =======================
//    LZ78 编码 / 解码
//    ======================= */
// vector<pair<int, char>> encode78(const string& input) {
//     map<string, int> dictionary;
//     vector<pair<int, char>> encoded;
//     string current;
//     int nextIndex = 1;

//     for (char ch : input) {
//         string candidate = current + ch;
//         if (dictionary.count(candidate)) {
//             current = candidate;
//         } else {
//             int prefixIndex = current.empty() ? 0 : dictionary[current];
//             encoded.push_back({prefixIndex, ch});
//             dictionary[candidate] = nextIndex++;
//             current.clear();
//         }
//     }

//     if (!current.empty()) {
//         encoded.push_back({dictionary[current], '\0'});
//     }

//     return encoded;
// }

// string decode78(const vector<pair<int, char>>& encoded) {
//     vector<string> dictionary(1, "");
//     string decoded;

//     for (const auto& item : encoded) {
//         int index = item.first;
//         char ch = item.second;
//         if (index < 0 || index >= static_cast<int>(dictionary.size())) {
//             throw runtime_error("invalid LZ78 dictionary index");
//         }

//         string word = dictionary[index];
//         if (ch != '\0') {
//             word.push_back(ch);
//         }
//         decoded += word;
//         dictionary.push_back(word);
//     }

//     return decoded;
// }

// /* =======================
//    LZW 编码 / 解码
//    ======================= */
// vector<int> encodeLZW(const string& input) {
//     unordered_map<string, int> dictionary;
//     for (int i = 0; i < 256; ++i) {
//         dictionary[string(1, static_cast<char>(i))] = i;
//     }

//     vector<int> encoded;
//     string current;
//     int nextCode = 256;

//     for (char ch : input) {
//         string candidate = current + ch;
//         if (dictionary.count(candidate)) {
//             current = candidate;
//         } else {
//             if (!current.empty()) {
//                 encoded.push_back(dictionary[current]);
//             }
//             dictionary[candidate] = nextCode++;
//             current = string(1, ch);
//         }
//     }

//     if (!current.empty()) {
//         encoded.push_back(dictionary[current]);
//     }

//     return encoded;
// }

// string decodeLZW(const vector<int>& encoded) {
//     if (encoded.empty()) {
//         return "";
//     }

//     vector<string> dictionary(256);
//     for (int i = 0; i < 256; ++i) {
//         dictionary[i] = string(1, static_cast<char>(i));
//     }

//     int nextCode = 256;
//     string previous = dictionary.at(encoded[0]);
//     string decoded = previous;

//     for (size_t i = 1; i < encoded.size(); ++i) {
//         int code = encoded[i];
//         string current;

//         if (code < static_cast<int>(dictionary.size())) {
//             current = dictionary[code];
//         } else if (code == nextCode) {
//             current = previous + previous[0];
//         } else {
//             throw runtime_error("invalid LZW code stream");
//         }

//         decoded += current;
//         dictionary.push_back(previous + current[0]);
//         ++nextCode;
//         previous = current;
//     }

//     return decoded;
// }

// void testLZ77WithZlib(const string& text, const string& label) {
//     vector<unsigned char> compressed = compressFixedZlib(text);
//     vector<LZ77Token> tokens = parseFixedDeflateToTokens(compressed);
//     string decoded = decode77(tokens);

//     double ratio = text.empty() ? 0.0 : static_cast<double>(compressed.size()) / static_cast<double>(text.size());

//     cout << "==== " << label << " ====" << '\n';
//     cout << "原始长度: " << text.size() << " 字节" << '\n';
//     cout << "压缩长度(zlib fixed): " << compressed.size() << " 字节" << '\n';
//     cout << "压缩率: " << fixed << setprecision(4) << ratio * 100.0 << "%" << '\n';
//     cout << "LZ77 token 数: " << tokens.size() << '\n';
//     cout << "译码是否正确: " << boolalpha << (decoded == text) << '\n';
//     cout << "压缩结果(hex): " << bytesToHex(compressed) << '\n';
//     cout << '\n';
// }

// void testLZ78AndLZW(const string& text, const string& label) {
//     auto lz78Encoded = encode78(text);
//     auto lz78Decoded = decode78(lz78Encoded);

//     auto lzwEncoded = encodeLZW(text);
//     auto lzwDecoded = decodeLZW(lzwEncoded);

//     cout << "---- " << label << " 的 LZ78 / LZW 测试 ----" << '\n';
//     cout << "LZ78 编码项数: " << lz78Encoded.size() << '\n';
//     cout << "LZ78 解码是否正确: " << boolalpha << (lz78Decoded == text) << '\n';
//     cout << "LZW 编码项数: " << lzwEncoded.size() << '\n';
//     cout << "LZW 解码是否正确: " << boolalpha << (lzwDecoded == text) << '\n';
//     cout << '\n';
// }

// int main() {
//     try {
//         const string eShort = "2.71828182845904523536";
//         const string eLong =
//             "2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427427466391"
//             "932003059921817413596629043572900334295260";
//         const string repetitive = "abracadabra abracadabra abracadabra";

//         testLZ77WithZlib(eShort, "自然对数短串");
//         testLZ77WithZlib(eLong, "自然对数长串");
//         testLZ77WithZlib(repetitive, "重复文本样例");

//         testLZ78AndLZW(eLong, "自然对数长串");

//         return 0;
//     } catch (const exception& ex) {
//         cerr << "程序运行失败: " << ex.what() << '\n';
//         return 1;
//     }
// }
