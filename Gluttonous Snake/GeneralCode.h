#pragma once
#include "GeneralData.h"


/*工具函数*/
namespace Util
{

	/*按键查询*/
	bool is_key_down(char key)
	{
		return static_cast<bool>(GetKeyState(key)&(1 << 8));
	}

	size_t randnext(size_t& holdrand)
	{
		return (holdrand = ((holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
	}

	template<typename C, typename T, typename E, typename S>
	void save(eecs::World<C, T, E, S>& world, typename eecs::World<C, T, E, S>::EntityProxy& proxy, BYTE* &data)
	{
		using World = eecs::World<C, T, E, S>;
		using Config = typename World::Config;
		const auto& bitset = proxy.get_bitset();
		memcpy(data, &bitset, sizeof(typename Config::Bitset));
		data += sizeof(typename Config::Bitset);
		eecs::MPL::forTypes<C>([&](auto v)
		{
			using t = typename decltype(v)::type;
			constexpr auto id = Config::metaBit<t>();
			if (bitset[id])
			{
				const t& c = proxy.get_component<t>();
				memcpy(data, &c, sizeof(t));
				data += sizeof(t);
			}
		});
	}

	template<typename C, typename T, typename E, typename S>
	void serialize(eecs::World<C, T, E, S>& world, typename eecs::World<C, T, E, S>::EntityProxy& proxy, BYTE* &data)
	{
		static_assert(eecs::MPL::Contains<CUniqueID, C>{}, "serialize only enable when CUniqueID is available");
		static_assert(eecs::MPL::Contains<UniqueMap, S>{}, "serialize only enable when UniqueMap is available");
		auto& uM = world.getSingleton<UniqueMap>();
		assert(uM.enabled);
		auto& uniqueMap = uM.map;
		if (!proxy.has_component<CUniqueID>())
		{
			GUID newID;
			CoCreateGuid(&newID);
			proxy.add_component<CUniqueID>(newID);
			uniqueMap.insert(std::make_pair(newID, proxy.get_local_id()));
		}
		save(world, proxy, data);
	}

}

namespace ImpureUtil
{
	template<typename C, typename T, typename E, typename S>
	void restore(eecs::World<C, T, E, S>& world, typename eecs::World<C, T, E, S>::EntityProxy& proxy, BYTE* &data)
	{
		using World = eecs::World<C, T, E, S>;
		using Config = typename World::Config;
		using Bitset = typename Config::Bitset;
		Bitset bitset;
		memcpy(&bitset, data, sizeof(Bitset));
		data += sizeof(Bitset);
		eecs::MPL::forTypes<C>([&](auto v)
		{
			using t = typename decltype(v)::type;
			constexpr auto id = Config::metaBit<t>();
			if (bitset[id])
			{
				t c;
				memcpy(&c, data, sizeof(t));
				data += sizeof(t);
				proxy.add_component<t>(c);
			}
		});
	}

	template<typename C, typename T, typename E, typename S>
	void unserialize(eecs::World<C, T, E, S>& world, BYTE* &data)
	{
		static_assert(eecs::MPL::Contains<CUniqueID, C>{}, "serialize only enable when CUniqueID is available");
		static_assert(eecs::MPL::Contains<UniqueMap, S>{}, "serialize only enable when UniqueMap is available");
		auto& uM = world.getSingleton<UniqueMap>();
		assert(uM.enabled);
		auto& uniqueMap = uM.map;
		size_t id = world.new_entity();
		world.for_local(id, [&](auto& temp)
		{
			restore(world, temp, data);
			auto& uid = temp.get_component<CUniqueID>();
			if (uniqueMap.find(uid.uid) == uniqueMap.end())
			{
				/*should we create one if not found???*/
				uniqueMap.insert(std::make_pair(uid.uid, temp.get_local_id()));
				//temp.kill();
			}
			else
			{
				size_t id = uniqueMap[uid.uid];
				world.for_local(id, [&world, &temp](auto& proxy)
				{
					proxy.move(temp);
				});
			}
		});
	}
}


namespace Systems
{
	template<typename T, typename World>
	using System = eecs::System<T, World>;

	/*渲染系统，收集渲染信息并渲染*/
	template<typename World>
	class RenderSystem : System<RenderSystem<World>, World>
	{
		//用于渲染
		class ScreenBuffer
		{
			std::vector<char> deferred;
			std::vector<char> content;
			size_t access(size_t x, size_t y) { return (height - y - 1)*width + x; }
		public:
			bool initialized;
			size_t height, width;
			ScreenBuffer() :initialized(false) {}
			ScreenBuffer(size_t h, size_t w) :initialized(true)
			{
				height = h; width = w;
				deferred.resize(h*w);
				content.resize(h*w);
				memset(deferred.data(), ' ', deferred.size() * sizeof(char));
				memset(content.data(), ' ', content.size() * sizeof(char));
			}
			void set(size_t x, size_t y, char v)
			{
				deferred[access(x, y)] = v;
			}
			void fresh()
			{
				std::swap(deferred, content);
				memset(deferred.data(), ' ', deferred.size() * sizeof(char));
			}
			bool get(size_t x, size_t y, char& v)
			{
				size_t pos = access(x, y);
				if (x < width && y < height && deferred[pos] != content[pos])
				{
					v = deferred[pos];
					return true;
				}
				return false;
			}
		};

		static ScreenBuffer screen;

		using SRender = eecs::Signature<CPosition, CAppearance>;
		inline static void Tick(World& world)
		{
			world.for_matching<SRender>([](auto&, auto& pos, auto& ap)
			{
				screen.set(pos.x, pos.y, ap.v);
			});
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++)
				{
					char v;
					if (screen.get(x, y, v))
					{
						SetConsoleCursorPosition(hOut, { (SHORT)x * 2,(SHORT)y });
						putchar(v);
					}
				}
			screen.fresh();
		}

	public:
		void receive(const CacheEvent& event)
		{
			Tick(world);
		}

		void receive(const RewindEvent& event)
		{
			Tick(world);
		}

		RenderSystem()
		{
			world.subscribe<CacheEvent>(*this);
			world.subscribe<RewindEvent>(*this);
			new(&screen) ScreenBuffer(height, width);
			HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_CURSOR_INFO CursorInfo;
			GetConsoleCursorInfo(handle, &CursorInfo);
			CursorInfo.bVisible = false;
			SetConsoleCursorInfo(handle, &CursorInfo);
		}
	};

	template<typename World>
	typename RenderSystem<World>::ScreenBuffer RenderSystem<World>::screen;

	/*物理移动系统，带碰撞的移动*/
	template<typename World>
	class PhysicalMovementSystem : System<PhysicalMovementSystem<World>, World>
	{
		using SMovement = eecs::Signature<CPosition, CVelocity>;
		using SCollision = eecs::Signature<CPosition, CCollision>;

		/*点碰撞查询*/
		static auto& position_trace(World& world, size_t x, size_t y)
		{
			auto& scenedata = world.getSingleton<SceneData>();
			return scenedata.get(x, y);
		}

		static int bounce(int x, size_t limit)
		{
			while (x < 0) x += static_cast<int>(limit);
			return x%limit;
		}

		inline static void Tick(World& world)
		{
			SceneData &scene = world.getSingleton<SceneData>();
			scene.clr();
			world.for_matching<SCollision>([&world, &scene](auto& proxy, auto& pos, auto& cls)
			{
				scene.add(pos.x, pos.y, proxy.get_local_id());
			});
			world.for_matching<SMovement>([&world](auto& proxy1, auto& pos, auto& vec)
			{
				auto tox = pos.x + vec.x;
				auto toy = pos.y + vec.y;
				if (proxy1.has_component<CCollision>())
				{
					//检测路径上的碰撞
					auto nowx = pos.x, nowy = pos.y;
					int bx = bounce(tox, width), by = bounce(toy, height);
					auto& vec = position_trace(world, bx, by);
					for (auto& e2 : vec)
					{
						world.for_local(e2, [&](auto& proxy2)
						{
							if (proxy1.get_component<CCollision>().response == Block&&
								proxy2.get_component<CCollision>().response == Block)
							{
								pos.x = nowx;
								pos.y = nowy;
								world.broadcast(CollisionEvent{ Block,proxy1.get_local_id() ,proxy2.get_local_id() });
								return;
							}
							else world.broadcast(CollisionEvent{ Overlap,proxy1.get_local_id() ,proxy2.get_local_id() });
						});
					}
				}
				pos.x = bounce(tox, width); pos.y = bounce(toy, height);
			});
		}
	public:
		void receive(const TickEvent& event)
		{
			Tick(world);
		}
		PhysicalMovementSystem()
		{
			world.subscribe<TickEvent>(*this);
			new (&world.getSingleton<SceneData>()) SceneData(width, height);
		}
	};

	/*输入系统，接受系统输入事件并转发*/
	template<typename World>
	class InputSystem : System<InputSystem<World>, World>
	{
		static std::thread thread;
	public:
		InputSystem()
		{
			thread = std::thread([this]() {
				HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
				INPUT_RECORD records[128];
				DWORD nReads;
				while (1)
				{
					ReadConsoleInput(handle, records, 128, &nReads);
					for (size_t i = 0; i < nReads; i++)
					{
						if (records[i].EventType == KEY_EVENT)
						{
							auto Event = records[i].Event.KeyEvent;
							world.broadcast(KeyEvent{ Event.uChar.AsciiChar, (bool)Event.bKeyDown });
						}
					}
				}
			});
		}
		~InputSystem()
		{
			thread.join();
		}
	};

	template<typename World>
	std::thread InputSystem<World>::thread;

	/*帧系统，负责发送帧更新事件*/
	template<typename World>
	class TickSystem : System<TickSystem<World>, World>
	{
		static constexpr size_t framerate = 10u;
		static std::thread thread;
	public:
		TickSystem()
		{

			thread = std::thread([this]() {
				size_t frame = 0;
				while (1)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 / framerate });
					while (Util::is_key_down('P')) std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 / framerate });

					if (Util::is_key_down('U'))
					{
						world.broadcast(RewindEvent{ frame });
					}
					else
					{
						world.broadcast(CacheEvent{ frame });
						world.broadcast(TickEvent{ frame++ });
						world.apply_changes();
					}
				}
			});
		}
		~TickSystem()
		{
			thread.join();
		}
	};

	template<typename World>
	std::thread TickSystem<World>::thread;


	/*快照系统，创建世界快照*/
	template<typename World>
	class CacheSystem : System<CacheSystem<World>, World>
	{

		static void cache_world(World& world)
		{
			auto& worldcache = world.getSingleton<Cache>();
			worldcache.cache.push();
			auto& cache = worldcache.cache.back().second;
			size_t &size = worldcache.cache.back().first;

			static BYTE buffer[10000];

			BYTE* pdata = buffer;
			world.for_all([&](auto& proxy)
			{
				Util::save(world, proxy, pdata);
			});
			size = (pdata - buffer) * sizeof(BYTE);
			if (size > cache.size()) cache.resize(size);
			memcpy(cache.data(), buffer, size);
		}

		static bool restore_world(World& world)
		{
			world.getSingleton<SceneData>().clr();
			auto& worldcache = world.getSingleton<Cache>();
			BYTE* pdata, *pbegin;
			if (worldcache.cache.empty()) return false;;
			auto& cache = worldcache.cache.back().second;
			const auto size = worldcache.cache.back().first;
			worldcache.cache.pop();
			pdata = pbegin = cache.data();
			world.kill_all();
			while (pdata < (pbegin + size))
			{
				auto id = world.new_entity();
				world.for_local(id, [&](auto& proxy)
				{
					ImpureUtil::restore(world, proxy, pdata);
				});
			}
			world.apply_changes();
			return true;
		}

		/*
		static void serialize_world(World& world)
		{
		auto& worldcache = world.getSingleton<Cache>();
		worldcache.cache.push();
		auto& cache = worldcache.cache.back().second;
		size_t &size = worldcache.cache.back().first;
		static BYTE buffer[10000];
		BYTE* pdata = buffer;
		world.for_all([&](auto& proxy)
		{
		Util::serialize(world, proxy, pdata);
		});
		size = (pdata - buffer)*sizeof(BYTE);
		if (size > cache.size()) cache.resize(size);
		memcpy(cache.data(), buffer, size);
		}

		static bool unserialize_world(World& world)
		{
		world.getSingleton<SceneData>().clr();
		auto& worldcache = world.getSingleton<Cache>();
		BYTE* pdata, *pbegin;
		if (worldcache.cache.empty()) return false;;
		auto& cache = worldcache.cache.back().second;
		const auto size = worldcache.cache.back().first;
		worldcache.cache.pop();
		pdata = pbegin = cache.data();
		while (pdata < (pbegin + size))
		{
		ImpureUtil::unserialize(world, pdata);
		}
		world.apply_changes();
		return true;
		}
		*/
	public:
		CacheSystem()
		{
			world.subscribe<RewindEvent>(*this);
			world.subscribe<CacheEvent>(*this);
		}

		void receive(const CacheEvent& event)
		{
			cache_world(world);
		}

		void receive(const RewindEvent& event)
		{
			if (restore_world(world)) event.frame--;
		}
	};


	template<typename World>
	class SerializeSystem : public System<SerializeSystem<World>, World>
	{
	public:

		void receive(const eecs::EntityDyingEvent& event)
		{
			auto& uM = world.getSingleton<UniqueMap>();
			assert(uM.enabled);
			auto& uniqueMap = uM.map;
			world.for_local(event.id, [this, &uniqueMap](auto& proxy)
			{
				if (proxy.has_component<CUniqueID>())
				{
					auto& uid = proxy.get_component<CUniqueID>();
					uniqueMap.erase(uid.uid);
				}
			});
		}

		SerializeSystem()
		{
			auto& uM = world.getSingleton<UniqueMap>();
			assert(uM.enabled = true);
			world.subscribe<typename eecs::EntityDyingEvent>(*this);
		}
	};


	/*寿命系统，负责计时销毁实体*/
	template<typename World>
	class LifeSystem : System<LifeSystem<World>, World>
	{
		using SLifeTime = eecs::Signature<CLifeTime>;
		inline static void Tick(World& world)
		{
			world.for_matching<SLifeTime>([&world](auto& proxy, auto& life)
			{
				if (life.frames == 0) proxy.kill();
				life.frames--;
			});
		}
	public:
		void receive(const TickEvent& event)
		{
			Tick(world);
		}
		LifeSystem()
		{
			world.subscribe<TickEvent>(*this);
		}
	};

}