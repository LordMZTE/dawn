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

#ifndef SRC_TINT_READER_WGSL_TOKEN_H_
#define SRC_TINT_READER_WGSL_TOKEN_H_

#include <string>
#include <string_view>
#include <variant>

#include "src/tint/source.h"

namespace tint::reader::wgsl {

/// Stores tokens generated by the Lexer
class Token {
  public:
    /// The type of the parsed token
    enum class Type {
        /// Error result
        kError = -2,
        /// Uninitialized token
        kUninitialized = 0,
        /// Placeholder token which maybe fillled in later
        kPlaceholder = 1,
        /// End of input string reached
        kEOF,

        /// An identifier
        kIdentifier,
        /// A float literal with no suffix
        kFloatLiteral,
        /// A float literal with an 'f' suffix
        kFloatLiteral_F,
        /// A float literal with an 'h' suffix
        kFloatLiteral_H,
        /// An integer literal with no suffix
        kIntLiteral,
        /// An integer literal with an 'i' suffix
        kIntLiteral_I,
        /// An integer literal with a 'u' suffix
        kIntLiteral_U,

        /// A '&'
        kAnd,
        /// A '&&'
        kAndAnd,
        /// A '->'
        kArrow,
        /// A '@'
        kAttr,
        /// A '/'
        kForwardSlash,
        /// A '!'
        kBang,
        /// A '['
        kBracketLeft,
        /// A ']'
        kBracketRight,
        /// A '{'
        kBraceLeft,
        /// A '}'
        kBraceRight,
        /// A ':'
        kColon,
        /// A ','
        kComma,
        /// A '='
        kEqual,
        /// A '=='
        kEqualEqual,
        /// A '>'
        kGreaterThan,
        /// A '>='
        kGreaterThanEqual,
        /// A '>>'
        kShiftRight,
        /// A '<'
        kLessThan,
        /// A '<='
        kLessThanEqual,
        /// A '<<'
        kShiftLeft,
        /// A '%'
        kMod,
        /// A '-'
        kMinus,
        /// A '--'
        kMinusMinus,
        /// A '!='
        kNotEqual,
        /// A '.'
        kPeriod,
        /// A '+'
        kPlus,
        /// A '++'
        kPlusPlus,
        /// A '|'
        kOr,
        /// A '||'
        kOrOr,
        /// A '('
        kParenLeft,
        /// A ')'
        kParenRight,
        /// A ';'
        kSemicolon,
        /// A '*'
        kStar,
        /// A '~'
        kTilde,
        /// A '_'
        kUnderscore,
        /// A '^'
        kXor,
        /// A '+='
        kPlusEqual,
        /// A '-='
        kMinusEqual,
        /// A '*='
        kTimesEqual,
        /// A '/='
        kDivisionEqual,
        /// A '%='
        kModuloEqual,
        /// A '&='
        kAndEqual,
        /// A '|='
        kOrEqual,
        /// A '^='
        kXorEqual,
        /// A '>>='
        kShiftRightEqual,
        /// A '<<='
        kShiftLeftEqual,

