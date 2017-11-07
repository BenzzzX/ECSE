#pragma once

#include <type_traits>
#include "./TypeList.hpp"
#include "./Count.hpp"

namespace eecs
{
    namespace MPL
    {
        namespace Impl
        {
            template <typename TCheckTypeList, typename TTypeList>
            struct ContainsAllHelper;

            template <typename TCheckTypeList, typename TTypeList>
            using ContainsAll =
                typename ContainsAllHelper<TCheckTypeList, TTypeList>::type;

            template <typename T, typename... TRest, typename TTypeList>
            struct ContainsAllHelper<TypeList<T, TRest...>, TTypeList>
                : std::integral_constant<bool,
                      Contains<T, TTypeList>::value &&
                          ContainsAll<TypeList<TRest...>, TTypeList>::value>
            {
            };

            template <typename TTypeList>
            struct ContainsAllHelper<TypeList<>, TTypeList> : std::true_type
            {
            };
        }
    }
}