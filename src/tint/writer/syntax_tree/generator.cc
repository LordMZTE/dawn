// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/syntax_tree/generator.h"
#include "src/tint/writer/syntax_tree/generator_impl.h"

namespace tint::writer::syntax_tree {

Result::Result() = default;
Result::~Result() = default;
Result::Result(const Result&) = default;

Result Generate(const Program* program, const Options&) {
    Result result;

    // Generate the AST dump.
    auto impl = std::make_unique<GeneratorImpl>(program);
    result.success = impl->Generate();
    result.error = impl->error();
    result.ast = impl->result();

    return result;
}

}  // namespace tint::writer::syntax_tree
