#pragma once
#include "ECS\eecs.hpp"
#include <iostream>
#include <thread>
/*benchmark 性能测试，比较对象:EntityPlus*/
#include "EntityPlus\entity.h"
namespace benchmark
{
	class Timer {
		::std::chrono::high_resolution_clock::time_point start;
		const char *str;
	public:
		Timer(const char *str) : start(::std::chrono::high_resolution_clock::now()),
			str(str) {}
		~Timer() {
			auto end = ::std::chrono::high_resolution_clock::now();
			::std::cout << str << " :time used: " << ::std::chrono::duration_cast<::std::chrono::milliseconds>(end - start).count() << "\n";
		}
	};

	void runTest(int entityCount, int iterationCount, int tagProb) {
		using World = eecs::World<
			eecs::ComponentList<int, float>,
			eecs::TagList<struct tag>,
			eecs::EventList<int>,
			eecs::SingletonList<>
		>;//定义世界
		World world;//安装世界
		using SomeSignature = eecs::Signature<tag, int>;//定义标识
		::std::cout << "eecs\n";
		{
			Timer timer("Add entities: ");
			for (int i = 0; i < entityCount; ++i) {
				auto id = world.new_entity();
				world.for_local(id, [&i, &tagProb](auto& proxy)//创建实体
				{
					proxy.add_component<int>(1);
					if (i % tagProb == 0)
						proxy.add_tag<tag>();//添加标记
				});
			}

			/*实体的创建和更新被延后到了apply_changes*/
			world.apply_changes();
		}
		{
			Timer timer("For_each entities: ");
			::std::uint64_t sum = 0;
			for (int i = 0; i < iterationCount; ++i) {
				world.for_matching<SomeSignature>([&sum](auto& proxy, auto i) //根据标识遍历实体
				{
					sum += i;
				});
			}
			::std::cout << sum << "\n";
		}
	}

	void runEntityPlusTest(int entityCount, int iterationCount, int tagProb) {
		using namespace entityplus;
		entity_manager<component_list<int, float>, tag_list<struct Tag>> em;
		//em.create_grouping<int, Tag>();
		::std::cout << "EntityPlus\n";
		{
			Timer timer("Add entities: ");
			for (int i = 0; i < entityCount; ++i) {
				auto ent = em.create_entity();
				ent.add_component<int>(1);
				if (i % tagProb == 0)
					ent.set_tag<Tag>(true);
			}
		}
		{
			Timer timer("For_each entities: ");
			::std::uint64_t sum = 0;
			for (int i = 0; i < iterationCount; ++i) {
				em.for_each<Tag, int>([&](auto ent, auto i) {
					sum += i;
				});
			}
			::std::cout << sum << "\n";
		}
	}

	void runCompare(int entityCount, int iterationCount, int tagProb) {
		runTest(entityCount, iterationCount, tagProb);
		runEntityPlusTest(entityCount, iterationCount, tagProb);
	}

}