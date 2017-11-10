#pragma once
#include "Tick.h"
using BYTE = unsigned char;

class SceneData {
	using EntityID = size_t;

	std::vector<std::vector<EntityID>> content;
	auto &access(size_t x, size_t y) { return content[y * width + x]; }
public:
	auto &get(size_t x, size_t y) { return access(x, y); }
	void clr() {
		for (int i = 0; i < height * width; i++)
			content[i].clear();
	}
	void add(size_t x, size_t y, EntityID eI) { access(x, y).push_back(eI); }
	size_t width, height;
	bool initialized = false;
	SceneData() {}
	SceneData(size_t w, size_t h) : width(w), height(h), initialized(true) {
		content.resize(width * height);
	}
};

enum CollisionResponse { Block, Overlap };

struct CCollision {
	CollisionResponse response;
};

struct CollisionEvent {
	CollisionResponse response;
	size_t id1, id2;
};

struct CLifeTime {
	size_t frames;
};

struct CPosition {
	int x, y;
};

struct CVelocity {
	int x, y;
};

namespace Util {
	size_t randnext(size_t &holdrand) {
		return (holdrand = ((holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
	}
}

namespace Systems {
	template <typename World> using System = eecs::System<World>;

	template <typename World> class PhysicalMovementSystem : System<World> {
		using SMovement = eecs::Signature<CPosition, CVelocity>;
		using SCollision = eecs::Signature<CPosition, CCollision>;

		static auto &position_trace(World &world, size_t x, size_t y) {
			auto &scenedata = world.template get_singleton<SceneData>();
			return scenedata.get(x, y);
		}

		static int bounce(int x, size_t limit) {
			while (x < 0)
				x += static_cast<int>(limit);
			return x % limit;
		}

		inline static void Tick(World &world) {
			SceneData &scene = world.template get_singleton<SceneData>();
			scene.clr();
			world.template for_matching<SCollision>(
				[&scene](auto &proxy, auto &pos, auto &cls) {
				scene.add(pos.x, pos.y, proxy.get_local_id());
			});
			world.template for_matching<SMovement>([&world](auto &proxy1, auto &pos,
				auto &vec) {
				auto tox = pos.x + vec.x;
				auto toy = pos.y + vec.y;
				if (proxy1.template has_component<CCollision>()) {
					//���·���ϵ���ײ
					auto nowx = pos.x, nowy = pos.y;
					int bx = bounce(tox, width), by = bounce(toy, height);
					auto &vec = position_trace(world, bx, by);
					for (auto &e2 : vec) {
						world.for_local(e2, [&](auto &proxy2) {
							if (proxy1.template get_component<CCollision>().response == Block &&
								proxy2.template get_component<CCollision>().response == Block) {
								pos.x = nowx;
								pos.y = nowy;
								world.broadcast(CollisionEvent{ Block, proxy1.get_local_id(),
									proxy2.get_local_id() });
								return;
							}
							else
								world.broadcast(CollisionEvent{ Overlap, proxy1.get_local_id(),
									proxy2.get_local_id() });
						});
					}
				}
				pos.x = bounce(tox, width);
				pos.y = bounce(toy, height);
			});
		}

	public:
		void receive(const TickEvent &event) { Tick(world); }
		PhysicalMovementSystem() {
			world.template subscribe<TickEvent>(*this);
			new (&world.template get_singleton<SceneData>()) SceneData(width, height);
		}
	};
	
	template <typename World> class LifeSystem : System<World> {
		using SLifeTime = eecs::Signature<CLifeTime>;
		inline static void Tick(World &world) {
			world.template for_matching<SLifeTime>([](auto &proxy, auto &life) {
				if (life.frames == 0)
					proxy.kill();
				life.frames--;
			});
		}

	public:
		void receive(const TickEvent &event) { Tick(world); }
		LifeSystem() { world.template subscribe<TickEvent>(*this); }
	};

} // namespace Systems