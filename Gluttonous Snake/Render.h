#pragma once
#include "ECS\eecs.hpp"


struct CAppearance {
	char v;
};

namespace Systems {
	template <typename World> using System = eecs::System<World>;

	template <typename World> class RenderSystem : System<World> {

		class ScreenBuffer {
			std::vector<char> deferred;
			std::vector<char> content;
			size_t access(size_t x, size_t y) { return (height - y - 1) * width + x; }

		public:
			bool initialized;
			size_t height, width;
			ScreenBuffer() : initialized(false) {}
			ScreenBuffer(size_t h, size_t w) : initialized(true) {
				height = h;
				width = w;
				deferred.resize(h * w);
				content.resize(h * w);
				memset(deferred.data(), ' ', deferred.size() * sizeof(char));
				memset(content.data(), ' ', content.size() * sizeof(char));
			}
			void set(size_t x, size_t y, char v) { deferred[access(x, y)] = v; }
			void fresh() {
				std::swap(deferred, content);
				memset(deferred.data(), ' ', deferred.size() * sizeof(char));
			}
			bool get(size_t x, size_t y, char &v) {
				size_t pos = access(x, y);
				if (x < width && y < height && deferred[pos] != content[pos]) {
					v = deferred[pos];
					return true;
				}
				return false;
			}
		};

		static ScreenBuffer screen;

		using SRender = eecs::Signature<CPosition, CAppearance>;
		inline static void Tick(World &world) {
			world.template for_matching<SRender>(
				[](auto &, auto &pos, auto &ap) { screen.set(pos.x, pos.y, ap.v); });
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++) {
					char v;
					if (screen.get(x, y, v)) {
						SetConsoleCursorPosition(hOut, { (SHORT)x * 2, (SHORT)y });
						putchar(v);
					}
				}
			screen.fresh();
		}

	public:
		void receive(const CacheEvent &event) { Tick(world); }

		void receive(const RewindEvent &event) { Tick(world); }

		RenderSystem() {
			world.template subscribe<CacheEvent>(*this);
			world.template subscribe<RewindEvent>(*this);
			new (&screen) ScreenBuffer(height, width);
			HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_CURSOR_INFO CursorInfo;
			GetConsoleCursorInfo(handle, &CursorInfo);
			CursorInfo.bVisible = false;
			SetConsoleCursorInfo(handle, &CursorInfo);
		}
	};

	template <typename World>
	typename RenderSystem<World>::ScreenBuffer RenderSystem<World>::screen;
}