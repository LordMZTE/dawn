// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/reader/wgsl/lexer.h"

#include <cctype>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "src/tint/debug.h"
#include "src/tint/number.h"
#include "src/tint/text/unicode.h"

namespace tint::reader::wgsl {
namespace {

// Unicode parsing code assumes that the size of a single std::string element is
// 1 byte.
static_assert(sizeof(decltype(tint::Source::FileContent::data[0])) == sizeof(uint8_t),
              "tint::reader::wgsl requires the size of a std::string element "
              "to be a single byte");

bool read_blankspace(std::string_view str, size_t i, bool* is_blankspace, size_t* blankspace_size) {
    // See https://www.w3.org/TR/WGSL/#blankspace

    auto* utf8 = reinterpret_cast<const uint8_t*>(&str[i]);
    auto [cp, n] = text::utf8::Decode(utf8, str.size() - i);

    if (n == 0) {
        return false;
    }

    static const auto kSpace = text::CodePoint(0x0020);  // space
    static const auto kHTab = text::CodePoint(0x0009);   // horizontal tab
    static const auto kL2R = text::CodePoint(0x200E);    // left-to-right mark
    static const auto kR2L = text::CodePoint(0x200F);    // right-to-left mark

    if (cp == kSpace || cp == kHTab || cp == kL2R || cp == kR2L) {
        *is_blankspace = true;
        *blankspace_size = n;
        return true;
    }

    *is_blankspace = false;
    return true;
}

uint32_t dec_value(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint32_t>(c - '0');
    }
    return 0;
}

uint32_t hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint32_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return 0xA + static_cast<uint32_t>(c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 0xA + static_cast<uint32_t>(c - 'A');
    }
    return 0;
}

}  // namespace

Lexer::Lexer(const Source::File* file) : file_(file), location_{1, 1} {}

Lexer::~Lexer() = default;

const std::string_view Lexer::line() const {
    if (file_->content.lines.size() == 0) {
        static const char* empty_string = "";
        return empty_string;
    }
    return file_->content.lines[location_.line - 1];
}

size_t Lexer::pos() const {
    return location_.column - 1;
}

size_t Lexer::length() const {
    return line().size();
}

const char& Lexer::at(size_t pos) const {
    auto l = line();
    // Unlike for std::string, if pos == l.size(), indexing `l[pos]` is UB for
    // std::string_view.
    if (pos >= l.size()) {
        static const char zero = 0;
        return zero;
    }
    return l[pos];
}

std::string_view Lexer::substr(size_t offset, size_t count) {
    return line().substr(offset, count);
}

void Lexer::advance(size_t offset) {
    location_.column += offset;
}

void Lexer::set_pos(size_t pos) {
    location_.column = pos + 1;
}

void Lexer::advance_line() {
    location_.line++;
    location_.column = 1;
}

bool Lexer::is_eof() const {
    return location_.line >= file_->content.lines.size() && pos() >= length();
}

bool Lexer::is_eol() const {
    return pos() >= length();
}

