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

#ifndef SRC_DAWN_NATIVE_D3D11_ADAPTERD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_ADAPTERD3D11_H_

#include "dawn/native/d3d/AdapterD3D.h"

#include "dawn/native/d3d/d3d_platform.h"
#include "dawn/native/d3d11/DeviceInfoD3D11.h"

namespace dawn::native::d3d11 {

class Backend;

class Adapter : public d3d::Adapter {
  public:
    Adapter(Backend* backend,
            ComPtr<IDXGIAdapter3> hardwareAdapter,
            const TogglesState& adapterToggles);
    ~Adapter() override;

    // AdapterBase Implementation
    bool SupportsExternalImages() const override;

    const DeviceInfo& GetDeviceInfo() const;
    ComPtr<ID3D11Device> GetD3D11Device() const;

  private:
    using Base = d3d::Adapter;

    void SetupBackendDeviceToggles(TogglesState* deviceToggles) const override;

    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(const DeviceDescriptor* descriptor,
                                                    const TogglesState& deviceToggles) override;

    MaybeError ResetInternalDeviceForTestingImpl() override;

    MaybeError InitializeImpl() override;
    void InitializeSupportedFeaturesImpl() override;
    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override;

    MaybeError ValidateFeatureSupportedWithTogglesImpl(wgpu::FeatureName feature,
                                                       const TogglesState& toggles) const override;
    ComPtr<ID3D11Device> mD3d11Device;
    D3D_FEATURE_LEVEL mFeatureLevel;

    DeviceInfo mDeviceInfo = {};
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_ADAPTERD3D11_H_
