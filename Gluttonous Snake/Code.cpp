#include <Windows.h>
#include <iostream>
#include <thread>
#undef max
#include "benchmark.h"
#include "Data.h"

static constexpr auto height = 14, width = 24;

namespace ImpureUtil {
	/*构造一个玩家蛇实体*/
	void construct_snake(World &world) {
		/*construct an entity*/
		auto id = world.new_entity();
		world.for_local(
			id, [](auto &character) { character.instantiate(snakeTemplate); });
	}

	/*构造一个食物实体*/
	auto construct_food(World &world, CPosition pos) {
		auto id = world.new_entity();
		world.for_local(id, [&pos](auto &food) {
			food.instantiate(foodTemplate).template get_component<CPosition>() = pos;
		});
	}

	/*构造一个食物生成器*/
	auto construct_foodspawner(World &world) {
		auto id = world.new_entity();
		world.for_local(id, [](auto &foodspawner) {
			foodspawner.template add_component<CFoodSpawner>(50u, 10u, (size_t)time(0));
		});
	}
} // namespace ImpureUtil

  /*系统*/
namespace Systems {
	/*控制系统，根据玩家输入更新速度*/
	class ControllSystem {
		using SPlayer = EEC::Signature<CVelocity, CInput>;
		World& world;
	public:
		ControllSystem(World& world):world(world) {
			world.subscribe<KeyEvent>(*this);
			world.subscribe<TickEvent>(*this);
		}

		void receive(const TickEvent &event) {
			world.for_matching<SPlayer>([](auto &proxy, auto &vec, auto &input) {
				switch (input.v) {
				case 'a':
					if (vec.x == 1)
						break;
					vec = { -1, 0 };
					break;
				case 'd':
					if (vec.x == -1)
						break;
					vec = { 1, 0 };
					break;
				case 'w':
					if (vec.y == 1)
						break;
					vec = { 0, -1 };
					break;
				case 's':
					if (vec.y == -1)
						break;
					vec = { 0, 1 };
					break;
				default:
					break;
				}
			});
		}

		void receive(const KeyEvent &event) {
			if (!event.isDown)
				return;
			world.for_matching<SPlayer>(
				[&event](auto &, auto &, auto &input) { input.v = event.key; });
		}
	};

	/*蛇系统，负责管理蛇身，并处理蛇的碰撞事件*/
	class SnakeSystem {
		using SSnake = EEC::Signature<CPosition, CVelocity, CSnake>;
		inline static void Tick(World &world) {
			world.for_matching<SSnake>(
				[&world](auto &, auto &pos, auto &, auto &snake) {
				size_t id = world.new_entity();
				world.for_local(id, [&pos, &snake](auto &proxy) {
					proxy.template add_component<CPosition>(pos.x, pos.y)
						.template add_component<CAppearance>('*')
						.template add_component<CLifeTime>(snake.length - 2)
						.template add_component<CCollision>(Block)
						.template add_tag<TSnakeBody>();
				});
			});
		}
		World& world;
	public:
		void receive(const TickEvent &event) { Tick(world); }

		void receive(const CollisionEvent &event) {
			world.for_local(event.id1, [&](auto &proxy1) {
				world.for_local(event.id2, [&](auto &proxy2) {
					if (proxy1.template has_component<CSnake>()) {
						auto &snake = proxy1.template get_component<CSnake>();
						if (proxy2.template has_tag<TFood>()) {
							snake.length += 1;
							proxy2.kill();
						}
						if (event.response == Block) {
							snake.length = std::max<size_t>(2u, snake.length - 1);
						}
					}
				});
			});
		}

		SnakeSystem(World& world):world(world) {
			world.subscribe<TickEvent>(*this);
			world.subscribe<CollisionEvent>(*this);
		}
	};

	/*食物系统，生成食物*/
	class FoodSystem {
		using SFoodSpawner = EEC::Signature<CFoodSpawner>;
		static void generate_food(World &world, size_t rand) {
			std::vector<CPosition> available;
			auto &scene = world.get_singleton<SceneData>();
			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++) {
					if (scene.get(x, y).empty())
						available.push_back({ x, y });
				}
			/*todo: initial with a rand seed*/
			const size_t randIndex = rand % available.size();
			ImpureUtil::construct_food(world, available[randIndex]);
		}
		World& world;
	public:
		void receive(const TickEvent &event) {
			world.for_matching<SFoodSpawner>([this](auto &, auto &spawner) {
				if (spawner.remain == 0) {
					spawner.remain = spawner.interval;
					generate_food(world, Util::randnext(spawner.holdrand));
				}
				spawner.remain--;
			});
		}

		FoodSystem(World& world):world(world) { world.subscribe<TickEvent>(*this); }
	};

} // namespace Systems

  /*构造匿名系统*/
#define Install(name) name ___##name { world }
#define InstallGeneral(name) name<World> ___##name { world }

void runSnake() {

	World world;

	ImpureUtil::construct_snake(world);

	ImpureUtil::construct_foodspawner(world);

	world.apply_changes();

	using namespace Systems;
	Install(ControllSystem);
	Install(SnakeSystem);
	InstallGeneral(PhysicalMovementSystem);
	InstallGeneral(LifeSystem);
	Install(FoodSystem);
	InstallGeneral(CacheSystem);
	InstallGeneral(RenderSystem);
	Install(InputSystem);
	Install(TickSystem);
}

struct test
{
	EntityID id;
};

void serialize_test() {
	World world;
	BYTE data[100];
	BYTE *pdata = data;
	Systems::SerializeSystem<World> _{ world };
	auto id = world.new_entity();
	id = world.new_entity();
	for(int i=0;i<2;i++)
	world.for_local(id, [&](auto &proxy) {
		test ref;
		ref.id = id;
		proxy.template add_component<CAppearance>('a'); //初始化
		std::cout << "local reference to entity:" << id << '\n';
		std::cout << "local entity (CAppearance) :" << proxy.template get_component<CAppearance>().v
			<< "\n";
		Util::serialize(world, proxy, pdata);
		OLECHAR* guidString;
		auto wrappedref = Util::wrap_ref(world, ref);
		StringFromCLSID(std::get<0>(wrappedref), &guidString);
		proxy.template get_component<CAppearance>().v = 'b'; //改变本地值
		std::cout << "change local entity (CAppearance) :"
			<< proxy.template get_component<CAppearance>().v << "\n";
		pdata = data;
		ImpureUtil::unserialize(world, pdata); //反序列化
		std::cout << "unserialize:\nlocal entity (CAppearance) :"
			<< proxy.template get_component<CAppearance>().v << "\n";
		auto unwrappedref = ImpureUtil::unwrap_ref(world, wrappedref);
		std::cout << "unserialized reference to entity :" << std::get<0>(unwrappedref) << '\n';
	});
}


int main() {
    runSnake();
	//benchmark::runCompare(100000, 1000000, 1000);
    //serialize_test();
	//to_tuple_test();
	using namespace EEC::MPL;
	test b;
	auto a = to_tuple(b);
	return 0;
}
