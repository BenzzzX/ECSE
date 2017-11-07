#pragma once
#ifndef ecs_ecs_hpp
#define ecs_ecs_hpp

#include <cassert>
#include <cstring>
#include <vector>

#include <bitset>
#include <memory>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <climits>
#include <cstdint>
#include <cstring>

#include "../MPL/MPL.hpp"
#include <Windows.h>

namespace std 
{
	template<> struct hash<GUID>
	{
		size_t operator()(const GUID& guid) const noexcept
		{
			static_assert(sizeof(_GUID) == 128 / CHAR_BIT, "GUID");
			const std::uint64_t* p = reinterpret_cast<const std::uint64_t*>(&guid);
			std::hash<std::uint64_t> hash;
			return hash(p[0]) ^ hash(p[1]);
		}
	};
}

namespace eecs {

	typedef std::size_t DataIndex;
	using BYTE = unsigned char;

	struct EntityID
	{
		bool serialized = false;
		size_t physicalID;
		GUID uniqueID;
	};

	typedef std::size_t EventHandle;

    template <typename... Ts>
    using Signature = MPL::TypeList<Ts...>;

    template <typename... Ts>
    using ComponentList = MPL::TypeList<Ts...>;

    template <typename... Ts>
    using TagList = MPL::TypeList<Ts...>;

	using MetaEvents = MPL::TypeList
	<
		//EntityCreationEvent,EntityDyingEvent
	>;

	template <typename... Ts>
	using EventList = MPL::Concat<MetaEvents, MPL::TypeList<Ts...>>;

	template <typename... Ts>
	using SingletonList = MPL::TypeList<Ts...>;

	namespace Impl {

		template <typename TConfig>
		struct Entity {
			using Config = TConfig;
			using Bitset = typename Config::Bitset;

			size_t localID;

			DataIndex dataIndex;
			Bitset bitset;
			bool fresh;
			bool alive;
		};

		template <typename TConfig>
		class ComponentStorage {
		public:
			void grow(std::size_t mNewCapacity) {
				vectors.resize(mNewCapacity);
			}

			template <typename T>
			auto& get_component(DataIndex eH) noexcept {
				return std::get<T>(vectors[eH]);
			}

		private:
			using Config = TConfig;
			using ComponentList = typename Config::ComponentList;

			template <typename... Ts>
			using VectorOfTuple = std::vector<std::tuple<Ts...>>;

			MPL::Rename<VectorOfTuple, ComponentList> vectors;
		};

		template <typename TConfig>
		struct SignatureBitsets {

			using Config = TConfig;
			using Bitset = typename Config::Bitset;

			template <typename T>
			using IsComponentFilter = std::integral_constant<bool, Config::template isComponent<T>()>;

			template <typename T>
			using IsTagFilter = std::integral_constant<bool, Config::template isTag<T>()>;

			template <typename TSignature>
			using SignatureComponents = MPL::Filter<IsComponentFilter, TSignature>;

			template <typename TSignature>
			using SignatureTags = MPL::Filter<IsTagFilter, TSignature>;
			/*
			note:
				using unsigned long long to initialize bitset
				cause the size of metaList limited to 64
			*/
			using ull = unsigned long long;
			template<typename ...Ts>
			static constexpr auto sum(ull arg,Ts ...args)
			{
				return arg | sum(args...);
			}


			static constexpr ull sum()
			{
				return ull{ 0 };
			}

			template<typename TC>
			static constexpr ull getBit()
			{
				return ull{ 1 } << Config::template metaBit<TC>();
			};

			template<typename ...TC>
			struct Bits
			{
				static constexpr ull value = sum(getBit<TC>()...);
			};

			template<typename TSignature>
			inline static constexpr auto getBits()
			{
				return Bitset{ MPL::Rename<Bits, TSignature>::value };
			}

			template<typename TSignature>
			inline static auto match(const Bitset& o)
			{
				constexpr auto b = getBits<TSignature>();
				
				return b == (b&o);
			}
		};