        /// A 'array'
        kArray,
        /// A 'atomic'
        kAtomic,
        /// A 'bitcast'
        kBitcast,
        /// A 'bool'
        kBool,
        /// A 'break'
        kBreak,
        /// A 'case'
        kCase,
        /// A 'const'
        kConst,
        /// A 'continue'
        kContinue,
        /// A 'continuing'
        kContinuing,
        /// A 'discard'
        kDiscard,
        /// A 'default'
        kDefault,
        /// A 'else'
        kElse,
        /// A 'enable'
        kEnable,
        /// A 'f16'
        kF16,
        /// A 'f32'
        kF32,
        /// A 'fallthrough'
        kFallthrough,
        /// A 'false'
        kFalse,
        /// A 'fn'
        kFn,
        // A 'for'
        kFor,
        /// A 'i32'
        kI32,
        /// A 'if'
        kIf,
        /// A 'let'
        kLet,
        /// A 'loop'
        kLoop,
        /// A 'mat2x2'
        kMat2x2,
        /// A 'mat2x3'
        kMat2x3,
        /// A 'mat2x4'
        kMat2x4,
        /// A 'mat3x2'
        kMat3x2,
        /// A 'mat3x3'
        kMat3x3,
        /// A 'mat3x4'
        kMat3x4,
        /// A 'mat4x2'
        kMat4x2,
        /// A 'mat4x3'
        kMat4x3,
        /// A 'mat4x4'
        kMat4x4,
        /// A 'override'
        kOverride,
        /// A 'ptr'
        kPtr,
        /// A 'return'
        kReturn,
        /// A 'sampler'
        kSampler,
        /// A 'sampler_comparison'
        kComparisonSampler,
        /// A 'static_assert'
        kStaticAssert,
        /// A 'struct'
        kStruct,
        /// A 'switch'
        kSwitch,
        /// A 'texture_depth_2d'
        kTextureDepth2d,
        /// A 'texture_depth_2d_array'
        kTextureDepth2dArray,
        /// A 'texture_depth_cube'
        kTextureDepthCube,
        /// A 'texture_depth_cube_array'
        kTextureDepthCubeArray,
        /// A 'texture_depth_multisampled_2d'
        kTextureDepthMultisampled2d,
        /// A 'texture_external'
        kTextureExternal,
        /// A 'texture_multisampled_2d'
        kTextureMultisampled2d,
        /// A 'texture_1d'
        kTextureSampled1d,
        /// A 'texture_2d'
        kTextureSampled2d,
        /// A 'texture_2d_array'
        kTextureSampled2dArray,
        /// A 'texture_3d'
        kTextureSampled3d,
        /// A 'texture_cube'
        kTextureSampledCube,
        /// A 'texture_cube_array'
        kTextureSampledCubeArray,
        /// A 'texture_storage_1d'
        kTextureStorage1d,
        /// A 'texture_storage_2d'
        kTextureStorage2d,
        /// A 'texture_storage_2d_array'
        kTextureStorage2dArray,
        /// A 'texture_storage_3d'
        kTextureStorage3d,
        /// A 'true'
        kTrue,
        /// A 'type'
        kType,
        /// A 'u32'
        kU32,
        /// A 'var'
        kVar,
        /// A 'vec2'
        kVec2,
        /// A 'vec3'
        kVec3,
        /// A 'vec4'
        kVec4,
        /// A 'while'
        kWhile,
    };

    /// Converts a token type to a name
    /// @param type the type to convert
    /// @returns the token type as as string
    static std::string_view TypeToName(Type type);

    /// Creates an uninitialized token
    Token();
    /// Create a Token
    /// @param type the Token::Type of the token
    /// @param source the source of the token
    Token(Type type, const Source& source);

    /// Create a string Token
    /// @param type the Token::Type of the token
    /// @param source the source of the token
    /// @param view the source string view for the token
    Token(Type type, const Source& source, const std::string_view& view);
    /// Create a string Token
    /// @param type the Token::Type of the token
    /// @param source the source of the token
    /// @param str the source string for the token
    Token(Type type, const Source& source, const std::string& str);
    /// Create a string Token
    /// @param type the Token::Type of the token
    /// @param source the source of the token
    /// @param str the source string for the token
    Token(Type type, const Source& source, const char* str);
    /// Create a integer Token of the given type
    /// @param type the Token::Type of the token
    /// @param source the source of the token
    /// @param val the source unsigned for the token
    Token(Type type, const Source& source, int64_t val);
    /// Create a double Token
    /// @param type the Token::Type of the token
    /// @param source the source of the token
    /// @param val the source double for the token
    Token(Type type, const Source& source, double val);
    /// Move constructor
    Token(Token&&);
    ~Token();

    /// Equality operator with an identifier
    /// @param ident the identifier string
    /// @return true if this token is an identifier and is equal to ident.
    bool operator==(std::string_view ident) const;

    /// Sets the token to the given type
    /// @param type the type to set
    void SetType(Token::Type type) { type_ = type; }

    /// Returns true if the token is of the given type
    /// @param t the type to check against.
    /// @returns true if the token is of type `t`
    bool Is(Type t) const { return type_ == t; }

