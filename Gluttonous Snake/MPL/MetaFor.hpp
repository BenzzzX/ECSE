#pragma once

#include <tuple>

namespace eecs {
    namespace Utils {
        namespace Impl {
            template <typename TFunction, typename TTuple, std::size_t... TIndices>
            constexpr decltype(auto) tupleApplyImpl(TFunction&& mFunction, TTuple&& mTuple, std::index_sequence<TIndices...>) {
                return MPL_FWD(mFunction)(std::get<TIndices>(MPL_FWD(mTuple))...);
            }

            template <typename TFunction, typename TTuple>
            constexpr decltype(auto) tupleApply(TFunction&& mFunction, TTuple&& mTuple) {
                using Indices = std::make_index_sequence<
                    std::tuple_size<std::decay_t<TTuple>>::value>;
                return tupleApplyImpl(
                    MPL_FWD(mFunction), MPL_FWD(mTuple), Indices{});
            }

            template <typename TFunction, typename... Ts>
            inline constexpr decltype(auto) forArgs(TFunction&& mFunction, Ts&&... mArgs) {
                return (void)std::initializer_list<int>{
                    (mFunction(MPL_FWD(mArgs)), 0)...};
            }

            template <typename TFunction, typename TTuple>
            constexpr decltype(auto) forTuple(TFunction&& mFunction, TTuple&& mTuple) {
                return tupleApply(
                    [&mFunction](auto&&... xs)
                    {
                        forArgs(mFunction, MPL_FWD(xs)...);
                    },
                    MPL_FWD(mTuple));
            }
        }

        // "Unpacks" the contents of a tuple inside a function call.
        template <typename TFunction, typename TTuple>
        constexpr decltype(auto) tupleApply(TFunction&& mFunction, TTuple&& mTuple) {
            return Impl::tupleApply(mFunction, MPL_FWD(mTuple));
        }

        // Invokes a function on every passed object.
        template <typename TFunction, typename... Ts>
        constexpr decltype(auto) forArgs(TFunction&& mFunction, Ts&&... mArgs) {
            return Impl::forArgs(mFunction, MPL_FWD(mArgs)...);
        }

        // Invokes a function on every element of a tuple.
        template <typename TFunction, typename TTuple>
        constexpr decltype(auto) forTuple(TFunction&& mFunction, TTuple&& mTuple) {
            return Impl::forTuple(mFunction, MPL_FWD(mTuple));
        }
    }
}
