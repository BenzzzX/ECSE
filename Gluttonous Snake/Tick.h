#pragma once
#include "ECS\EEC.hpp"
#include <thread>
#include <chrono>


struct CacheEvent {
	size_t frame = 0;
};

struct TickEvent {
	size_t frame = 0;
};

struct RewindEvent {
	size_t &frame;
};


namespace Systems {

	class TickSystem {
		static constexpr size_t framerate = 10u;
		std::thread thread;
	public:
		template<typename World>
		TickSystem(World& world) {
			thread = std::thread([&]() {
				size_t frame = 0;
				while (1) {
					std::this_thread::sleep_for(
						std::chrono::milliseconds{ 1000 / framerate });
					while (Util::is_key_down('P'))
						std::this_thread::sleep_for(
							std::chrono::milliseconds{ 1000 / framerate });

					if (Util::is_key_down('U')) {
						world.broadcast(RewindEvent{ frame });
					}
					else {
						world.broadcast(CacheEvent{ frame });
						world.broadcast(TickEvent{ frame++ });
						world.apply_changes();
					}
				}
			});
		}
		~TickSystem() { thread.join(); }
	};
}