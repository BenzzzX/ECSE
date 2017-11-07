#pragma once

#define MPL_FWD(mX) ::std::forward<decltype(mX)>(mX)

#include "./StrongTypedef.hpp"
#include "./MetaFor.hpp"
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
#include "./Interface.hpp"
#include "./Tests.hpp"