		template<typename TEventList>
		class EventManager
		{
			using EventList = typename TEventList::TypeList;

			EventHandle counter{ 0 };

			template<typename TE>
			using TSubscribers = typename std::unordered_map<EventHandle, typename std::function<void(const TE&)>>;
			using TEventMap = MPL::Tuple<MPL::Map<TSubscribers, EventList>>;
			TEventMap eventmap;

			template<typename TE>
			auto& getSubscribers() { return std::get<TSubscribers<TE>>(eventmap); }

			template<typename TE>
			static constexpr bool isEvent() { return MPL::Contains<TE, EventList>(); }
		public:
			template<typename TE, typename T>
			auto subscribe(T& o)
			{
				static_assert(isEvent<TE>(), "Event expect");
				auto& subs = getSubscribers<TE>();
				subs.insert(std::make_pair
				(
					++counter,
					std::function<void(const TE&)>([&o](const TE &e)
					{ o.receive(e); }
				)));
				return counter;
			}

			template<typename TE, typename T>
			void unsubscribe(EventHandle eH)
			{
				static_assert(isEvent<TE>(), "Event expect");
				auto& subs = getSubscribers<TE>();
				subs.erase(eH);
			}

			template<typename TE>
			void broadcast(TE&& e)
			{
				static_assert(isEvent<TE>(), "Event expect");
				auto& subs = getSubscribers<TE>();
				for (auto& pair : subs)
					pair.second(e);
			}
		};

		template <typename TConfig>
		class EntityManager 
		{
			using Config = TConfig;
			using ThisType = EntityManager<Config>;
			using Bitset = typename Config::Bitset;
			using Entity = Impl::Entity<Config>;
			using ComponentStorage = Impl::ComponentStorage<Config>;
		public:

			template<typename ...Ts>
			class EntityTemplate
			{
				using Signature = Signature<Ts...>;

				using ComponentList = typename Config::SignatureBitsets::template SignatureComponents<Signature>;
				using TagList = typename Config::SignatureBitsets::template SignatureTags<Signature>;

				MPL::Rename<std::tuple, ComponentList> metadata;

				friend class EntityProxy;
				friend EntityManager;

				template<typename ...Ts>
				EntityTemplate(Ts&& ...args)
				{
					metadata = std::make_tuple(args...);
				}
			public:
				template<typename T>
				auto& set_default(T&& v)
				{
					std::get<T>(metadata) = v;
					return *this;
				}
			};
			
			template<typename ...TTag,typename ...Ts>
			static auto make_template(Ts&& ...arg) 
			{
				using Signature = MPL::Concat<ComponentList<Ts...>, TagList<TTag...>>;
				using EntityTemplate = MPL::Rename<EntityTemplate, Signature>;
				return EntityTemplate(arg...);
			}
			
			class EntityProxy
			{
				Entity& e;
				EntityManager& manager;
				bool valid = true;
				EntityProxy(size_t id, EntityManager& m) :e(m.entities[id]), manager(m) {}
				friend EntityManager;
				inline auto& get() { assert(valid); return e; }
				inline auto& get() const { assert(valid); return e; }
			public:
				~EntityProxy() { valid = false; }
				EntityProxy(EntityProxy&) = delete;
				EntityProxy() = delete;
				EntityProxy(EntityProxy&&) = delete;
				EntityProxy& operator = (const EntityProxy&) = delete;

				auto is_alive() const noexcept { return get().alive; }

				void kill() noexcept { get().alive = false; valid = false; }

				template <typename T>
				auto has_tag() const noexcept {
					static_assert(Config::template isTag<T>(), "Tag expect");
					return get().bitset[Config::template metaBit<T>()];
				}

				template <typename T>
				auto& add_tag() noexcept {
					static_assert(Config::template isTag<T>(), "Tag expect");
					get().bitset[Config::template metaBit<T>()] = true;
					return *this;
				}

				template <typename T>
				auto& erase_tag() noexcept {
					static_assert(Config::template isTag<T>(), "Tag expect");
					get().bitset[Config::template metaBit<T>()] = false;
					return *this;
				}

