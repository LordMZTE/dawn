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

#include "src/tint/ir/construct.h"
#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Construct);

namespace tint::ir {

Construct::Construct(Value* result, utils::VectorRef<Value*> args) : Base(result, args) {}

Construct::~Construct() = default;

utils::StringStream& Construct::ToString(utils::StringStream& out, const SymbolTable& st) const {
    Result()->ToString(out, st);
    out << " = construct(" << Result()->Type()->FriendlyName(st);
    if (!Args().IsEmpty()) {
        out << ", ";
        EmitArgs(out, st);
    }
    out << ")";
    return out;
}

}  // namespace tint::ir
