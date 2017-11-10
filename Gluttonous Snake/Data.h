#pragma once
#include "GeneralData.h"

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
    eecs::ComponentList<CPosition, CVelocity, CAppearance, CCollision,
                        CLifeTime, CSnake, CInput, CFoodSpawner, CUniqueID>;
using Tags = eecs::TagList 
    <struct TPlayer, struct TFood, struct TSnakeBody, struct TFood>;
using Events = eecs::EventList<TickEvent, CollisionEvent, KeyEvent, RewindEvent,
                               CacheEvent>;
using Singletons = eecs::SingletonList
    <SceneData, Cache, UniqueData>;

using World = eecs::World
    <Components, Tags, Events, Singletons>;

template <typename... Ts> using EntityTemplate = World::EntityTemplate<Ts...>;

auto snakeTemplate = World::make_template<TPlayer>(
    CPosition{7, 4}, CAppearance{'a'}, CVelocity{1, 0}, CSnake{5u},
    CCollision{Block}, CInput{});

auto foodTemplate = World::make_template<TFood>(
    CPosition{0, 0}, CAppearance{'@'}, CCollision{Overlap});
