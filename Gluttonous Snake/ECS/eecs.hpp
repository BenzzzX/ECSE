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
#pragma clang diagnostic ignored "-Wmicrosoft-template"
#include "../MPL/MPL.hpp"

#define Property(exp) static constexpr auto exp

namespace eecs {
	
	
	typedef std::size_t DataIndex;
	using BYTE = unsigned char;

#if 0
	template <typename TConfig>
	struct FrameData
	{
		size_t frame;
		size_t entityNum;
		struct
		{
			GUID uid;
			EntityData<TConfig> entityData;
		} data[0];
	};

	template <typename TConfig>
	struct EntityData
	{
		TConfig::Bitset bitset;
		BYTE data[0];
	};
#endif
	struct EntityCreatingEvent
	{
		size_t id;
	};

	struct EntityDyingEvent
	{
		size_t id;
	};

	
	struct EntityUpdateEvent
	{
		size_t id;
		size_t component;
		enum {
			change,erase,add
		}type;
	};

	using MetaEvents = MPL::TypeList
		<
		EntityCreatingEvent, EntityDyingEvent, EntityUpdateEvent
		>;
	template <typename... Ts>
	using EventList = MPL::Concat<MetaEvents, MPL::TypeList<Ts...>>;

	typedef std::size_t EventHandle;

    template <typename... Ts>
    using Signature = MPL::TypeList<Ts...>;

    template <typename... Ts>
    using ComponentList = MPL::TypeList<Ts...>;

    template <typename... Ts>
    using TagList = MPL::TypeList<Ts...>;

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

		template <typename TConfig,typename TEventList>
		class EntityManager : public EventManager<TEventList> 
		{
		public:
			using ThisType = EntityManager<TConfig, TEventList>;
			using Config = TConfig;
			using Bitset = typename Config::Bitset;
			using Entity = Impl::Entity<Config>;
			using ComponentStorage = Impl::ComponentStorage<Config>;
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
#if NDEBUG
#else
				bool valid = true;
#endif
				EntityProxy(size_t id, EntityManager& m) :e(m.entities[id]), manager(m)
				{ 
#if NDEBUG
#else
					valid = get().alive;
#endif
				}
				friend EntityManager;
				inline auto& get() { assert(valid); return e; }
				inline auto& get() const { assert(valid); return e; }

				template<typename T>
				inline void add_bit()
				{
					const auto prebitset = get().bitset;
					constexpr auto id = Config::template metaBit<T>(); 
					e.bitset[id] = true;
					manager.enter_groups(e, prebitset);
				}

				template<typename T>
				inline void erase_bit()
				{
					const auto prebitset = get().bitset;
					e.bitset[Config::template metaBit<T>()] = false;
					manager.leave_groups(e, prebitset);
				}
			public:
				~EntityProxy() 
				{ 
#if NDEBUG
#else
					valid = false;
#endif
				}
				EntityProxy(EntityProxy&) = delete;
				EntityProxy() = delete;
				EntityProxy(EntityProxy&&) = delete;
				EntityProxy& operator = (const EntityProxy&) = delete;


				auto is_alive() const noexcept { return get().alive; }

				void kill() noexcept 
				{ 
					manager.broadcast(EntityDyingEvent{ get().localID });
					get().alive = false; 
					manager.leave_groups(e, get().bitset);
#if NDEBUG
#else
					valid = false;
#endif
				}

				template <typename T>
				auto has_tag() const noexcept {
					static_assert(Config::template isTag<T>(), "Tag expect");
					return get().bitset[Config::template metaBit<T>()];
				}

				template <typename T>
				auto& add_tag() noexcept {
					static_assert(Config::template isTag<T>(), "Tag expect");
					add_bit<T>();
					return *this;
				}

				template <typename T>
				auto& erase_tag() noexcept {
					static_assert(Config::template isTag<T>(), "Tag expect");
					erase_bit<T>();
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

					
					constexpr auto id = Config::template metaBit<T>();
					if (!e.bitset[id])
					{
						add_bit<T>();
						manager.broadcast(EntityUpdateEvent{ e.localID,id,EntityUpdateEvent::add });
					}

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
					erase_bit<T>();
					constexpr auto id = Config::template metaBit<T>();
					manager.broadcast(EntityUpdateEvent{ get().localID,id,EntityUpdateEvent::erase });
					return *this;
				}

				inline size_t get_local_id() const noexcept
				{
					return get().localID;
				}

				inline const auto& get_bitset() const noexcept
				{
					return get().bitset;
				}

				inline void set_bitset(const Bitset& s) noexcept
				{
					if (get().bitset != s)
					{
						const auto prebitset = get().bitset;
						get().bitset = s;
						if((s|prebitset) == s)
							manager.leave_groups(e, prebitset);
						else manager.enter_groups(e, prebitset);
					}
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
				MPL::forTypes<typename Config::ComponentList>([](auto t)
				{
					using type = typename decltype(t)::type;
					static_assert(std::is_pod_v<type>, "Component should be POD");
				});
				grow_to(1000); 
			}

			size_t new_entity()
			{
				size_t id = create_entity_impl();
				broadcast(EntityCreatingEvent{ id });
				return id;
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
					EntityProxy proxy{ i,*this };
					mFunction(proxy);
				}
			}

			static size_t FMCounter;