    /// @returns true if the token is uninitialized
    bool IsUninitialized() const { return type_ == Type::kUninitialized; }
    /// @returns true if the token is a placeholder
    bool IsPlaceholder() const { return type_ == Type::kPlaceholder; }
    /// @returns true if the token is EOF
    bool IsEof() const { return type_ == Type::kEOF; }
    /// @returns true if the token is Error
    bool IsError() const { return type_ == Type::kError; }
    /// @returns true if the token is an identifier
    bool IsIdentifier() const { return type_ == Type::kIdentifier; }
    /// @returns true if the token is a literal
    bool IsLiteral() const {
        return type_ == Type::kIntLiteral || type_ == Type::kIntLiteral_I ||
               type_ == Type::kIntLiteral_U || type_ == Type::kFalse || type_ == Type::kTrue ||
               type_ == Type::kFloatLiteral || type_ == Type::kFloatLiteral_F ||
               type_ == Type::kFloatLiteral_H;
    }
    /// @returns true if token is a 'matNxM'
    bool IsMatrix() const {
        return type_ == Type::kMat2x2 || type_ == Type::kMat2x3 || type_ == Type::kMat2x4 ||
               type_ == Type::kMat3x2 || type_ == Type::kMat3x3 || type_ == Type::kMat3x4 ||
               type_ == Type::kMat4x2 || type_ == Type::kMat4x3 || type_ == Type::kMat4x4;
    }
    /// @returns true if token is a 'mat3xM'
    bool IsMat3xN() const {
        return type_ == Type::kMat3x2 || type_ == Type::kMat3x3 || type_ == Type::kMat3x4;
    }
    /// @returns true if token is a 'mat4xM'
    bool IsMat4xN() const {
        return type_ == Type::kMat4x2 || type_ == Type::kMat4x3 || type_ == Type::kMat4x4;
    }
    /// @returns true if token is a 'matNx3'
    bool IsMatNx3() const {
        return type_ == Type::kMat2x3 || type_ == Type::kMat3x3 || type_ == Type::kMat4x3;
    }
    /// @returns true if token is a 'matNx4'
    bool IsMatNx4() const {
        return type_ == Type::kMat2x4 || type_ == Type::kMat3x4 || type_ == Type::kMat4x4;
    }

    /// @returns true if token is a 'vecN'
    bool IsVector() const {
        return type_ == Type::kVec2 || type_ == Type::kVec3 || type_ == Type::kVec4;
    }

    /// @returns true if the token can be split during parse into component tokens
    bool IsSplittable() const {
        return Is(Type::kShiftRight) || Is(Type::kGreaterThanEqual) || Is(Type::kAndAnd) ||
               Is(Type::kMinusMinus);
    }

    /// @returns true if the token is a binary operator
    bool IsBinaryOperator() const {
        switch (type_) {
            case Type::kAnd:
            case Type::kAndAnd:
            case Type::kEqualEqual:
            case Type::kForwardSlash:
            case Type::kGreaterThan:
            case Type::kGreaterThanEqual:
            case Type::kLessThan:
            case Type::kLessThanEqual:
            case Type::kMinus:
            case Type::kMod:
            case Type::kNotEqual:
            case Type::kOr:
            case Type::kOrOr:
            case Type::kPlus:
            case Type::kShiftLeft:
            case Type::kShiftRight:
            case Type::kStar:
            case Type::kXor:
                return true;
            default:
                return false;
        }
    }

    /// @returns the source information for this token
    Source source() const { return source_; }

    /// @returns the type of the token
    Type type() const { return type_; }

    /// Returns the string value of the token
    /// @return std::string
    std::string to_str() const;
    /// Returns the float value of the token. 0 is returned if the token does not
    /// contain a float value.
    /// @return double
    double to_f64() const;
    /// Returns the int64_t value of the token. 0 is returned if the token does
    /// not contain an integer value.
    /// @return int64_t
    int64_t to_i64() const;

    /// @returns the token type as string
    std::string_view to_name() const { return Token::TypeToName(type_); }

  private:
    /// The Token::Type of the token
    Type type_ = Type::kError;
    /// The source where the token appeared
    Source source_;
    /// The value represented by the token
    std::variant<int64_t, double, std::string, std::string_view> value_;
};

#ifndef NDEBUG
inline std::ostream& operator<<(std::ostream& out, Token::Type type) {
    out << Token::TypeToName(type);
    return out;
}
#endif  // NDEBUG

}  // namespace tint::reader::wgsl

#endif  // SRC_TINT_READER_WGSL_TOKEN_H_