				template <typename T>
				auto has_component() const noexcept {
					static_assert(Config::template isComponent<T>(), "Component expect");
					return get().bitset[Config::template metaBit<T>()];
				}

				template <typename T, typename... TArgs>
				auto& add_component(TArgs&&... mXs) noexcept {
					static_assert(Config::template isComponent<T>(), "Component expect");

					auto& e(get());
					e.bitset[Config::template metaBit<T>()] = true;

					auto& c(manager.components.template get_component<T>(e.dataIndex));
					new(&c) T{ MPL_FWD(mXs)... };

					return *this;
				}

				template <typename T>
				auto& get_component() const  noexcept {
					static_assert(Config::template isComponent<T>(), "Component expect");
					assert(has_component<T>());

					return manager.components.template get_component<T>(get().dataIndex);
				}

				template <typename T>
				auto& erase_component() noexcept {
					static_assert(Config::template isComponent<T>(), "Component expect");
					get().bitset[Config::template metaBit<T>()] = false;
					return *this;
				}

				auto& save(BYTE* &data) const noexcept
				{
					manager.save_entity_impl(get(), data);
					return *this;
				}

				auto& serialize(BYTE* &data) const noexcept
				{
					manager.serialize_entity_impl(get(), data);
					return *this;
				}

				auto& restore(BYTE* &data)
				{
					manager.restore_entity_impl(get(), data);
					return *this;
				}

				inline size_t get_size() const noexcept
				{
					return manager.get_entity_size_impl(get());
				}

				inline size_t get_local_id() const noexcept
				{
					return get().localID;
				}

				auto& duplicate(EntityProxy& other)
				{
					manager.duplicate_entity_impl(get(), other.get());
					return *this;
				}

				template<typename ...Ts>
				auto& instantiate(const EntityTemplate<Ts...>& t)
				{
					using ComponentList = typename EntityTemplate<Ts...>::ComponentList;
					using TagList = typename EntityTemplate<Ts...>::TagList;
					MPL::forTypes<ComponentList>([&](auto v)
					{
						using type = typename decltype(v)::type;
						add_component<type>(std::get<type>(t.metadata));
					});
					MPL::forTypes<TagList>([this](auto v)
					{
						using type = typename decltype(v)::type;
						add_tag<type>();
					});
					return *this;
				}
			};
			friend EntityProxy;

			

			EntityManager() 
			{ 
				MPL::forTypes<Config::ComponentList>([](auto t)
				{
					using type = typename decltype(t)::type;
					static_assert(std::is_pod_v<type>, "Component should be POD");
				});
				grow_to(1000); 
			}

			size_t new_entity()
			{
				return create_entity_impl();
			}

			void apply_changes() noexcept {

				if (sizeNext == 0) {
					size = 0;
					return;
				}

				refresh();
			}

			template<typename TF>
			inline void for_all(TF&& mFunction)
			{
				
				for (size_t i = 0; i < size; i++)
				{
					Entity& e = entities[i];
					mFunction(EntityProxy{ i,*this });
				}
			}

			static size_t FMCounter;

			template<typename T>
			inline void create_group()
			{
				static_assert (MPL::size<T>() >= 1, "Signature should not be empty");
				groups.push_back(std::make_pair(Config::SignatureBitsets::getBits<T>(), std::vector<size_t>{}));
				auto& bitset = groups[groupsize].first;
				auto& group = groups[groupsize].second;
				for (int i = 0; i < size; i++)
				{
					auto& e = entities[i];
					auto isEnteringGroup = ((bitset&e.bitset) == bitset);
					if (isEnteringGroup)
					{
						group.push_back(e.localID);
					}
				}
				++groupsize;
			}

