#pragma once

#include <type_traits>
#include "./Type.hpp"
#include "./TypeList.hpp"
#include "./Rename.hpp"
#include "./Concat.hpp"
#include "./Map.hpp"
#include "./TypeListOps.hpp"
#include "./Repeat.hpp"
#include "./IndexOf.hpp"
#include "./Count.hpp"
#include "./ContainsAll.hpp"
#include "./Filter.hpp"
#include "./Unique.hpp"


namespace eecs
{
    namespace MPL
    {
        using Impl::Type;
        using Impl::Tuple;
        using Impl::TypeList;
        using Impl::Nth;
        using Impl::Concat;
        using Impl::PushBack;
        using Impl::PushFront;
        using Impl::Repeat;
        using Impl::IndexOf;
        using Impl::Map;
        using Impl::Contains;
        using Impl::ContainsAll;
        using Impl::Filter;
        using Impl::Head;
        using Impl::Tail;
        using Impl::Rename;
        using Impl::Count;
        using Impl::Unique;

        template <typename TTypeList>
        constexpr auto size() noexcept
        {
            return Impl::size<TTypeList>();
        }

        template <typename TTypeList, typename TFunction>
        constexpr void forTypes(TFunction&& mFunction) noexcept
        {
            Impl::forTypes<TTypeList>(MPL_FWD(mFunction));
        }

        template <typename... Ts, typename TFunction>
        constexpr void forTypesExp(TFunction&& mFunction) noexcept
        {
            forTypes<TypeList<Ts...>>(MPL_FWD(mFunction));
        }
    }
}
