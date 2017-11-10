#pragma once
#include "GeneralData.h"
#include "to_tuple.h"
namespace Util {

	bool is_key_down(char key) {
		return static_cast<bool>(GetKeyState(key) & (1 << 8));
	}

	size_t randnext(size_t &holdrand) {
		return (holdrand = ((holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
	}

	template <typename C, typename T, typename E, typename S>
	void save(eecs::World<C, T, E, S> &world,
		typename eecs::World<C, T, E, S>::EntityProxy &proxy, BYTE *&data) {
		using World = eecs::World<C, T, E, S>;
		using Config = typename World::Config;
		const auto &bitset = proxy.get_bitset();
		memcpy(data, &bitset, sizeof(typename Config::Bitset));
		data += sizeof(typename Config::Bitset);
		eecs::MPL::forTypes<C>([&](auto v) {
			using t = typename decltype(v)::type;
			constexpr auto id = Config::template metaBit<t>();
			if (bitset[id]) {
				const t &c = proxy.template get_component<t>();
				memcpy(data, &c, sizeof(t));
				data += sizeof(t);
			}
		});
	}



	template <typename C, typename T, typename E, typename S, typename V>
	auto wrap_ref(eecs::World<C, T, E, S> &world, V& v)
	{
		static_assert(eecs::MPL::Contains<UniqueData, S>{},
			"translate_id only enable when UniqueMap is available");

		using namespace eecs::MPL;

		using membertypelist = to_member_list_t<V>;
		using targettypelist = typename replace_all<EntityID, UniqueID, membertypelist>::type;
		using targettype = Rename<std::tuple, targettypelist>;

		targettype result;

		using tupletype = to_tuple_t<V>;
		const auto& astuple = to_tuple(v);

		forTypes<typename to_index<membertypelist>::type>([&](auto v)
		{
			using t = typename decltype(v)::type;
			constexpr auto index = t::value;
			using type = typename Nth<index, membertypelist>;
			if constexpr(std::is_same<type, EntityID>::value)
			{
				const auto id = std::get<index>(astuple);
				world.for_local(id, [&](auto& proxy)
				{
					assert(proxy.has_component<CUniqueID>());
					const auto& uid = proxy.get_component<CUniqueID>();
					std::get<index>(result) = uid.uid;
				});
			}
			else
			{
				std::get<index>(result) = std::get<index>(astuple);
			}
		});
		return result;
	}

	template <typename C, typename T, typename E, typename S>
	void serialize(eecs::World<C, T, E, S> &world,
		typename eecs::World<C, T, E, S>::EntityProxy &proxy,
		BYTE *&data) {
		static_assert(eecs::MPL::Contains<CUniqueID, C>{},
			"serialize only enable when CUniqueID is available");
		static_assert(eecs::MPL::Contains<UniqueData, S>{},
			"serialize only enable when UniqueMap is available");
		auto &uM = world.template get_singleton<UniqueData>();
		assert(uM.enabled);
		auto &uniqueMap = uM.map;
		if (!proxy.template has_component<CUniqueID>()) {
			GUID newID;
			CoCreateGuid(&newID);
			//std::cout << "creating GUID for entity:" << proxy.get_local_id() << '\n';
			proxy.template add_component<CUniqueID>(newID);
			uniqueMap.insert(std::make_pair(newID, proxy.get_local_id()));
		}
		using World = eecs::World<C, T, E, S>;
		using Config = typename World::Config;
		using Bitset = typename Config::Bitset;

		const auto &uid = proxy.template get_component<CUniqueID>();
		memcpy(data, &uid, sizeof(uid));
		data += sizeof(uid);
		const auto &bitset = proxy.get_bitset();
		memcpy(data, &bitset, sizeof(Bitset));
		data += sizeof(Bitset);
		Bitset dirty,cacheBitset;
		BYTE* cacheData;

		bool creatingCache = false;
		if (uM.cache.find(uid.uid) == uM.cache.end())
		{
			creatingCache = true;
			uM.cache.insert(std::make_pair(uid.uid, std::vector<BYTE>{}));
		}
		else
		{
			auto& cache = uM.cache[uid.uid];
			cacheData = cache.data();
			memcpy(&cacheBitset, cacheData, sizeof(Bitset));
			cacheData += sizeof(Bitset);
		}
		size_t cacheNeedSize = sizeof(Bitset);

		eecs::MPL::forTypes<C>([&](auto v) {
			using t = typename decltype(v)::type;
			constexpr auto id = Config::template metaBit<t>();
			if (bitset[id]) {
				constexpr auto serializeStrategy = GetSerializeStrategy<t>::value;
				if constexpr (serializeStrategy == SerializeStrategy::Always) {
					dirty[id] = true;
					cacheNeedSize += sizeof(t);
				}
				else if constexpr (serializeStrategy == SerializeStrategy::Changing)
				{
					cacheNeedSize += sizeof(t);
					if (creatingCache)
					{
						dirty[id] = true;
					}
					else if(cacheBitset[id])
					{
						t &c = proxy.template get_component<t>();
						bool changed = memcmp(&c, cacheData, sizeof(t)) == 0;
						if (changed)
						{
							//std::cout << "serialize on changing\n";
							dirty[id] = true;
						}
					}
					else
					{
						dirty[id] = true;
					}
				}
			}
			if (!creatingCache && cacheBitset[id])
				cacheData += sizeof(t);
		});
		std::vector<BYTE>& cache = uM.cache[uid.uid];
		if (cache.capacity()* sizeof(BYTE) < cacheNeedSize) cache.resize(cacheNeedSize/sizeof(BYTE) + 1);
		cacheData = cache.data();

		memcpy(data, &dirty, sizeof(Bitset));
		data += sizeof(Bitset);
		memcpy(cacheData, &bitset, sizeof(Bitset));
		cacheData += sizeof(Bitset);

		eecs::MPL::forTypes<C>([&](auto v) {
			using t = typename decltype(v)::type;
			constexpr auto id = Config::template metaBit<t>();
			if (dirty[id]) {
				t &c = proxy.template get_component<t>();
				auto wrappedc = wrap_ref(world, c);
				memcpy(data, &wrappedc, sizeof(wrappedc));
				data += sizeof(wrappedc);
			}
			constexpr auto serializeStrategy = GetSerializeStrategy<t>::value;
			if constexpr (
				serializeStrategy == SerializeStrategy::Always
				|| serializeStrategy == SerializeStrategy::Changing)
			{
				if (bitset[id])
				{
					t &c = proxy.template get_component<t>();
					memcpy(cacheData, &c, sizeof(t));
					cacheData += sizeof(t);
				}
			}
		});
	}

} // namespace Util

namespace ImpureUtil {
	template <typename C, typename T, typename E, typename S>
	void restore(eecs::World<C, T, E, S> &world,
		typename eecs::World<C, T, E, S>::EntityProxy &proxy,
		BYTE *&data) {
		using World = eecs::World<C, T, E, S>;
		using Config = typename World::Config;
		using Bitset = typename Config::Bitset;
		Bitset bitset;
		memcpy(&bitset, data, sizeof(Bitset));
		proxy.set_bitset(bitset);
		data += sizeof(Bitset);
		eecs::MPL::forTypes<C>([&](auto v) {
			using t = typename decltype(v)::type;
			constexpr auto id = Config::template metaBit<t>();
			if (bitset[id]) {
				t c;
				memcpy(&c, data, sizeof(t));
				data += sizeof(t);
				proxy.template add_component<t>(c);
			}
		});
	}

	template <typename C, typename T, typename E, typename S, typename ...V>
	auto unwrap_ref(eecs::World<C, T, E, S> &world, const std::tuple<V...>& v)
	{
		static_assert(eecs::MPL::Contains<UniqueData, S>{},
			"translate_id only enable when UniqueMap is available");
		auto &uM = world.template get_singleton<UniqueData>();
		assert(uM.enabled);
		auto &uniqueMap = uM.map;

		using namespace eecs::MPL;
		using membertypelist = TypeList<V...>;
		using targettypelist = typename replace_all<UniqueID, EntityID, membertypelist>::type;
		using targettype = Rename<std::tuple, targettypelist>;

		targettype result;
		const auto& astuple = v;

		forTypes<typename to_index<membertypelist>::type>([&](auto v)
		{
			using t = typename decltype(v)::type;
			constexpr auto index = t::value;
			using type = typename Nth<index, membertypelist>;
			if constexpr(std::is_same<type, UniqueID>::value)
			{
				const auto uid = std::get<index>(astuple);
				if (uniqueMap.find(uid) == uniqueMap.end())
				{
					size_t nid = world.new_entity();
					uniqueMap.insert(std::make_pair(uid, nid));
					world.for_local(nid, [&](auto& proxy)
					{
						proxy.add_component<CUniqueID>(uid);
					});
				}
				else
				{
					std::get<index>(result) = uniqueMap[uid];
				}
			}
			else
			{
				std::get<index>(result) = std::get<index>(astuple);
			}
		});
		return result;
	}

	template <typename C, typename T, typename E, typename S>
	void unserialize(eecs::World<C, T, E, S> &world, BYTE *&data) {
		static_assert(eecs::MPL::Contains<CUniqueID, C>{},
			"serialize only enable when CUniqueID is available");
		static_assert(eecs::MPL::Contains<UniqueData, S>{},
			"serialize only enable when UniqueMap is available");
		auto &uM = world.template get_singleton<UniqueData>();
		assert(uM.enabled);
		auto &uniqueMap = uM.map;
		CUniqueID uid;
		memcpy(&uid, data, sizeof(uid));
		data += sizeof(uid);
		if (uniqueMap.find(uid.uid) == uniqueMap.end()) {
			size_t nid = world.new_entity();
			uniqueMap.insert(std::make_pair(uid.uid, nid));
			world.for_local(nid, [&](auto& proxy)
			{
				proxy.add_component<CUniqueID>(uid);
			});
		}
		size_t id = uniqueMap[uid.uid];
		world.for_local(id, [&](auto &proxy) {
			using World = eecs::World<C, T, E, S>;
			using Config = typename World::Config;
			using Bitset = typename Config::Bitset;
			proxy.template add_component<CUniqueID>(uid);
			Bitset bitset, dirty;
			memcpy(&bitset, data, sizeof(Bitset));
			data += sizeof(Bitset);
			proxy.set_bitset(bitset);
			memcpy(&dirty, data, sizeof(Bitset));
			data += sizeof(Bitset);
			eecs::MPL::forTypes<C>([&](auto v) {
				using t = typename decltype(v)::type;
				constexpr auto id = Config::template metaBit<t>();
				if (dirty[id]) {
					t c;
					using wraped_type = decltype(Util::wrap_ref(world, c));
					wraped_type wrappedc;
					memcpy(&wrappedc, data, sizeof(wrappedc));
					data += sizeof(wrappedc);
					c = eecs::MPL::to_struct<t>(unwrap_ref(world, std::move(wrappedc)));
					proxy.template add_component<t>(c);
				}
			});
		});
	}
} // namespace ImpureUtil

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

	template <typename World> class InputSystem : System<World> {
		static std::thread thread;

	public:
		InputSystem() {
			thread = std::thread([this]() {
				HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
				INPUT_RECORD records[128];
				DWORD nReads;
				while (1) {
					ReadConsoleInput(handle, records, 128, &nReads);
					for (size_t i = 0; i < nReads; i++) {
						if (records[i].EventType == KEY_EVENT) {
							auto Event = records[i].Event.KeyEvent;
							world.broadcast(
								KeyEvent{ Event.uChar.AsciiChar, (bool)Event.bKeyDown });
						}
					}
				}
			});
		}
		~InputSystem() { thread.join(); }
	};

	template <typename World> std::thread InputSystem<World>::thread;

	template <typename World> class TickSystem : System<World> {
		static constexpr size_t framerate = 10u;
		static std::thread thread;

	public:
		TickSystem() {

			thread = std::thread([this]() {
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

	template <typename World> std::thread TickSystem<World>::thread;


	template <typename World> class CacheSystem : System<World> {

		static void cache_world(World &world) {
			auto &worldcache = world.template get_singleton<Cache>();
			worldcache.cache.push();
			auto &cache = worldcache.cache.back().second;
			size_t &size = worldcache.cache.back().first;

			static BYTE buffer[10000];

			BYTE *pdata = buffer;
			world.for_all([&](auto &proxy) { Util::save(world, proxy, pdata); });
			size = (pdata - buffer) * sizeof(BYTE);
			if (size > cache.size())
				cache.resize(size);
			memcpy(cache.data(), buffer, size);
		}

		static bool restore_world(World &world) {
			world.template get_singleton<SceneData>().clr();
			auto &worldcache = world.template get_singleton<Cache>();
			BYTE *pdata, *pbegin;
			if (worldcache.cache.empty())
				return false;
			;
			auto &cache = worldcache.cache.back().second;
			const auto size = worldcache.cache.back().first;
			worldcache.cache.pop();
			pdata = pbegin = cache.data();
			world.kill_all();
			while (pdata < (pbegin + size)) {
				auto id = world.new_entity();
				world.for_local(
					id, [&](auto &proxy) { ImpureUtil::restore(world, proxy, pdata); });
			}
			world.apply_changes();
			return true;
		}
	public:
		CacheSystem() {
			world.template subscribe<RewindEvent>(*this);
			world.template subscribe<CacheEvent>(*this);
		}

		void receive(const CacheEvent &event) { cache_world(world); }

		void receive(const RewindEvent &event) {
			if (restore_world(world))
				event.frame--;
		}
	};

	template <typename World> class SerializeSystem : public System<World> {
	public:
		void receive(const eecs::EntityDyingEvent &event) {
			auto &uM = world.template get_singleton<UniqueData>();
			assert(uM.enabled);
			auto &uniqueMap = uM.map;
			world.for_local(event.id, [&uniqueMap](auto &proxy) {
				if (proxy.template has_component<CUniqueID>()) {
					auto &uid = proxy.template get_component<CUniqueID>();
					uniqueMap.erase(uid.uid);
				}
			});
		}

		SerializeSystem() {
			auto &uM = world.template get_singleton<UniqueData>();
			assert(uM.enabled = true);
			world.template subscribe<typename eecs::EntityDyingEvent>(*this);
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