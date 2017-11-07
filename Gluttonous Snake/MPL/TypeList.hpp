#pragma once

namespace eecs
{
    namespace MPL
    {
        namespace Impl
        {
            // Compile-time list of types.
            template <typename... Ts>
            struct TypeList
            {
                // Size of the list.
                static constexpr std::size_t size{sizeof...(Ts)};
            };
        }
    }
}