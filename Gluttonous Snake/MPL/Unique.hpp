#pragma once

#include <type_traits>
#include "./TypeList.hpp"
#include "./Count.hpp"

namespace EEC
{
    namespace MPL
    {
        namespace Impl
        {
            template <typename TTypeList>
            struct UniqueHelper : std::true_type
            {
            };

            template <typename T, typename... Ts>
            struct UniqueHelper<TypeList<T, Ts...>>
                : std::integral_constant<bool,
                      (Count<T, TypeList<T, Ts...>>::value <= 1) &&
                          UniqueHelper<TypeList<Ts...>>::value>
            {
            };

            template <typename... Ts>
            using Unique = UniqueHelper<TypeList<Ts...>>;
        }
    }
}