			template <typename T, typename TF>
			inline void for_matching(TF&& mFunction) noexcept {
				static_assert (MPL::size<T>() >= 1, "Signature should not be empty");

				using RequiredComponents = typename Config::SignatureBitsets::template SignatureComponents<T>;
				using ExactComponentHelper = MPL::Rename<ExpandCallHelper, RequiredComponents>;

				static size_t FMLocalCounter = 0;
				static bool Optimized = false;

				auto bestGroup = get_smallest_group<T>();

				if (bestGroup == -1)
				{
					/*Heuristic optimization*/
					if (!Optimized && FMLocalCounter > size * 3 && ((float)FMLocalCounter) / FMCounter > 0.3)
					{
						Optimized = true;
						create_group<T>();
					}

					for (size_t i = 0; i < size; i++)
					{
						FMLocalCounter++; FMCounter++;
						Entity& e = entities[i];
						if (Config::SignatureBitsets::match<T>(e.bitset))
							ExactComponentHelper::call(EntityProxy{ i, *this }, mFunction);
					}
				}
				else
				{
					auto& pair = groups[bestGroup];
					auto& group = pair.second;
					auto& key = pair.first;
					auto size = group.size();
					if (Config::SignatureBitsets::getBits<T>() == key)
					{
						for (size_t i = 0; i < size; i++)
						{
							size_t id = localMap[group[i]].physicalID;
							Entity& e = entities[i];
							ExactComponentHelper::call(EntityProxy{ id, *this }, mFunction);
						}
					}
					else //oh we still need match
					{
						for (size_t i = 0; i < size; i++)
						{
							size_t id = localMap[group[i]].physicalID;
							Entity& e = entities[i];
							if (Config::SignatureBitsets::match<T>(e.bitset))
								ExactComponentHelper::call(EntityProxy{ id, *this }, mFunction);
						}
					}
				}

			}

			void kill_all()
			{
				apply_changes();
				for (size_t i = 0; i < groupsize; i++)
				{
					groups[i].second.clear();
				}
				for (size_t i = 0; i < size; ++i)
				{
					entities[i].alive = false;
				}
				size = sizeNext = 0;
			}

			template<typename F>
			void for_local(size_t id, F&& f)
			{
				size_t i = localMap[id].physicalID;
				f(EntityProxy{ i,*this });
			}

			void serialize(size_t id, BYTE* &data)
			{
				serialize_entity_impl(entities[localMap[id].physicalID], data);
			}

			size_t unserialize(BYTE* &data)
			{
				return unserialize_entity_impl(data);
			}
		private:

			std::size_t capacity{ 0 }, size{ 0 }, sizeNext{ 0 }, groupsize{ 0 };
			std::vector<Entity> entities;
			ComponentStorage components;
			std::vector<EntityID> localMap;
			std::vector<std::pair<Bitset, std::vector<size_t>>> groups;
			std::unordered_map<GUID, size_t> uniqueMap;


			auto create_entity_impl() {
				grow_if_needed();

				size_t freeIndex(sizeNext++);


				auto& e(entities[freeIndex]);
				assert(!e.alive);
				e.alive = true;
				e.bitset.reset();
				e.fresh = true;
				return e.localID;
			}

			void grow_to(std::size_t mNewCapacity) {

				assert(mNewCapacity > capacity);

				entities.resize(mNewCapacity);
				components.grow(mNewCapacity);
				localMap.resize(mNewCapacity);

				for (auto i(capacity); i < mNewCapacity; ++i) {
					auto& e(entities[i]);
					e.dataIndex = i;
					e.bitset.reset();
					e.alive = false;
					e.localID = i;
					e.fresh = false;
					localMap[i].physicalID = i;
				}

				capacity = mNewCapacity;
			}

			void grow_if_needed() {
				if (capacity > sizeNext) return;
				grow_to((capacity + 10) * 2);
			}

			template <typename... Ts>
			struct ExpandCallHelper {
				template <typename TF>
				static void call(EntityProxy& handle, TF&& mFunction) {
					mFunction(handle , handle.get_component<Ts>()...);
				}
			};

			template<typename ...Ts>
			struct TypeSize
			{
				static constexpr size_t sizes[] = { sizeof(Ts) ... };
			};

