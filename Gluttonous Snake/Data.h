#pragma once
#include "UtilSystem.h"
#include "Serialize.h"
#include "Cache.h"
#include "Render.h"
#include "Input.h"
#include "Tick.h"

struct CInput {
  char v;
};

struct CSnake {
  size_t length;
};

struct CFoodSpawner {
  size_t interval;
  size_t remain;
  size_t holdrand;
};

using Components =
    EEC::ComponentList<CPosition, CVelocity, CAppearance, CCollision,
                        CLifeTime, CSnake, CInput, CFoodSpawner, CUniqueID>;
using Tags = EEC::TagList 
    <struct TPlayer, struct TFood, struct TSnakeBody, struct TFood>;
using Events = EEC::EventList<TickEvent, CollisionEvent, KeyEvent, RewindEvent,
                               CacheEvent>;
using Singletons = EEC::SingletonList
    <SceneData, Cache, UniqueData, ScreenBuffer>;

using World = EEC::World
    <Components, Tags, Events, Singletons>;

template <typename... Ts> using EntityTemplate = World::EntityTemplate<Ts...>;

auto snakeTemplate = World::make_template<TPlayer>(
    CPosition{7, 4}, CAppearance{'a'}, CVelocity{1, 0}, CSnake{5u},
    CCollision{Block}, CInput{});

auto foodTemplate = World::make_template<TFood>(
    CPosition{0, 0}, CAppearance{'@'}, CCollision{Overlap});
