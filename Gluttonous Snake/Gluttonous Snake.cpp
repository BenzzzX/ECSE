// Gluttonous Snake.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <thread>

#include "benchmark.h"
#include "Data.h" 

static constexpr auto height = 14, width = 24;

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

}

namespace ImpureUtil
{
	/*构造一个玩家蛇实体*/
	void construct_snake(World& world)
	{
		/*construct an entity*/
		auto id = world.new_entity();
		world.for_local(id, [](auto& character)
		{
			character.instantiate(snakeTemplate);
		});
	}

	/*构造一个食物实体*/
	auto construct_food(World& world, CPosition pos)
	{
		auto id = world.new_entity();
		world.for_local(id, [&pos](auto& food)
		{
			food.instantiate(foodTemplate).get_component<CPosition>() = pos;
		});
	}

	/*构造一个食物生成器*/
	auto construct_foodspawner(World& world)
	{
		auto id = world.new_entity();
		world.for_local(id, [](auto& foodspawner)
		{
			foodspawner.add_component<CFoodSpawner>(50u, 10u, (size_t)time(0));
		});
	}
}

class ScreenBuffer
{
	std::vector<char> deferred;
	std::vector<char> content;
	int access(int x, int y) { return (height - y - 1)*width + x; }
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
		int pos = access(x, y);
		if (x < width && y < height && deferred[pos] != content[pos])
		{
			v = deferred[pos];
			return true;
		}
		return false;
	}
};
ScreenBuffer screen; //定义全局单例，注意这里和世界单例的不同


/*系统*/
namespace Systems
{
	template<typename T>
	using System = World::System<T>;

	/*渲染系统，收集渲染信息并渲染*/
	class RenderSystem : System<RenderSystem>
	{
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

	/*移动系统,负责移动*/
	class MovementSystem : System<MovementSystem>
	{
		using SMovement = eecs::Signature<CPosition, CVelocity>;
		static int bounce(int x, size_t limit)
		{
			while (x < 0) x += limit;
			return x%limit;
		}

		inline static void Tick(World& world)
		{
			world.for_matching<SMovement>([&world](auto&, auto& pos, auto& vec)
			{
				auto tox = pos.x + vec.x;
				auto toy = pos.y + vec.y;
				pos.x = bounce(tox, width); pos.y = bounce(toy, height);
			});
		}
	public:

		void receive(const TickEvent& event)
		{
			Tick(world);
		}

		MovementSystem()
		{
			world.subscribe<TickEvent>(*this);

		}
	};

	/*物理移动系统，带碰撞的移动*/
	class PhysicalMovementSystem : System<PhysicalMovementSystem>
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
			while (x < 0) x += limit;
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
	class InputSystem : System<InputSystem>
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

	std::thread InputSystem::thread;

	/*帧系统，负责发送帧更新事件*/
	class TickSystem : System<TickSystem>
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

	std::thread TickSystem::thread;

	/*控制系统，根据玩家输入更新速度*/
	class ControllSystem : System<ControllSystem>
	{
		using SPlayer = eecs::Signature<CVelocity, CInput>;
	public:
		ControllSystem()
		{
			world.subscribe<KeyEvent>(*this);
			world.subscribe<TickEvent>(*this);
		}

		void receive(const TickEvent& event)
		{
			world.for_matching<SPlayer>([this, &event](auto& proxy, auto& vec, auto& input)
			{
				switch (input.v)
				{
				case 'a':
					if (vec.x == 1) break;
					vec = { -1,0 }; break;
				case 'd':
					if (vec.x == -1) break;
					vec = { 1,0 }; break;
				case 'w':
					if (vec.y == 1) break;
					vec = { 0,-1 }; break;
				case 's':
					if (vec.y == -1) break;
					vec = { 0,1 }; break;
				default:	break;
				}
			});
		}

		void receive(const KeyEvent& event)
		{
			if (!event.isDown) return;
			world.for_matching<SPlayer>([this, &event](auto&, auto&, auto& input)
			{
				input.v = event.key;
			});
		}
	};

	/*蛇系统，负责管理蛇身，并处理蛇的碰撞事件*/
	class SnakeSystem : System<SnakeSystem>
	{
		using SSnake = eecs::Signature<CPosition, CVelocity, CSnake>;
		inline static void Tick(World& world)
		{
			world.for_matching<SSnake>([&world](auto&, auto& pos, auto&, auto& snake)
			{
				size_t id = world.new_entity();
				world.for_local(id, [&pos, &snake](auto& proxy)
				{
					proxy.add_component<CPosition>(pos.x, pos.y)
					.add_component<CAppearance>('*')
					.add_component<CLifeTime>(snake.length - 2)
					.add_component<CCollision>(Block)
					.add_tag<TSnakeBody>();
				});
			});
		}
	public:
		void receive(const TickEvent& event)
		{
			Tick(world);
		}

