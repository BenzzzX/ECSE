// Gluttonous Snake.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <thread>

//#include "benchmark.h"
#include "Data.h" 
#include "GeneralCode.h"

static constexpr auto height = 14, width = 24;

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

/*系统*/
namespace Systems
{
	/*控制系统，根据玩家输入更新速度*/
	class ControllSystem : System<ControllSystem, World>
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
	class SnakeSystem : System<SnakeSystem, World>
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


	/*食物系统，生成食物*/
	class FoodSystem : System<FoodSystem, World>
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
#define InstallGeneral(name) Systems::name<World> ___##name

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
	InstallGeneral(PhysicalMovementSystem);	InstallGeneral(LifeSystem);
	Install(FoodSystem);				
	InstallGeneral(CacheSystem);
	InstallGeneral(SerializeSystem);
	/*交互*/
	{
		InstallGeneral(RenderSystem); //渲染
		InstallGeneral(InputSystem); //转发系统输入
		InstallGeneral(TickSystem);  //帧更新
	}
}

void serialize_test()
{
	World world;
	BYTE data[100];
	BYTE *pdata = data;
	Systems::SerializeSystem<World> _;
	auto id = world.new_entity();
	world.for_local(id, [&](auto& proxy)
	{
		proxy.add_component<CAppearance>('a'); //初始化
		std::cout << proxy.get_component<CAppearance>().v << "\n";
		Util::serialize(world, proxy, pdata);
		proxy.get_component<CAppearance>().v = 'b';  //改变值
		std::cout << proxy.get_component<CAppearance>().v << "\n";
		pdata = data;
		ImpureUtil::unserialize(world, pdata); //反序列化
		std::cout << proxy.get_component<CAppearance>().v << "\n";
	});
}

int main()
{
	runSnake();
	//benchmark::runCompare(100000, 10000, 1);
	//serialize_test();
	return 0;
}