			template<typename T>
			inline void create_group()
			{
				static_assert (MPL::size<T>() >= 1, "Signature should not be empty");
				groups.push_back(std::make_pair(Config::SignatureBitsets::template getBits<T>(), std::vector<size_t>{}));
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
					if (!Optimized && FMLocalCounter > size * 10 && ((float)FMLocalCounter) / FMCounter > 0.3)
					{
						Optimized = true;
						create_group<T>();
					}

					for (size_t i = 0; i < size; i++)
					{
						FMLocalCounter++; FMCounter++;
						Entity& e = entities[i];
						if (Config::SignatureBitsets::template match<T>(e.bitset))
						{
							EntityProxy proxy{ i, *this };
							ExactComponentHelper::call(proxy, mFunction);
						}
					}
				}
				else
				{
					auto& pair = groups[bestGroup];
					auto& group = pair.second;
					auto& key = pair.first;
					auto size = group.size();
					if (Config::SignatureBitsets::template getBits<T>() == key)
					{
						for (size_t i = 0; i < size; i++)
						{
							size_t id = localMap[group[i]];
							EntityProxy proxy{ id, *this };
							ExactComponentHelper::call(proxy, mFunction);
						}
					}
					else //oh we still need match
					{
						for (size_t i = 0; i < size; i++)
						{
							size_t id = localMap[group[i]];
							Entity& e = entities[i];
							if (Config::SignatureBitsets::template match<T>(e.bitset))
							{
								EntityProxy proxy{ id, *this };
								ExactComponentHelper::call(proxy, mFunction);
							}
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
					auto& e = entities[i];
					broadcast(EntityDyingEvent{ e.localID });
					e.alive = false;
				}
				size = sizeNext = 0;
			}

			template<typename F>
			void for_local(size_t id, F&& f)
			{
				size_t i = localMap[id];
				EntityProxy proxy{ i,*this };
				f(proxy);
			}
		private:

			std::size_t capacity{ 0 }, size{ 0 }, sizeNext{ 0 }, groupsize{ 0 };
			std::vector<Entity> entities;
			ComponentStorage components;
			std::vector<size_t> localMap;
			std::vector<std::pair<Bitset, std::vector<size_t>>> groups;


			auto create_entity_impl() {
				grow_if_needed();

				size_t freeIndex(sizeNext++);


				auto& e(entities[freeIndex]);
				assert(!e.alive);
				e.alive = true;
				e.bitset.reset();
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
					localMap[i] = i;
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
					mFunction(handle , handle.template get_component<Ts>()...);
				}
			};

			template<typename ...Ts>
			struct TypeSize
			{
				static constexpr size_t sizes[] = { sizeof(Ts) ... };
			};

			template<typename T>
			inline auto get_smallest_group()
			{
				size_t id = -1, ssize = this->size * 2 / 3, size = groupsize;
				for (size_t i = 0; i < size; i++)
				{
					auto& pair = groups[i];
					auto& bitset = pair.first;
					auto size = pair.second.size();
					constexpr auto sbitset = Config::SignatureBitsets::template getBits<T>();
					auto isInGroup = ((bitset&sbitset) == bitset);
					if (isInGroup)
					{
						if (ssize > size) ssize = size;
						id = i;
					}
				}
				return id;
			}

			inline void enter_groups(const Entity& e,const Bitset& prebitset)
			{
				for (size_t i = 0; i < groupsize; i++)
				{
					auto& bitset = groups[i].first;
					auto& group = groups[i].second;
					auto isEnteringGroup = ((bitset&e.bitset) == bitset);
					auto wasInGroup = ((bitset&prebitset) == bitset);
					if (!wasInGroup && isEnteringGroup)
					{
						group.push_back(e.localID);
					}
				}
			}

			inline void leave_groups(const Entity& e, const Bitset& prebitset)
			{
				for (size_t i = 0; i < groupsize; i++)
				{
					auto& bitset = groups[i].first;
					auto& group = groups[i].second;
					auto isLeavingGroup = !e.alive || ((bitset&e.bitset) != bitset);
					auto wasInGroup = ((bitset&prebitset) == bitset);
					if (isLeavingGroup&&wasInGroup)
					{
						group.erase(std::find(group.begin(), group.end(), e.localID));
					}
				}

			}

			void refresh() noexcept {

				size_t iD{ 0 }, iA{ sizeNext - 1 };
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

		template <typename TConfig,typename TEventList>
		size_t EntityManager<TConfig, TEventList>::FMCounter = 0;
	}

	template<typename TWorld>
	class System
	{
		
	protected:
		TWorld &world;
		friend TWorld;
		System() : world(*TWorld::pWorld)
		{	
		}
	};

	template <typename TComponentList, typename TTagList, typename TEventList, typename TSingletonList>
	class World : 
		public Impl::EntityManager
		<
			Impl::Config<TComponentList,TTagList>,
			TEventList
		>
	{
		MPL::Rename<std::tuple, TSingletonList> singletons;
		using EntityManager = Impl::EntityManager
		<
			Impl::Config<TComponentList, TTagList>,
			TEventList
		>;
		struct vtable { virtual ~vtable() {} };
	public:
		static World *pWorld;
		using EntityProxy = typename EntityManager::EntityProxy;

		template<typename T>
		auto& get_singleton()
		{
			return std::get<T>(singletons);
		}

		template<typename ...Ts>
		void run_with()
		{
			eecs::MPL::forTypes<MPL::TypeList<Ts...>>([&](auto v)
			{
				using t = typename decltype(v)::type;
				static_assert(sizeof(t) == sizeof(System<World>) + (std::has_virtual_destructor_v<t> ? sizeof(vtable) : 0u), "System should not have any data");
			});
			pWorld = this;
			std::initializer_list<int> _ = { (Ts{},0)... };
		}

		World() { pWorld = this; }
		~World() { pWorld = nullptr; }
	};

	template<typename C,typename T,typename E,typename S>
	World<C, T, E, S> *World<C, T, E, S>::pWorld;
}

#endif /* ecs_ecs_hpp */