			inline size_t get_entity_size_impl(const Entity& e)
			{
				size_t total = 0;
				const auto& bitset = e.bitset;
				using ComponentSize = MPL::Rename<TypeSize, Config::ComponentList>;
				for (size_t i = 0; i < MPL::size<Config::ComponentList>(); i++)
				{
					total += bitset[i] * ComponentSize::sizes[i];
				}
				return total + sizeof(Bitset);
			}

			inline void save_entity_impl(const Entity& e, BYTE* &data)
			{
				const auto& bitset = e.bitset;
				memcpy(data, &e.bitset, sizeof(Bitset));
				data += sizeof(Bitset);
				MPL::forTypes<Config::ComponentList>([&](auto v)
				{
					using t = typename decltype(v)::type;
					constexpr auto id = Config::metaBit<t>();
					if (bitset[id])
					{
						const t& c = components.get_component<t>(e.dataIndex);
						memcpy(data, &c, sizeof(t));
						data += sizeof(t);
					}
				});
			}

			inline void serialize_entity_impl(const Entity& e, BYTE* &data)
			{
				auto& id = localMap[e.localID];
				if (!id.serialized)
				{
					id.serialized = true;
					CoCreateGuid(&id.uniqueID);
					uniqueMap.insert(std::make_pair(id.uniqueID, e.localID));
				}
				memcpy(data, &id.uniqueID, sizeof(GUID));
				data += sizeof(GUID);
				save_entity_impl(e, data);
			}

			inline size_t unserialize_entity_impl(BYTE* &data)
			{
				GUID uniqueID;
				memcpy(&uniqueID, data, sizeof(GUID));
				data += sizeof(GUID);
				if (uniqueMap.find(uniqueID) == uniqueMap.end())
				{
					auto localID = create_entity_impl();
					auto& e = entities[localID];
					auto& id = localMap[localID];
					id.serialized = true;
					id.uniqueID = uniqueID;
					restore_entity_impl(e, data);
					return e.localID;
				}
				else
				{
					auto& e = entities[localMap[uniqueMap[uniqueID]].physicalID];
					restore_entity_impl(e, data);
					return e.localID;
				}
			}

			void restore_entity_impl(Entity& e, BYTE* &data)
			{
				auto& bitset = e.bitset;
				memcpy(&bitset, data, sizeof(Bitset));
				data += sizeof(Bitset);
				MPL::forTypes<Config::ComponentList>([&](auto v)
				{
					using t = typename decltype(v)::type;
					constexpr auto id = Config::metaBit<t>();
					if (bitset[id])
					{
						t& c = components.get_component<t>(e.dataIndex);
						memcpy(&c, data, sizeof(t));
						data += sizeof(t);
					}
				});
			}

			void duplicate_entity_impl(Entity& e, Entity& s)
			{
				auto& bitset = e.bitset;
				bitset = s.bitset;
				MPL::forTypes<Config::ComponentList>([&](auto v)
				{
					using t = typename decltype(v)::type;
					constexpr auto id = Config::metaBit<t>();
					if (bitset[id])
					{
						t& ec = components.get_component<t>(e.dataIndex);
						t& sc = components.get_component<t>(s.dataIndex);
						ec = sc;
					}
				});
			}

			template<typename T>
			inline auto get_smallest_group()
			{
				size_t id = -1, ssize = this->size * 2 / 3, size = groupsize;
				for (size_t i = 0; i < size; i++)
				{
					auto& pair = groups[i];
					auto& bitset = pair.first;
					auto size = pair.second.size();
					constexpr auto sbitset = Config::SignatureBitsets::getBits<T>();
					auto isInGroup = ((bitset&sbitset) == bitset);
					if (isInGroup)
					{
						if (ssize > size) ssize = size;
						id = i;
					}
				}
				return id;
			}

			inline void update_group(size_t groupID)
			{
				auto& bitset = groups[groupID].first;
				auto& group = groups[groupID].second;
				for (int i = 0; i < size; i++)
				{
					auto& e = entities[i];
					auto isFresh = e.fresh;
					auto isEnteringGroup = ((bitset&e.bitset) == bitset);
			
					if (isFresh && isEnteringGroup)
					{
						group.push_back(e.localID);
					}
					
				}
			}

