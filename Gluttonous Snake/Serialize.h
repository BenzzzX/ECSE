#pragma once
#include "ECS\EEC.hpp"
#include <Windows.h>
#include "to_tuple.h"

namespace std {
	template <> struct hash<GUID> {
		size_t operator()(const GUID &guid) const noexcept {
			static_assert(sizeof(GUID) == 128 / CHAR_BIT, "GUID");
			const std::uint64_t *p = reinterpret_cast<const std::uint64_t *>(&guid);
			std::hash<std::uint64_t> hash;
			return hash(p[0]) ^ hash(p[1]);
		}
	};
} // namespace std

template < typename T >
struct IDType
{
	T id;
	operator T() { return id; }
	operator T() const { return id; }
	auto& operator = (T r) { id = r; return *this; }
};
using EntityID = IDType<size_t>;
using UniqueID = IDType<GUID>;
enum class SerializeStrategy : size_t { Always, Changing, Never };

struct CUniqueID {
	GUID uid;
	Property(serializeStrategy = SerializeStrategy::Never);
};

struct UniqueData {
	std::unordered_map<GUID, size_t> map;
	std::unordered_map<GUID, std::vector<BYTE>> cache;
#ifdef NDEBUG
#else
	bool enabled = false;
#endif
};


template <typename T, typename = void> struct GetSerializeStrategy {
	static constexpr auto value = SerializeStrategy::Always;
};

template <typename T>
struct GetSerializeStrategy<T, decltype(T::serializeStrategy, void())> {
	static constexpr auto value = T::serializeStrategy;
};

namespace Util
{
	template <typename C, typename T, typename E, typename S, typename V>
	auto wrap_ref(EEC::World<C, T, E, S> &world, V& v)
	{
		static_assert(EEC::MPL::Contains<UniqueData, S>{},
			"translate_id only enable when UniqueMap is available");

		using namespace EEC::MPL;

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
	void serialize(EEC::World<C, T, E, S> &world,
		typename EEC::World<C, T, E, S>::EntityProxy &proxy,
		BYTE *&data) {
		static_assert(EEC::MPL::Contains<CUniqueID, C>{},
			"serialize only enable when CUniqueID is available");
		static_assert(EEC::MPL::Contains<UniqueData, S>{},
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
		using World = EEC::World<C, T, E, S>;
		using Config = typename World::Config;
		using Bitset = typename Config::Bitset;

		const auto &uid = proxy.template get_component<CUniqueID>();
		memcpy(data, &uid, sizeof(uid));
		data += sizeof(uid);
		const auto &bitset = proxy.get_bitset();
		memcpy(data, &bitset, sizeof(Bitset));
		data += sizeof(Bitset);
		Bitset dirty, cacheBitset;
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

		EEC::MPL::forTypes<C>([&](auto v) {
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
					else if (cacheBitset[id])
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
		if (cache.capacity() * sizeof(BYTE) < cacheNeedSize) cache.resize(cacheNeedSize / sizeof(BYTE) + 1);
		cacheData = cache.data();

		memcpy(data, &dirty, sizeof(Bitset));
		data += sizeof(Bitset);
		memcpy(cacheData, &bitset, sizeof(Bitset));
		cacheData += sizeof(Bitset);

		EEC::MPL::forTypes<C>([&](auto v) {
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

}

namespace ImpureUtil
{
	template <typename C, typename T, typename E, typename S, typename ...V>
	auto unwrap_ref(EEC::World<C, T, E, S> &world, const std::tuple<V...>& v)
	{
		static_assert(EEC::MPL::Contains<UniqueData, S>{},
			"translate_id only enable when UniqueMap is available");
		auto &uM = world.template get_singleton<UniqueData>();
		assert(uM.enabled);
		auto &uniqueMap = uM.map;

		using namespace EEC::MPL;
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
	void unserialize(EEC::World<C, T, E, S> &world, BYTE *&data) {
		static_assert(EEC::MPL::Contains<CUniqueID, C>{},
			"serialize only enable when CUniqueID is available");
		static_assert(EEC::MPL::Contains<UniqueData, S>{},
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
			using World = EEC::World<C, T, E, S>;
			using Config = typename World::Config;
			using Bitset = typename Config::Bitset;
			proxy.template add_component<CUniqueID>(uid);
			Bitset bitset, dirty;
			memcpy(&bitset, data, sizeof(Bitset));
			data += sizeof(Bitset);
			proxy.set_bitset(bitset);
			memcpy(&dirty, data, sizeof(Bitset));
			data += sizeof(Bitset);
			EEC::MPL::forTypes<C>([&](auto v) {
				using t = typename decltype(v)::type;
				constexpr auto id = Config::template metaBit<t>();
				if (dirty[id]) {
					t c;
					using wraped_type = decltype(Util::wrap_ref(world, c));
					wraped_type wrappedc;
					memcpy(&wrappedc, data, sizeof(wrappedc));
					data += sizeof(wrappedc);
					c = EEC::MPL::to_struct<t>(unwrap_ref(world, std::move(wrappedc)));
					proxy.template add_component<t>(c);
				}
			});
		});
	}
}

namespace Systems
{
	template <typename World> 
	class SerializeSystem {
		World& world;
	public:
		void receive(const EEC::EntityDyingEvent &event) {
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

		SerializeSystem(World& world):world(world) {
			auto &uM = world.template get_singleton<UniqueData>();
			assert(uM.enabled = true);
			world.template subscribe<typename EEC::EntityDyingEvent>(*this);
		}
	};

}