Token Lexer::next() {
    if (auto t = skip_blankspace_and_comments(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_hex_float(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_hex_integer(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_float(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_integer(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_ident(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_punctuation(); !t.IsUninitialized()) {
        return t;
    }

    return {Token::Type::kError, begin_source(),
            (is_null() ? "null character found" : "invalid character found")};
}

Source Lexer::begin_source() const {
    Source src{};
    src.file = file_;
    src.range.begin = location_;
    src.range.end = location_;
    return src;
}

void Lexer::end_source(Source& src) const {
    src.range.end = location_;
}

bool Lexer::is_null() const {
    return (pos() < length()) && (at(pos()) == 0);
}

bool Lexer::is_digit(char ch) const {
    return std::isdigit(static_cast<unsigned char>(ch));
}

bool Lexer::is_hex(char ch) const {
    return std::isxdigit(static_cast<unsigned char>(ch));
}

bool Lexer::matches(size_t pos, std::string_view sub_string) {
    if (pos >= length()) {
        return false;
    }
    return substr(pos, sub_string.size()) == sub_string;
}

Token Lexer::skip_blankspace_and_comments() {
    for (;;) {
        auto loc = location_;
        while (!is_eof()) {
            if (is_eol()) {
                advance_line();
                continue;
            }

            bool is_blankspace;
            size_t blankspace_size;
            if (!read_blankspace(line(), pos(), &is_blankspace, &blankspace_size)) {
                return {Token::Type::kError, begin_source(), "invalid UTF-8"};
            }
            if (!is_blankspace) {
                break;
            }

            advance(blankspace_size);
        }

        auto t = skip_comment();
        if (!t.IsUninitialized()) {
            return t;
        }

        // If the cursor didn't advance we didn't remove any blankspace
        // so we're done.
        if (loc == location_) {
            break;
        }
    }
    if (is_eof()) {
        return {Token::Type::kEOF, begin_source()};
    }

    return {};
}

Token Lexer::skip_comment() {
    if (matches(pos(), "//")) {
        // Line comment: ignore everything until the end of line.
        while (!is_eol()) {
            if (is_null()) {
                return {Token::Type::kError, begin_source(), "null character found"};
            }
            advance();
        }
        return {};
    }

    if (matches(pos(), "/*")) {
        // Block comment: ignore everything until the closing '*/' token.

        // Record source location of the initial '/*'
        auto source = begin_source();
        source.range.end.column += 1;

        advance(2);

        int depth = 1;
        while (!is_eof() && depth > 0) {
            if (matches(pos(), "/*")) {
                // Start of block comment: increase nesting depth.
                advance(2);
                depth++;
            } else if (matches(pos(), "*/")) {
                // End of block comment: decrease nesting depth.
                advance(2);
                depth--;
            } else if (is_eol()) {
                // Newline: skip and update source location.
                advance_line();
            } else if (is_null()) {
                return {Token::Type::kError, begin_source(), "null character found"};
            } else {
                // Anything else: skip and update source location.
                advance();
            }
        }
        if (depth > 0) {
            return {Token::Type::kError, source, "unterminated block comment"};
        }
    }
    return {};
}

Token Lexer::try_float() {
    auto start = pos();
    auto end = pos();

    auto source = begin_source();
    bool has_mantissa_digits = false;

    if (matches(end, "-")) {
        end++;
    }
    while (end < length() && is_digit(at(end))) {
        has_mantissa_digits = true;
        end++;
    }

    bool has_point = false;
    if (end < length() && matches(end, ".")) {
        has_point = true;
        end++;
    }

    while (end < length() && is_digit(at(end))) {
        has_mantissa_digits = true;
        end++;
    }

    if (!has_mantissa_digits) {
        return {};
    }

    // Parse the exponent if one exists
    bool has_exponent = false;
    if (end < length() && (matches(end, "e") || matches(end, "E"))) {
        end++;
        if (end < length() && (matches(end, "+") || matches(end, "-"))) {
            end++;
        }

        while (end < length() && isdigit(at(end))) {
            has_exponent = true;
            end++;
        }

        // If an 'e' or 'E' was present, then the number part must also be present.
        if (!has_exponent) {
            const auto str = std::string{substr(start, end - start)};
            return {Token::Type::kError, source,
                    "incomplete exponent for floating point literal: " + str};
        }
    }

    bool has_f_suffix = false;
    bool has_h_suffix = false;
    if (end < length() && matches(end, "f")) {
        end++;
        has_f_suffix = true;
    } else if (end < length() && matches(end, "h")) {
        end++;
        has_h_suffix = true;
    }

    if (!has_point && !has_exponent && !has_f_suffix && !has_h_suffix) {
        // If it only has digits then it's an integer.
        return {};
    }

    // Save the error string, for use by diagnostics.
    const auto str = std::string{substr(start, end - start)};

    advance(end - start);
    end_source(source);

    double value = std::strtod(&at(start), nullptr);

    if (has_f_suffix) {
        if (auto f = CheckedConvert<f32>(AFloat(value))) {
            return {Token::Type::kFloatLiteral_F, source, static_cast<double>(f.Get())};
        } else {
            return {Token::Type::kError, source, "value cannot be represented as 'f32'"};
        }
    }

    if (has_h_suffix) {
        if (auto f = CheckedConvert<f16>(AFloat(value))) {
            return {Token::Type::kFloatLiteral_H, source, static_cast<double>(f.Get())};
        } else {
            return {Token::Type::kError, source, "value cannot be represented as 'f16'"};
        }
    }

    if (value == HUGE_VAL || -value == HUGE_VAL) {
        return {Token::Type::kError, source, "value cannot be represented as 'abstract-float'"};
    } else {
        return {Token::Type::kFloatLiteral, source, value};
    }
}

Token Lexer::try_hex_float() {
    constexpr uint64_t kExponentBits = 11;
    constexpr uint64_t kMantissaBits = 52;
    constexpr uint64_t kTotalBits = 1 + kExponentBits + kMantissaBits;
    constexpr uint64_t kTotalMsb = kTotalBits - 1;
    constexpr uint64_t kMantissaMsb = kMantissaBits - 1;
    constexpr uint64_t kMantissaShiftRight = kTotalBits - kMantissaBits;
    constexpr int64_t kExponentBias = 1023;
    constexpr uint64_t kExponentMask = (1 << kExponentBits) - 1;
    constexpr int64_t kExponentMax = kExponentMask;  // Including NaN / inf
    constexpr uint64_t kExponentLeftShift = kMantissaBits;
    constexpr uint64_t kSignBit = kTotalBits - 1;
    constexpr uint64_t kOne = 1;

    auto start = pos();
    auto end = pos();

    auto source = begin_source();

    // clang-format off
  // -?0[xX]([0-9a-fA-F]*.?[0-9a-fA-F]+ | [0-9a-fA-F]+.[0-9a-fA-F]*)(p|P)(+|-)?[0-9]+  // NOLINT
    // clang-format on

    // -?
    uint64_t sign_bit = 0;
    if (matches(end, "-")) {
        sign_bit = 1;
        end++;
    }
    // 0[xX]
    if (matches(end, "0x") || matches(end, "0X")) {
        end += 2;
    } else {
        return {};
    }

    uint64_t mantissa = 0;
    uint64_t exponent = 0;

    // TODO(dneto): Values in the normal range for the format do not explicitly
    // store the most significant bit.  The algorithm here works hard to eliminate
    // that bit in the representation during parsing, and then it backtracks
    // when it sees it may have to explicitly represent it, and backtracks again
    // when it sees the number is sub-normal (i.e. the exponent underflows).
    // I suspect the logic can be clarified by storing it during parsing, and
    // then removing it later only when needed.

    // `set_next_mantissa_bit_to` sets next `mantissa` bit starting from msb to
    // lsb to value 1 if `set` is true, 0 otherwise. Returns true on success, i.e.
    // when the bit can be accommodated in the available space.
    uint64_t mantissa_next_bit = kTotalMsb;
    auto set_next_mantissa_bit_to = [&](bool set, bool integer_part) -> bool {
        // If adding bits for the integer part, we can overflow whether we set the
        // bit or not. For the fractional part, we can only overflow when setting
        // the bit.
        const bool check_overflow = integer_part || set;
        // Note: mantissa_next_bit actually decrements, so comparing it as
        // larger than a positive number relies on wraparound.
        if (check_overflow && (mantissa_next_bit > kTotalMsb)) {
            return false;  // Overflowed mantissa
        }
        if (set) {
            mantissa |= (kOne << mantissa_next_bit);
        }
        --mantissa_next_bit;
        return true;
    };

    // Collect integer range (if any)
    auto integer_range = std::make_pair(end, end);
    while (end < length() && is_hex(at(end))) {
        integer_range.second = ++end;
    }

    // .?
    bool hex_point = false;
    if (matches(end, ".")) {
        hex_point = true;
        end++;
    }

    // Collect fractional range (if any)
    auto fractional_range = std::make_pair(end, end);
    while (end < length() && is_hex(at(end))) {
        fractional_range.second = ++end;
    }

    // Must have at least an integer or fractional part
    if ((integer_range.first == integer_range.second) &&
        (fractional_range.first == fractional_range.second)) {
        return {};
    }

    // Is the binary exponent present?  It's optional.
    const bool has_exponent = (matches(end, "p") || matches(end, "P"));
    if (has_exponent) {
        end++;
    }
    if (!has_exponent && !hex_point) {
        // It's not a hex float. At best it's a hex integer.
        return {};
    }

    // At this point, we know for sure our token is a hex float value,
    // or an invalid token.

    // Parse integer part
    // [0-9a-fA-F]*

    bool has_zero_integer = true;
    // The magnitude is zero if and only if seen_prior_one_bits is false.
    bool seen_prior_one_bits = false;
    for (auto i = integer_range.first; i < integer_range.second; ++i) {
        const auto nibble = hex_value(at(i));
        if (nibble != 0) {
            has_zero_integer = false;
        }

        for (int bit = 3; bit >= 0; --bit) {
            auto v = 1 & (nibble >> bit);

            // Skip leading 0s and the first 1
            if (seen_prior_one_bits) {
                if (!set_next_mantissa_bit_to(v != 0, true)) {
                    return {Token::Type::kError, source, "mantissa is too large for hex float"};
                }
                ++exponent;
            } else {
                if (v == 1) {
                    seen_prior_one_bits = true;
                }
            }
        }
    }

    // Parse fractional part
    // [0-9a-fA-F]*
    for (auto i = fractional_range.first; i < fractional_range.second; ++i) {
        auto nibble = hex_value(at(i));
        for (int bit = 3; bit >= 0; --bit) {
            auto v = 1 & (nibble >> bit);

            if (v == 1) {
                seen_prior_one_bits = true;
            }

            // If integer part is 0, we only start writing bits to the
            // mantissa once we have a non-zero fractional bit. While the fractional
            // values are 0, we adjust the exponent to avoid overflowing `mantissa`.
            if (!seen_prior_one_bits) {
                --exponent;
            } else {
                if (!set_next_mantissa_bit_to(v != 0, false)) {
                    return {Token::Type::kError, source, "mantissa is too large for hex float"};
                }
            }
        }
    }

    // Determine if the value of the mantissa is zero.
    // Note: it's not enough to check mantissa == 0 as we drop the initial bit,
    // whether it's in the integer part or the fractional part.
    const bool is_zero = !seen_prior_one_bits;
    TINT_ASSERT(Reader, !is_zero || mantissa == 0);

    // Parse the optional exponent.
    // ((p|P)(\+|-)?[0-9]+)?
    uint64_t input_exponent = 0;  // Defaults to 0 if not present
    int64_t exponent_sign = 1;
    // If the 'p' part is present, the rest of the exponent must exist.
    bool has_f_suffix = false;
    bool has_h_suffix = false;
    if (has_exponent) {
        // Parse the rest of the exponent.
        // (+|-)?
        if (matches(end, "+")) {
            end++;
        } else if (matches(end, "-")) {
            exponent_sign = -1;
            end++;
        }

        // Parse exponent from input
        // [0-9]+
        // Allow overflow (in uint64_t) when the floating point value magnitude is
        // zero.
        bool has_exponent_digits = false;
        while (end < length() && isdigit(at(end))) {
            has_exponent_digits = true;
            auto prev_exponent = input_exponent;
            input_exponent = (input_exponent * 10) + dec_value(at(end));
            // Check if we've overflowed input_exponent. This only matters when
            // the mantissa is non-zero.
            if (!is_zero && (prev_exponent > input_exponent)) {
                return {Token::Type::kError, source, "exponent is too large for hex float"};
            }
            end++;
        }

        // Parse optional 'f' or 'h' suffix.  For a hex float, it can only exist
        // when the exponent is present. Otherwise it will look like
        // one of the mantissa digits.
        if (end < length() && matches(end, "f")) {
            has_f_suffix = true;
            end++;
        } else if (end < length() && matches(end, "h")) {
            has_h_suffix = true;
            end++;
        }

        if (!has_exponent_digits) {
            return {Token::Type::kError, source, "expected an exponent value for hex float"};
        }
    }

    advance(end - start);
    end_source(source);

    if (is_zero) {
        // If value is zero, then ignore the exponent and produce a zero
        exponent = 0;
    } else {
        // Ensure input exponent is not too large; i.e. that it won't overflow when
        // adding the exponent bias.
        const uint64_t kIntMax = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
        const uint64_t kMaxInputExponent = kIntMax - kExponentBias;
        if (input_exponent > kMaxInputExponent) {
            return {Token::Type::kError, source, "exponent is too large for hex float"};
        }

        // Compute exponent so far
        exponent += static_cast<uint64_t>(static_cast<int64_t>(input_exponent) * exponent_sign);

        // Bias exponent if non-zero
        // After this, if exponent is <= 0, our value is a denormal
        exponent += kExponentBias;

        // We know the number is not zero.  The MSB is 1 (by construction), and
        // should be eliminated because it becomes the implicit 1 that isn't
        // explicitly represented in the binary32 format.  We'll bring it back
        // later if we find the exponent actually underflowed, i.e. the number
        // is sub-normal.
        if (has_zero_integer) {
            mantissa <<= 1;
            --exponent;
        }
    }

    // We can now safely work with exponent as a signed quantity, as there's no
    // chance to overflow
    int64_t signed_exponent = static_cast<int64_t>(exponent);

    // Shift mantissa to occupy the low 23 bits
    mantissa >>= kMantissaShiftRight;

    // If denormal, shift mantissa until our exponent is zero
    if (!is_zero) {
        // Denorm has exponent 0 and non-zero mantissa. We set the top bit here,
        // then shift the mantissa to make exponent zero.
        if (signed_exponent <= 0) {
            mantissa >>= 1;
            mantissa |= (kOne << kMantissaMsb);
        }

        while (signed_exponent < 0) {
            mantissa >>= 1;
            ++signed_exponent;

            // If underflow, clamp to zero
            if (mantissa == 0) {
                signed_exponent = 0;
            }
        }
    }

    if (signed_exponent >= kExponentMax || (signed_exponent == kExponentMax && mantissa != 0)) {
        std::string type = has_f_suffix ? "f32" : (has_h_suffix ? "f16" : "abstract-float");
        return {Token::Type::kError, source, "value cannot be represented as '" + type + "'"};
    }

    // Combine sign, mantissa, and exponent
    uint64_t result_u64 = sign_bit << kSignBit;
    result_u64 |= mantissa;
    result_u64 |= (static_cast<uint64_t>(signed_exponent) & kExponentMask) << kExponentLeftShift;

    // Reinterpret as f16 and return
    double result_f64;
    std::memcpy(&result_f64, &result_u64, 8);

    if (has_f_suffix) {
        // Check value fits in f32
        if (result_f64 < static_cast<double>(f32::kLowest) ||
            result_f64 > static_cast<double>(f32::kHighest)) {
            return {Token::Type::kError, source, "value cannot be represented as 'f32'"};
        }
        // Check the value can be exactly represented, i.e. only high 23 mantissa bits are valid for
        // normal f32 values, and less for subnormal f32 values. The rest low mantissa bits must be
        // 0.
        int valid_mantissa_bits = 0;
        double abs_result_f64 = std::fabs(result_f64);
        if (abs_result_f64 >= static_cast<double>(f32::kSmallest)) {
            // The result shall be a normal f32 value.
            valid_mantissa_bits = 23;
        } else if (abs_result_f64 >= static_cast<double>(f32::kSmallestSubnormal)) {
            // The result shall be a subnormal f32 value, represented as double.
            // The smallest positive normal f32 is f32::kSmallest = 2^-126 = 0x1.0p-126, and the
            //   smallest positive subnormal f32 is f32::kSmallestSubnormal = 2^-149. Thus, the
            //   value v in range 2^-126 > v >= 2^-149 must be represented as a subnormal f32
            //   number, but is still normal double (f64) number, and has a exponent in range -127
            //   to -149, inclusive.
            // A value v, if 2^-126 > v >= 2^-127, its binary32 representation will have binary form
            //   s_00000000_1xxxxxxxxxxxxxxxxxxxxxx, having mantissa of 1 leading 1 bit and 22
            //   arbitrary bits. Since this value is represented as normal double number, the
            //   leading 1 bit is omitted, only the highest 22 mantissia bits can be arbitrary, and
            //   the rest lowest 40 mantissa bits of f64 number must be zero.
            // 2^-127 > v >= 2^-128, binary32 s_00000000_01xxxxxxxxxxxxxxxxxxxxx, having mantissa of
            //   1 leading 0 bit, 1 leading 1 bit, and 21 arbitrary bits. The f64 representation
            //   omits the leading 0 and 1 bits, and only the highest 21 mantissia bits can be
            //   arbitrary.
            // 2^-128 > v >= 2^-129, binary32 s_00000000_001xxxxxxxxxxxxxxxxxxxx, 20 arbitrary bits.
            // ...
            // 2^-147 > v >= 2^-148, binary32 s_00000000_0000000000000000000001x, 1 arbitrary bit.
            // 2^-148 > v >= 2^-149, binary32 s_00000000_00000000000000000000001, 0 arbitrary bit.

            // signed_exponent must be in range -149 + 1023 = 874 to -127 + 1023 = 896, inclusive
            TINT_ASSERT(Reader, (874 <= signed_exponent) && (signed_exponent <= 896));
            int unbiased_exponent =
                static_cast<int>(signed_exponent) - static_cast<int>(kExponentBias);
            TINT_ASSERT(Reader, (-149 <= unbiased_exponent) && (unbiased_exponent <= -127));
            valid_mantissa_bits = unbiased_exponent + 149;  // 0 for -149, and 22 for -127
        } else if (abs_result_f64 != 0.0) {
            // The result is smaller than the smallest subnormal f32 value, but not equal to zero.
            // Such value will never be exactly represented by f32.
            return {Token::Type::kError, source, "value cannot be exactly represented as 'f32'"};
        }
        // Check the low 52-valid_mantissa_bits mantissa bits must be 0.
        TINT_ASSERT(Reader, (0 <= valid_mantissa_bits) && (valid_mantissa_bits <= 23));
        if (result_u64 & ((uint64_t(1) << (52 - valid_mantissa_bits)) - 1)) {
            return {Token::Type::kError, source, "value cannot be exactly represented as 'f32'"};
        }
        return {Token::Type::kFloatLiteral_F, source, result_f64};
    } else if (has_h_suffix) {
        // Check value fits in f16
        if (result_f64 < static_cast<double>(f16::kLowest) ||
            result_f64 > static_cast<double>(f16::kHighest)) {
            return {Token::Type::kError, source, "value cannot be represented as 'f16'"};
        }
        // Check the value can be exactly represented, i.e. only high 10 mantissa bits are valid for
        // normal f16 values, and less for subnormal f16 values. The rest low mantissa bits must be
        // 0.
        int valid_mantissa_bits = 0;
        double abs_result_f64 = std::fabs(result_f64);
        if (abs_result_f64 >= static_cast<double>(f16::kSmallest)) {
            // The result shall be a normal f16 value.
            valid_mantissa_bits = 10;
        } else if (abs_result_f64 >= static_cast<double>(f16::kSmallestSubnormal)) {
            // The result shall be a subnormal f16 value, represented as double.
            // The smallest positive normal f16 is f16::kSmallest = 2^-14 = 0x1.0p-14, and the
            //   smallest positive subnormal f16 is f16::kSmallestSubnormal = 2^-24. Thus, the value
            //   v in range 2^-14 > v >= 2^-24 must be represented as a subnormal f16 number, but
            //   is still normal double (f64) number, and has a exponent in range -15 to -24,
            //   inclusive.
            // A value v, if 2^-14 > v >= 2^-15, its binary16 representation will have binary form
            //   s_00000_1xxxxxxxxx, having mantissa of 1 leading 1 bit and 9 arbitrary bits. Since
            //   this value is represented as normal double number, the leading 1 bit is omitted,
            //   only the highest 9 mantissia bits can be arbitrary, and the rest lowest 43 mantissa
            //   bits of f64 number must be zero.
            // 2^-15 > v >= 2^-16, binary16 s_00000_01xxxxxxxx, having mantissa of 1 leading 0 bit,
            //   1 leading 1 bit, and 8 arbitrary bits. The f64 representation omits the leading 0
            //   and 1 bits, and only the highest 8 mantissia bits can be arbitrary.
            // 2^-16 > v >= 2^-17, binary16 s_00000_001xxxxxxx, 7 arbitrary bits.
            // ...
            // 2^-22 > v >= 2^-23, binary16 s_00000_000000001x, 1 arbitrary bits.
            // 2^-23 > v >= 2^-24, binary16 s_00000_0000000001, 0 arbitrary bits.

            // signed_exponent must be in range -24 + 1023 = 999 to -15 + 1023 = 1008, inclusive
            TINT_ASSERT(Reader, (999 <= signed_exponent) && (signed_exponent <= 1008));
            int unbiased_exponent =
                static_cast<int>(signed_exponent) - static_cast<int>(kExponentBias);
            TINT_ASSERT(Reader, (-24 <= unbiased_exponent) && (unbiased_exponent <= -15));
            valid_mantissa_bits = unbiased_exponent + 24;  // 0 for -24, and 9 for -15
        } else if (abs_result_f64 != 0.0) {
            // The result is smaller than the smallest subnormal f16 value, but not equal to zero.
            // Such value will never be exactly represented by f16.
            return {Token::Type::kError, source, "value cannot be exactly represented as 'f16'"};
        }
        // Check the low 52-valid_mantissa_bits mantissa bits must be 0.
        TINT_ASSERT(Reader, (0 <= valid_mantissa_bits) && (valid_mantissa_bits <= 10));
        if (result_u64 & ((uint64_t(1) << (52 - valid_mantissa_bits)) - 1)) {
            return {Token::Type::kError, source, "value cannot be exactly represented as 'f16'"};
        }
        return {Token::Type::kFloatLiteral_H, source, result_f64};
    }

    return {Token::Type::kFloatLiteral, source, result_f64};
}

Token Lexer::build_token_from_int_if_possible(Source source, size_t start, int32_t base) {
    const char* start_ptr = &at(start);
    char* end_ptr = nullptr;

    errno = 0;
    int64_t res = strtoll(start_ptr, &end_ptr, base);
    const bool overflow = errno == ERANGE;

    if (end_ptr) {
        advance(static_cast<size_t>(end_ptr - start_ptr));
    }

    if (matches(pos(), "u")) {
        if (!overflow && CheckedConvert<u32>(AInt(res))) {
            advance(1);
            end_source(source);
            return {Token::Type::kIntLiteral_U, source, res};
        }
        return {Token::Type::kError, source, "value cannot be represented as 'u32'"};
    }

    if (matches(pos(), "i")) {
        if (!overflow && CheckedConvert<i32>(AInt(res))) {
            advance(1);
            end_source(source);
            return {Token::Type::kIntLiteral_I, source, res};
        }
        return {Token::Type::kError, source, "value cannot be represented as 'i32'"};
    }

    end_source(source);
    if (overflow) {
        return {Token::Type::kError, source, "value cannot be represented as 'abstract-int'"};
    }
    return {Token::Type::kIntLiteral, source, res};
}

Token Lexer::try_hex_integer() {
    auto start = pos();
    auto curr = start;

    auto source = begin_source();

    if (matches(curr, "-")) {
        curr++;
    }

    if (matches(curr, "0x") || matches(curr, "0X")) {
        curr += 2;
    } else {
        return {};
    }

    if (!is_hex(at(curr))) {
        return {Token::Type::kError, source,
                "integer or float hex literal has no significant digits"};
    }

    return build_token_from_int_if_possible(source, start, 16);
}

Token Lexer::try_integer() {
    auto start = pos();
    auto curr = start;

    auto source = begin_source();

    if (matches(curr, "-")) {
        curr++;
    }

    if (curr >= length() || !is_digit(at(curr))) {
        return {};
    }

    // If the first digit is a zero this must only be zero as leading zeros
    // are not allowed.
    if (auto next = curr + 1; next < length()) {
        if (at(curr) == '0' && is_digit(at(next))) {
            return {Token::Type::kError, source, "integer literal cannot have leading 0s"};
        }
    }

    return build_token_from_int_if_possible(source, start, 10);
}

Token Lexer::try_ident() {
    auto source = begin_source();
    auto start = pos();

    // Must begin with an XID_Source unicode character, or underscore
    {
        auto* utf8 = reinterpret_cast<const uint8_t*>(&at(pos()));
        auto [code_point, n] = text::utf8::Decode(utf8, length() - pos());
        if (n == 0) {
            advance();  // Skip the bad byte.
            return {Token::Type::kError, source, "invalid UTF-8"};
        }
        if (code_point != text::CodePoint('_') && !code_point.IsXIDStart()) {
            return {};
        }
        // Consume start codepoint
        advance(n);
    }

    while (!is_eol()) {
        // Must continue with an XID_Continue unicode character
        auto* utf8 = reinterpret_cast<const uint8_t*>(&at(pos()));
        auto [code_point, n] = text::utf8::Decode(utf8, line().size() - pos());
        if (n == 0) {
            advance();  // Skip the bad byte.
            return {Token::Type::kError, source, "invalid UTF-8"};
        }
        if (!code_point.IsXIDContinue()) {
            break;
        }

        // Consume continuing codepoint
        advance(n);
    }

    if (at(start) == '_') {
        // Check for an underscore on its own (special token), or a
        // double-underscore (not allowed).
        if ((pos() == start + 1) || (at(start + 1) == '_')) {
            set_pos(start);
            return {};
        }
    }

    auto str = substr(start, pos() - start);
    end_source(source);

    auto t = check_keyword(source, str);
    if (!t.IsUninitialized()) {
        return t;
    }

    return {Token::Type::kIdentifier, source, str};
}

Token Lexer::try_punctuation() {
    auto source = begin_source();
    auto type = Token::Type::kUninitialized;

    if (matches(pos(), "@")) {
        type = Token::Type::kAttr;
        advance(1);
    } else if (matches(pos(), "(")) {
        type = Token::Type::kParenLeft;
        advance(1);
    } else if (matches(pos(), ")")) {
        type = Token::Type::kParenRight;
        advance(1);
    } else if (matches(pos(), "[")) {
        type = Token::Type::kBracketLeft;
        advance(1);
    } else if (matches(pos(), "]")) {
        type = Token::Type::kBracketRight;
        advance(1);
    } else if (matches(pos(), "{")) {
        type = Token::Type::kBraceLeft;
        advance(1);
    } else if (matches(pos(), "}")) {
        type = Token::Type::kBraceRight;
        advance(1);
    } else if (matches(pos(), "&&")) {
        type = Token::Type::kAndAnd;
        advance(2);
    } else if (matches(pos(), "&=")) {
        type = Token::Type::kAndEqual;
        advance(2);
    } else if (matches(pos(), "&")) {
        type = Token::Type::kAnd;
        advance(1);
    } else if (matches(pos(), "/=")) {
        type = Token::Type::kDivisionEqual;
        advance(2);
    } else if (matches(pos(), "/")) {
        type = Token::Type::kForwardSlash;
        advance(1);
    } else if (matches(pos(), "!=")) {
        type = Token::Type::kNotEqual;
        advance(2);
    } else if (matches(pos(), "!")) {
        type = Token::Type::kBang;
        advance(1);
    } else if (matches(pos(), ":")) {
        type = Token::Type::kColon;
        advance(1);
    } else if (matches(pos(), ",")) {
        type = Token::Type::kComma;
        advance(1);
    } else if (matches(pos(), "==")) {
        type = Token::Type::kEqualEqual;
        advance(2);
    } else if (matches(pos(), "=")) {
        type = Token::Type::kEqual;
        advance(1);
    } else if (matches(pos(), ">=")) {
        type = Token::Type::kGreaterThanEqual;
        advance(2);
    } else if (matches(pos(), ">>")) {
        type = Token::Type::kShiftRight;
        advance(2);
    } else if (matches(pos(), ">")) {
        type = Token::Type::kGreaterThan;
        advance(1);
    } else if (matches(pos(), "<=")) {
        type = Token::Type::kLessThanEqual;
        advance(2);
    } else if (matches(pos(), "<<")) {
        type = Token::Type::kShiftLeft;
        advance(2);
    } else if (matches(pos(), "<")) {
        type = Token::Type::kLessThan;
        advance(1);
    } else if (matches(pos(), "%=")) {
        type = Token::Type::kModuloEqual;
        advance(2);
    } else if (matches(pos(), "%")) {
        type = Token::Type::kMod;
        advance(1);
    } else if (matches(pos(), "->")) {
        type = Token::Type::kArrow;
        advance(2);
    } else if (matches(pos(), "--")) {
        type = Token::Type::kMinusMinus;
        advance(2);
    } else if (matches(pos(), "-=")) {
        type = Token::Type::kMinusEqual;
        advance(2);
    } else if (matches(pos(), "-")) {
        type = Token::Type::kMinus;
        advance(1);
    } else if (matches(pos(), ".")) {
        type = Token::Type::kPeriod;
        advance(1);
    } else if (matches(pos(), "++")) {
        type = Token::Type::kPlusPlus;
        advance(2);
    } else if (matches(pos(), "+=")) {
        type = Token::Type::kPlusEqual;
        advance(2);
    } else if (matches(pos(), "+")) {
        type = Token::Type::kPlus;
        advance(1);
    } else if (matches(pos(), "||")) {
        type = Token::Type::kOrOr;
        advance(2);
    } else if (matches(pos(), "|=")) {
        type = Token::Type::kOrEqual;
        advance(2);
    } else if (matches(pos(), "|")) {
        type = Token::Type::kOr;
        advance(1);
    } else if (matches(pos(), ";")) {
        type = Token::Type::kSemicolon;
        advance(1);
    } else if (matches(pos(), "*=")) {
        type = Token::Type::kTimesEqual;
        advance(2);
    } else if (matches(pos(), "*")) {
        type = Token::Type::kStar;
        advance(1);
    } else if (matches(pos(), "~")) {
        type = Token::Type::kTilde;
        advance(1);
    } else if (matches(pos(), "_")) {
        type = Token::Type::kUnderscore;
        advance(1);
    } else if (matches(pos(), "^=")) {
        type = Token::Type::kXorEqual;
        advance(2);
    } else if (matches(pos(), "^")) {
        type = Token::Type::kXor;
        advance(1);
    }

    end_source(source);

    return {type, source};
}

Token Lexer::check_keyword(const Source& source, std::string_view str) {
    if (str == "array") {
        return {Token::Type::kArray, source, "array"};
    }
    if (str == "atomic") {
        return {Token::Type::kAtomic, source, "atomic"};
    }
    if (str == "bitcast") {
        return {Token::Type::kBitcast, source, "bitcast"};
    }
    if (str == "bool") {
        return {Token::Type::kBool, source, "bool"};
    }
    if (str == "break") {
        return {Token::Type::kBreak, source, "break"};
    }
    if (str == "case") {
        return {Token::Type::kCase, source, "case"};
    }
    if (str == "const") {
        return {Token::Type::kConst, source, "const"};
    }
    if (str == "continue") {
        return {Token::Type::kContinue, source, "continue"};
    }
    if (str == "continuing") {
        return {Token::Type::kContinuing, source, "continuing"};
    }
    if (str == "discard") {
        return {Token::Type::kDiscard, source, "discard"};
    }
    if (str == "default") {
        return {Token::Type::kDefault, source, "default"};
    }
    if (str == "else") {
        return {Token::Type::kElse, source, "else"};
    }
    if (str == "enable") {
        return {Token::Type::kEnable, source, "enable"};
    }
    if (str == "f16") {
        return {Token::Type::kF16, source, "f16"};
    }
    if (str == "f32") {
        return {Token::Type::kF32, source, "f32"};
    }
    if (str == "fallthrough") {
        return {Token::Type::kFallthrough, source, "fallthrough"};
    }
    if (str == "false") {
        return {Token::Type::kFalse, source, "false"};
    }
    if (str == "fn") {
        return {Token::Type::kFn, source, "fn"};
    }
    if (str == "for") {
        return {Token::Type::kFor, source, "for"};
    }
    if (str == "function") {
        return {Token::Type::kFunction, source, "function"};
    }
    if (str == "i32") {
        return {Token::Type::kI32, source, "i32"};
    }
    if (str == "if") {
        return {Token::Type::kIf, source, "if"};
    }
    if (str == "import") {
        return {Token::Type::kImport, source, "import"};
    }
    if (str == "let") {
        return {Token::Type::kLet, source, "let"};
    }
    if (str == "loop") {
        return {Token::Type::kLoop, source, "loop"};
    }
    if (str == "mat2x2") {
        return {Token::Type::kMat2x2, source, "mat2x2"};
    }
    if (str == "mat2x3") {
        return {Token::Type::kMat2x3, source, "mat2x3"};
    }
    if (str == "mat2x4") {
        return {Token::Type::kMat2x4, source, "mat2x4"};
    }
    if (str == "mat3x2") {
        return {Token::Type::kMat3x2, source, "mat3x2"};
    }
    if (str == "mat3x3") {
        return {Token::Type::kMat3x3, source, "mat3x3"};
    }
    if (str == "mat3x4") {
        return {Token::Type::kMat3x4, source, "mat3x4"};
    }
    if (str == "mat4x2") {
        return {Token::Type::kMat4x2, source, "mat4x2"};
    }
    if (str == "mat4x3") {
        return {Token::Type::kMat4x3, source, "mat4x3"};
    }
    if (str == "mat4x4") {
        return {Token::Type::kMat4x4, source, "mat4x4"};
    }
    if (str == "override") {
        return {Token::Type::kOverride, source, "override"};
    }
    if (str == "private") {
        return {Token::Type::kPrivate, source, "private"};
    }
    if (str == "ptr") {
        return {Token::Type::kPtr, source, "ptr"};
    }
    if (str == "return") {
        return {Token::Type::kReturn, source, "return"};
    }
    if (str == "sampler") {
        return {Token::Type::kSampler, source, "sampler"};
    }
    if (str == "sampler_comparison") {
        return {Token::Type::kComparisonSampler, source, "sampler_comparison"};
    }
    if (str == "storage_buffer" || str == "storage") {
        return {Token::Type::kStorage, source, "storage"};
    }
    if (str == "struct") {
        return {Token::Type::kStruct, source, "struct"};
    }
    if (str == "switch") {
        return {Token::Type::kSwitch, source, "switch"};
    }
    if (str == "texture_1d") {
        return {Token::Type::kTextureSampled1d, source, "texture_1d"};
    }
    if (str == "texture_2d") {
        return {Token::Type::kTextureSampled2d, source, "texture_2d"};
    }
    if (str == "texture_2d_array") {
        return {Token::Type::kTextureSampled2dArray, source, "texture_2d_array"};
    }
    if (str == "texture_3d") {
        return {Token::Type::kTextureSampled3d, source, "texture_3d"};
    }
    if (str == "texture_cube") {
        return {Token::Type::kTextureSampledCube, source, "texture_cube"};
    }
    if (str == "texture_cube_array") {
        return {Token::Type::kTextureSampledCubeArray, source, "texture_cube_array"};
    }
    if (str == "texture_depth_2d") {
        return {Token::Type::kTextureDepth2d, source, "texture_depth_2d"};
    }
    if (str == "texture_depth_2d_array") {
        return {Token::Type::kTextureDepth2dArray, source, "texture_depth_2d_array"};
    }
    if (str == "texture_depth_cube") {
        return {Token::Type::kTextureDepthCube, source, "texture_depth_cube"};
    }
    if (str == "texture_depth_cube_array") {
        return {Token::Type::kTextureDepthCubeArray, source, "texture_depth_cube_array"};
    }
    if (str == "texture_depth_multisampled_2d") {
        return {Token::Type::kTextureDepthMultisampled2d, source, "texture_depth_multisampled_2d"};
    }
    if (str == "texture_external") {
        return {Token::Type::kTextureExternal, source, "texture_external"};
    }
    if (str == "texture_multisampled_2d") {
        return {Token::Type::kTextureMultisampled2d, source, "texture_multisampled_2d"};
    }
    if (str == "texture_storage_1d") {
        return {Token::Type::kTextureStorage1d, source, "texture_storage_1d"};
    }
    if (str == "texture_storage_2d") {
        return {Token::Type::kTextureStorage2d, source, "texture_storage_2d"};
    }
    if (str == "texture_storage_2d_array") {
        return {Token::Type::kTextureStorage2dArray, source, "texture_storage_2d_array"};
    }
    if (str == "texture_storage_3d") {
        return {Token::Type::kTextureStorage3d, source, "texture_storage_3d"};
    }
    if (str == "true") {
        return {Token::Type::kTrue, source, "true"};
    }
    if (str == "type") {
        return {Token::Type::kType, source, "type"};
    }
    if (str == "u32") {
        return {Token::Type::kU32, source, "u32"};
    }
    if (str == "uniform") {
        return {Token::Type::kUniform, source, "uniform"};
    }
    if (str == "var") {
        return {Token::Type::kVar, source, "var"};
    }
    if (str == "vec2") {
        return {Token::Type::kVec2, source, "vec2"};
    }
    if (str == "vec3") {
        return {Token::Type::kVec3, source, "vec3"};
    }
    if (str == "vec4") {
        return {Token::Type::kVec4, source, "vec4"};
    }
    if (str == "while") {
        return {Token::Type::kWhile, source, "while"};
    }
    if (str == "workgroup") {
        return {Token::Type::kWorkgroup, source, "workgroup"};
    }
    return {};
}

}  // namespace tint::reader::wgsl
