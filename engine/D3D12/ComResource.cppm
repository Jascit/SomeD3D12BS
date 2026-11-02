/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    ComResource.cppm
 * @brief
 *
 * @date    26/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;

#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <string>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.com_resource;
import d3d12.utils;
import core.log;
using Microsoft::WRL::ComPtr;

template <typename ComObj>
concept d3d12_object = std::derived_from<ComObj, ID3D12Object>;
template <typename ComObj>
concept dxgi_object = std::derived_from<ComObj, IDXGIObject>;

export namespace d3d12
{

template <typename ComObj> class ComResource
{
  protected:
    using interface_type = ComObj;
    using interface_pointer = interface_type *;

  public:
    ~ComResource() noexcept = default;
    ComResource() noexcept = default;
    ComResource(ComResource &&other) noexcept = default;
    ComResource(const ComResource &other) noexcept = delete;
    auto operator=(const ComResource &other) noexcept -> ComResource & = delete;
    auto operator=(ComResource &&other) noexcept -> ComResource & = default;

    explicit ComResource(ComPtr<ComObj> &&obj, const char *name) noexcept : interface_(std::move(obj))
    {
        set_name(name);
    };

    [[nodiscard]] auto get() const noexcept -> interface_type *
    {
        return interface_.Get();
    };

    [[nodiscard]] auto put() noexcept -> interface_type **
    {
        return interface_.ReleaseAndGetAddressOf();
    };

    auto operator->() const noexcept
    {
        return interface_.Get();
    };

    explicit operator bool() const noexcept
    {
        return interface_ != nullptr;
    };

    void set_name(const char *name) noexcept
    {
        if (interface_ == nullptr || name == nullptr)
        {
            return;
        }
        if constexpr (d3d12_object<ComObj>)
        {
            std::size_t length = std::strlen(name);
            std::string_view view(name, length);
            std::wstring wide_name = utils::to_wide(view);
            if (FAILED(interface_->SetName(wide_name.data())))
            {
                core::diagnostics::log_warning("Failed to name object! Name: {}", name);
            };
        }
        else if constexpr (dxgi_object<ComObj>)
        {
            std::uint64_t length = std::strlen(name);
            if (FAILED(interface_->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(length), name)))
            {
                core::diagnostics::log_warning("Failed to name object! Name: {}", name);
            }
        }
        else
        {
            static_assert(false, "Undefined type!");
        }
    }

    [[nodiscard]] auto get_name() noexcept -> std::string
    {
        if (interface_ == nullptr)
        {
            return {};
        }
        if constexpr (d3d12_object<ComObj>)
        {
            std::uint32_t wlength = 0;
            if (FAILED(interface_->GetPrivateData(WKPDID_D3DDebugObjectNameW, &wlength, nullptr)))
            {
                return {};
            }
            std::wstring wide_name(wlength, L'\0');
            if (FAILED(interface_->GetPrivateData(WKPDID_D3DDebugObjectNameW, &wlength, wide_name.data())))
            {
                return {};
            }
            std::string name = utils::to_utf8(wide_name);
            return name;
        }
        else if constexpr (dxgi_object<ComObj>)
        {
            std::uint32_t length = 0;
            if (FAILED(interface_->GetPrivateData(WKPDID_D3DDebugObjectName, &length, nullptr)))
            {
                return {};
            }
            std::string name(length, '\0');
            if (FAILED(interface_->GetPrivateData(WKPDID_D3DDebugObjectName, &length, name.data())))
            {
                return {};
            }
            return name;
        }
        else
        {
            static_assert(false, "Undefined type!");
            return {};
        }
    }

  protected:
    ComPtr<interface_type> interface_;
};
} // namespace d3d12