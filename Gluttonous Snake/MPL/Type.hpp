#pragma once

namespace EEC
{
    namespace MPL
    {
        namespace Impl
        {
            // Simple wrapper for types that can be instantiated.
            template <typename T>
            struct Type
            {
                using type = T;
            };
        }
    }
}