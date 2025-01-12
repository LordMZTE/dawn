// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_DEVICEINFOD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_DEVICEINFOD3D11_H_

#include "dawn/native/Error.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Adapter;

struct DeviceInfo {
    bool isUMA;

    // shaderModel indicates the maximum supported shader model, for example, the value 62
    // indicates that current driver supports the maximum shader model is shader model 6.2.
    uint32_t shaderModel;
    PerStage<std::wstring> shaderProfiles;
};

ResultOrError<DeviceInfo> GatherDeviceInfo(const Adapter& adapter);
}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_DEVICEINFOD3D11_H_