		void receive(const CollisionEvent& event)
		{
			world.for_local(event.id1, [&](auto& proxy1)
			{
				world.for_local(event.id2 , [&](auto& proxy2)
				{
					if (proxy1.has_component<CSnake>())
					{
						auto& snake = proxy1.get_component<CSnake>();
						if (proxy2.has_tag<TFood>())
						{
							snake.length += 1;
							proxy2.kill();
						}
						if (event.response == Block)
						{
							snake.length = max(2, snake.length - 1);
						}
					}
				});
			});
		}

		SnakeSystem()
		{
			world.subscribe<TickEvent>(*this);
			world.subscribe<CollisionEvent>(*this);
		}
	};

	/*寿命系统，负责计时销毁实体*/
	class LifeSystem : System<LifeSystem>
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

	/*快照系统，创建世界快照*/
	class CacheSystem : System<CacheSystem>
	{	
		static void cache_world(World& world)
		{
			auto& worldcache = world.getSingleton<Cache>();
			worldcache.cache.push();
			auto& cache = worldcache.cache.back().second;
			size_t &size = worldcache.cache.back().first;
			size = 0;
			world.for_all([&](auto& proxy)
			{
				size += proxy.get_size();
			});
			if (size > cache.size()) cache.resize(size);
			BYTE* pdata = cache.data();
			world.for_all([&](auto& proxy)
			{
				proxy.save(pdata);
			});
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
				world.for_local(id, [&pdata](auto& proxy)
				{
					proxy.restore(pdata);
				});
			}
			world.apply_changes();
			return true;
		}
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
			if(restore_world(world)) event.frame--;
		}
	};

	/*食物系统，生成食物*/
	class FoodSystem : System<FoodSystem>
	{
		using SFoodSpawner = eecs::Signature<CFoodSpawner>;
		static void generate_food(World& world,size_t rand)
		{
			std::vector<CPosition> available;
			auto& scene = world.getSingleton<SceneData>();
			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++)
				{
					if (scene.get(x, y).empty()) available.push_back({ x,y });
				}
			/*todo: initial with a rand seed*/
			const size_t randIndex = rand % available.size();
			ImpureUtil::construct_food(world, available[randIndex]);
		}
	public:
		void receive(const TickEvent& event)
		{
			world.for_matching<SFoodSpawner>([this](auto&, auto& spawner) {
				if (spawner.remain == 0)
				{
					spawner.remain = spawner.interval;
					generate_food(world,Util::randnext(spawner.holdrand));
				}
				spawner.remain--;
			});
		}

		FoodSystem()
		{
			world.subscribe<TickEvent>(*this);
		}
	};
}


/*构造匿名系统*/
#define Install(name) Systems::name ___##name
#define Open(name) pWorld = &name;

void runSnake()
{
	/*安装世界*/
	World world;
	/*构造角色*/
	ImpureUtil::construct_snake(world);
	/*构造食物生成器*/
	ImpureUtil::construct_foodspawner(world);
	
	world.apply_changes();

	/*安装系统*/
	/*游戏*/
	Install(ControllSystem);			Install(SnakeSystem);
	Install(PhysicalMovementSystem);	Install(LifeSystem);
	Install(FoodSystem);				Install(CacheSystem);
	/*交互*/
	{
		Install(RenderSystem); //渲染
		Install(InputSystem); //转发系统输入
		Install(TickSystem);  //帧更新
	}
}
int main()
{
	//runSnake();
	//benchmark::runCompare(100000, 10000, 1);
	using World = eecs::World<
		eecs::ComponentList<int, float>,
		eecs::TagList<struct tag>,
		eecs::EventList<int>,
		eecs::SingletonList<>
	>;
	World world;
	BYTE data[100];
	BYTE *pdata = data;
	auto id = world.new_entity();
	world.for_local(id, [&](auto& proxy)
	{
		proxy.add_component<int>(1); //初始化
		std::cout << proxy.get_component<int>() << "\n";
		proxy.serialize(pdata); //序列化
		proxy.get_component<int>() = 2;  //改变值
		std::cout << proxy.get_component<int>() << "\n";
		pdata = data;
		world.unserialize(pdata); //反序列化
		std::cout << proxy.get_component<int>() << "\n";
	});


	return 0;
}
