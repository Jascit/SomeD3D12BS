/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Factory.cppm
 * @brief
 *
 * @date    25/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <dxgi1_6.h>
#include <wrl/client.h>
export module d3d12.context:factory;
import d3d12.com_resource;
using namespace Microsoft::WRL;

export namespace d3d12
{
class Factory : public ComResource<IDXGIFactory6>
{
    using base = ComResource;

  public:
    static auto create(const char *name) noexcept -> Factory;

};
} // namespace d3d12