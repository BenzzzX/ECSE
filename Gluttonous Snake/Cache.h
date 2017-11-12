#pragma once
#include "ECS\EEC.hpp"
#include "Tick.h"

class Cache {
	template <typename ET, int S> class ringbuffer {
	public:
		typedef ET value_type;

		ringbuffer() { clear(); }

		~ringbuffer() {}

		int size() const { return m_size; }
		int max_size() const { return S; }

		bool empty() const { return m_size == 0; }
		bool full() const { return m_size == S; }

		value_type &front() { return m_buffer[m_front]; }
		value_type &back() { return m_buffer[m_back]; }

		void clear() {
			m_size = 0;
			m_front = 0;
			m_back = S - 1;
		}

		void push() {
			m_back = (m_back + 1) % S;
			if (size() == S)
				m_front = (m_front + 1) % S;
			else
				m_size++;
		}

		void push(const value_type &x) {
			push();
			back() = x;
		}

		void pop() {
			if (m_size > 0) {
				m_size--;
				m_back = (m_front + m_size - 1) % S;
			}
		}

	protected:
		std::vector<value_type> m_buffer{ S };

		int m_size;

		int m_front;
		int m_back;
	};

public:
	using TCacheData = std::vector<BYTE>;
	using TFrameCache = std::pair<size_t, TCacheData>;
	ringbuffer<TFrameCache, 10000> cache;
};

namespace Util 
{
	template <typename C, typename T, typename E, typename S>
	void save(EEC::World<C, T, E, S> &world,
		typename EEC::World<C, T, E, S>::EntityProxy &proxy, BYTE *&data) {
		using World = EEC::World<C, T, E, S>;
		using Config = typename World::Config;
		const auto &bitset = proxy.get_bitset();
		memcpy(data, &bitset, sizeof(typename Config::Bitset));
		data += sizeof(typename Config::Bitset);
		EEC::MPL::forTypes<C>([&](auto v) {
			using t = typename decltype(v)::type;
			constexpr auto id = Config::template metaBit<t>();
			if (bitset[id]) {
				const t &c = proxy.template get_component<t>();
				memcpy(data, &c, sizeof(t));
				data += sizeof(t);
			}
		});
	}
}

namespace ImpureUtil
{
	template <typename C, typename T, typename E, typename S>
	void restore(EEC::World<C, T, E, S> &world,
		typename EEC::World<C, T, E, S>::EntityProxy &proxy,
		BYTE *&data) {
		using World = EEC::World<C, T, E, S>;
		using Config = typename World::Config;
		using Bitset = typename Config::Bitset;
		Bitset bitset;
		memcpy(&bitset, data, sizeof(Bitset));
		proxy.set_bitset(bitset);
		data += sizeof(Bitset);
		EEC::MPL::forTypes<C>([&](auto v) {
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

}

namespace Systems
{

	template <typename World> class CacheSystem {

		World& world;

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
		CacheSystem(World& world) :world(world) {
			world.template subscribe<RewindEvent>(*this);
			world.template subscribe<CacheEvent>(*this);
		}

		void receive(const CacheEvent &event) { cache_world(world); }

		void receive(const RewindEvent &event) {
			if (restore_world(world))
				event.frame--;
		}
	};
}