			inline void fresh_group(size_t groupID)
			{
				auto& bitset = groups[groupID].first;
				auto& group = groups[groupID].second;
				for (int i = 0; i < size; i++)
				{
					auto& e = entities[i];
					auto isLeavingGroup = !e.alive;
					auto isFresh = e.fresh;
					auto isInGroup = ((bitset&e.bitset) == bitset) && !isFresh;
					if (isInGroup && isLeavingGroup)
					{
						group.erase(std::find(group.begin(), group.end(), e.localID));
					}
				}
			}

			void refresh() noexcept {

				size_t iD{ 0 }, iA{ sizeNext - 1 };
				groupsize = groups.size();

				/*remove dying entities from groups*/
				for (int i = 0; i < groupsize; i++)
				{
					fresh_group(i);
				}

				/*
				collect living entities together
				*/
				while (true) {
					for (; true; ++iD) {
						if (iD > iA) goto complete;
						if (!entities[iD].alive) break;
					}

					for (; true; --iA) {
						if (entities[iA].alive) break;
						if (iA <= iD) goto complete;
					}
					auto& eA = entities[iA];
					auto& eD = entities[iD];
					
					assert(eA.alive);
					assert(!eD.alive);

					std::swap(eA, eD);
					std::swap(localMap[eA.localID], localMap[eD.localID]);

					++iD;
					--iA;
				}
				complete:
				size = sizeNext = iD;

				/*add fresh entities to groups*/
				for (int i = 0; i < groupsize; i++)
				{
					update_group(i);
				}

				for (int i = 0; i < size; i++)
				{
					auto& e = entities[i];
					e.fresh = false;;
				}
			}
		};

		template <typename TComponentList, typename TTagList>
		struct Config {
			using ComponentList = typename TComponentList::TypeList;
			using TagList = typename TTagList::TypeList;
			using MetaList = MPL::Concat<ComponentList, TagList>;

			using SignatureBitsets = Impl::SignatureBitsets<Config
				<
				TComponentList, TTagList
				>>;

			template <typename T>
			static constexpr bool isComponent() noexcept {
				return MPL::Contains<T, ComponentList>{};
			}

			template <typename T>
			static constexpr bool isTag() noexcept {
				return MPL::Contains<T, TagList>{};
			}

			static constexpr auto bitCount = MPL::size<MetaList>();
			using Bitset = std::bitset<bitCount>;

			template <typename T>
			static constexpr std::size_t metaBit() noexcept {
				return MPL::IndexOf<T, MetaList>();
			}
		};

		template <typename TConfig>
		size_t EntityManager<TConfig>::FMCounter = 0;
	}

	template <typename TComponentList, typename TTagList, typename TEventList, typename TSingletonList>
	class World : 
		public Impl::EntityManager
		<
			Impl::Config<TComponentList,TTagList>
		>,
		public Impl::EventManager<TEventList>
	{
		MPL::Rename<std::tuple, typename TSingletonList> singletons;
		using EntityManager = Impl::EntityManager
		<
			Impl::Config<TComponentList, TTagList>
		>;

		static World *pWorld;
	public:
		using EntityProxy = EntityManager::EntityProxy;
		template<typename T>
		auto& getSingleton()
		{
			return std::get<T>(singletons);
		}

		World() { pWorld = this; }

		template<typename T>
		struct System
		{
			World &world;
			struct vtable { virtual ~vtable() {} };
			System() : world(*pWorld)
			{
				
				static_assert(sizeof(T) == sizeof(System) + (std::has_virtual_destructor_v<T> ? sizeof(vtable) : 0u), "System should not have any data");
			}
		};
	};

	template<typename C,typename T,typename E,typename S>
	World<C, T, E, S> *World<C, T, E, S>::pWorld;

	

}

#endif /* ecs_ecs_hpp */
