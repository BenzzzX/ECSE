//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <initializer_list>

#include "event.h"

namespace entityplus {
namespace detail {
#define ENTITY_TEMPS \
template <typename... CTs, typename... TTs> 

#define ENTITY_SPEC \
entity<component_list<CTs...>, tag_list<TTs...>>

ENTITY_TEMPS
template <typename Component>
bool ENTITY_SPEC::has_component() const {
	assert(entityManager);
	using IsCompValid = meta::typelist_has_type<Component, component_t>;
	return meta::eval_if(
		[&](auto) { 
			return meta::get<Component>(compTags); 
		},
		meta::fail_cond<IsCompValid>([](auto id) {
			static_assert(id(false), "has_component called with invalid component");
			return false;
		})
	);
}

ENTITY_TEMPS
template <typename Component, typename... Args>
std::pair<Component&, bool> ENTITY_SPEC::add_component(Args&&... args) {
	assert(entityManager);
	using IsCompValid = meta::typelist_has_type<Component, component_t>;
	using IsConstructible = std::is_constructible<Component, Args&&...>;
	auto argTuple = std::forward_as_tuple(std::forward<Args>(args)...);
	return meta::eval_if(
		[&](auto) { 
			return entityManager->template add_component<Component>(*this, std::move(argTuple)); 
		},
		meta::fail_cond<IsCompValid>([](auto id) {
			static_assert(id(false), "add_component called with invalid component");
			return std::declval<std::pair<Component&, bool>>(); 
		}),
		meta::fail_cond<IsConstructible>([](auto id) {
			static_assert(id(false), "add_component cannot construct component with given args");
			return std::declval<std::pair<Component&, bool>>();
		})
	);
}

ENTITY_TEMPS
template <typename Component>
bool ENTITY_SPEC::remove_component() {
	assert(entityManager);
	using IsCompValid = meta::typelist_has_type<Component, component_t>;
	return meta::eval_if(
		[&](auto) { 
			return entityManager->template remove_component<Component>(*this); 
		},
		meta::fail_cond<IsCompValid>([](auto id) {
			static_assert(id(false), "remove_component called with invalid component");
			return false;
		})
	);
}

ENTITY_TEMPS
template <typename Component>
const Component& ENTITY_SPEC::get_component() const {
	assert(entityManager);
	using IsCompValid = meta::typelist_has_type<Component, component_t>;
	return meta::eval_if(
		[&](auto) -> decltype(auto) {
			return entityManager->template get_component<Component>(*this); 
		},
		meta::fail_cond<IsCompValid>([](auto id) {
			static_assert(id(false), "get_component called with invalid component");
			return std::declval<const Component &>();
		})
	);
}

ENTITY_TEMPS
template <typename Tag>
bool ENTITY_SPEC::has_tag() const {
	assert(entityManager);
	using IsTagValid = meta::typelist_has_type<Tag, tag_t>;
	return meta::eval_if(
		[&](auto) { 
			return meta::get<Tag>(compTags); 
		},
		meta::fail_cond<IsTagValid>([](auto id) {
			static_assert(id(false), "has_tag called with invalid tag");
			return false;
		})
	);
}

ENTITY_TEMPS
template <typename Tag>
bool ENTITY_SPEC::set_tag(bool set) {
	assert(entityManager);
	using IsTagValid = meta::typelist_has_type<Tag, tag_t>;
	return meta::eval_if(
		[&](auto) { 
			return entityManager->template set_tag<Tag>(*this, set); 
		},
		meta::fail_cond<IsTagValid>([](auto id) {
			static_assert(id(false), "set_tag called with invalid tag");
			return false;
		})
	);
}

#undef ENTITY_TEMPS
#undef ENTITY_SPEC
} // namespace detail

#define ENTITY_MANAGER_TEMPS \
template <typename... CTs, typename... TTs> 

#define ENTITY_MANAGER_SPEC \
entity_manager<component_list<CTs...>, tag_list<TTs...>>

namespace detail {
template <typename... Ts, typename Container, std::size_t... Is>
void initialize_groupings_impl(flat_map<entity_grouping_id_t,
	std::pair<meta::type_bitset<meta::typelist<Ts...>>, Container>> &groupings,
							   std::index_sequence<Is...>) {
	(void)groupings;
	std::initializer_list<int> _ =
	{((void)groupings.emplace(Is,
							  std::make_pair(meta::make_key<meta::typelist<Ts>, meta::typelist<Ts...>>(),
											 Container{})),
	  0)...};
}

template <typename... Ts, typename Container>
void initialize_groupings(flat_map<entity_grouping_id_t,
	std::pair<meta::type_bitset<meta::typelist<Ts...>>, Container>> &groupings) {
	initialize_groupings_impl(groupings, std::index_sequence_for<Ts...>{});
}
} // namespace detail

ENTITY_MANAGER_TEMPS
ENTITY_MANAGER_SPEC::entity_manager() {
	detail::initialize_groupings(groupings);
}

ENTITY_MANAGER_TEMPS
void ENTITY_MANAGER_SPEC::report_error(error_code_t errCode, const char * msg) const {
#ifdef ENTITYPLUS_NO_EXCEPTIONS
	handle_error(errCode, msg);
#else
	switch (errCode) {
	case entityplus::error_code_t::BAD_ENTITY:
		throw bad_entity(msg);
	case entityplus::error_code_t::INVALID_COMPONENT:
		throw invalid_component(msg);
	}
	// unreachable
	assert(0);
	std::terminate();
#endif
}

ENTITY_MANAGER_TEMPS
auto ENTITY_MANAGER_SPEC::get_entity_and_status(const entity_t &entity) const -> std::pair<const entity_t*, entity_status> {
	auto local = entities.find(entity);
	if (local == entities.end())
		return{nullptr, entity_status::DELETED};

	if (entity.compTags != local->compTags)
		return{&*local, entity_status::STALE};

	return{&*local, entity_status::OK};
}

#if !NDEBUG
ENTITY_MANAGER_TEMPS
auto ENTITY_MANAGER_SPEC::assert_entity(const entity_t &entity) const -> const entity_t&{
	auto entStatus = get_entity_and_status(entity);
	switch (entStatus.second) {
	case entity_status::DELETED:
		report_error(error_code_t::BAD_ENTITY,
					 "Entity has been deleted.");
	case entity_status::STALE:
		report_error(error_code_t::BAD_ENTITY,
					 "Entity's components/tags are stale. Don't store stale entities.");
	case entity_status::OK:
		return *entStatus.first;
	default:
		// unreachable
		assert(0);	
		std::terminate();
	}

}
#endif

ENTITY_MANAGER_TEMPS
template <typename T>
void ENTITY_MANAGER_SPEC::add_bit(entity_t &local, entity_t &foreign) {
	auto prevBits = local.compTags;

	meta::get<T>(local.compTags) =
		meta::get<T>(foreign.compTags) = true;

	meta::type_bitset<comp_tag_t> singleBit;
	meta::get<T>(singleBit) = true;
	for (auto &groupingEntry : groupings) {
		const auto &groupingBitset = groupingEntry.second.first;
		auto &groupingContainer = groupingEntry.second.second;
		bool wasInGrouping = (groupingBitset & prevBits) == groupingBitset,
			enteredGrouping = (groupingBitset & (prevBits | singleBit)) == groupingBitset;
		if (!wasInGrouping && enteredGrouping) {
			groupingContainer.emplace(local);
		}
		else if (wasInGrouping) {
			auto ent = groupingContainer.find(local);
			assert(ent != groupingContainer.end());
			meta::get<T>(ent->compTags) = true;
		}
	}
}

ENTITY_MANAGER_TEMPS
template <typename T>
void ENTITY_MANAGER_SPEC::remove_bit(entity_t &local, entity_t &foreign) {
	meta::get<T>(local.compTags) =
		meta::get<T>(foreign.compTags) = false;

	meta::type_bitset<comp_tag_t> singleBit;
	meta::get<T>(singleBit) = true;
	for (auto &groupingEntry : groupings) {
		const auto &groupingBitset = groupingEntry.second.first;
		auto &groupingContainer = groupingEntry.second.second;
		bool inGrouping = (groupingBitset & local.compTags) == groupingBitset,
			wasInGrouping = (groupingBitset & (local.compTags | singleBit)) == groupingBitset;
		if (!inGrouping && wasInGrouping) {
			auto er = groupingContainer.erase(local);
			(void)er; assert(er == 1);
		}
		else if (inGrouping) {
			auto ent = groupingContainer.find(local);
			assert(ent != groupingContainer.end());
			meta::get<T>(ent->compTags) = false;
		}
	}
}

ENTITY_MANAGER_TEMPS
template <typename Component, typename... Args>
std::pair<Component&, bool> 
ENTITY_MANAGER_SPEC::add_component(entity_t &entity, std::tuple<Args...> &&args) {
#if NDEBUG
	auto &myEnt = *entities.find(entity);
#else
	auto &myEnt = assert_entity(entity);
#endif
	assert(meta::get<Component>(entity.compTags) == meta::get<Component>(myEnt.compTags));
	
	auto &container = meta::get<Component, component_list_t>(components);
	if (meta::get<Component>(entity.compTags)) {
		auto comp = container.find(entity.id);
		assert(comp != container.end());
		return{comp->second, false};
	}

	auto comp = container.emplace(std::piecewise_construct, std::make_tuple(entity.id), std::move(args));
	assert(comp.second);

	add_bit<Component>(myEnt, entity);

	if (eventManager) eventManager->broadcast(component_added<entity_t, Component>{myEnt, comp.first->second});

	return {comp.first->second, true};
}

ENTITY_MANAGER_TEMPS
template <typename Component>
bool ENTITY_MANAGER_SPEC::remove_component(entity_t &entity) {
#if NDEBUG
	auto &myEnt = *entities.find(entity);
#else
	auto &myEnt = assert_entity(entity);
#endif
	assert(meta::get<Component>(entity.compTags) == meta::get<Component>(myEnt.compTags));
	if (!meta::get<Component>(entity.compTags)) {
		return false;
	}

	auto &container = meta::get<Component, component_list_t>(components);
	auto comp = container.find(entity.id);
	assert(comp != container.end());

	if (eventManager) eventManager->broadcast(component_removed<entity_t, Component>{myEnt, comp->second});

	container.erase(comp);

	remove_bit<Component>(myEnt, entity);

	return true;
}

ENTITY_MANAGER_TEMPS
template <typename Component>
const Component& ENTITY_MANAGER_SPEC::get_component(const entity_t &entity) const {
#if !NDEBUG
	auto &myEnt = assert_entity(entity);
	assert(meta::get<Component>(entity.compTags) == meta::get<Component>(myEnt.compTags));
#endif
	if (!meta::get<Component>(entity.compTags)) {
		report_error(error_code_t::INVALID_COMPONENT,
					 "Tried to get a component the entity does not have");
	}

	const auto &container = meta::get<Component, component_list_t>(components);
	auto comp = container.find(entity.id);
	assert(comp != container.end());
	return comp->second;
}

ENTITY_MANAGER_TEMPS
template <typename Tag>
bool ENTITY_MANAGER_SPEC::set_tag(entity_t &entity, bool set) {
#if NDEBUG
	auto &myEnt = *entities.find(entity);
#else
	auto &myEnt = assert_entity(entity);
#endif
	assert(meta::get<Tag>(entity.compTags) == meta::get<Tag>(myEnt.compTags));

	bool old = meta::get<Tag>(entity.compTags);
	if (old != set) {
		if (set) {
			add_bit<Tag>(myEnt, entity);
			if (eventManager)
				eventManager->broadcast(tag_added<entity_t, Tag>{myEnt});
		}
		else {
			if (eventManager)
				eventManager->broadcast(tag_removed<entity_t, Tag>{myEnt});
			remove_bit<Tag>(myEnt, entity);
		}
	}
	return old;
}

ENTITY_MANAGER_TEMPS
template <typename... Ts, typename... Us>
auto ENTITY_MANAGER_SPEC::create_entity(Us&&... us) -> entity_t {
	using AreTagsValid = meta::and_all<meta::typelist_has_type<Ts, tag_t>...>;
	using AreTagsUnique = meta::is_typelist_unique<meta::typelist<Ts...>>;

	using AreCompsValid = meta::and_all<meta::typelist_has_type<std::decay_t<Us>, component_t>...>;
	using AreCompsUnique = meta::is_typelist_unique<meta::typelist<std::decay_t<Us>...>>;

	return meta::eval_if(
		[&](auto) {
			//assert(std::numeric_limits<detail::entity_id_t>::max() != currentEntityId);
			auto emp = entities.emplace(typename entity_t::private_access{}, currentEntityId++, this);
			assert(emp.second);

			auto &ent = *emp.first;

			if (eventManager) eventManager->broadcast(entity_created<entity_t>{ent});

			this->set_tags<Ts...>(ent, true);
			this->add_components(ent, decltype(us)(us)...);

			return ent;
		},
		meta::fail_cond<AreTagsValid>([](auto id) {
			static_assert(id(false), "create_entity called with invalid tags");
			return std::declval<entity_t>();
		}),
		meta::fail_cond<AreTagsUnique>([](auto id) {
			static_assert(id(false), "create_entity called with non-unique tags");
			return std::declval<entity_t>();
		}),
		meta::fail_cond<AreCompsValid>([](auto id) {
			static_assert(id(false), "create_entity called with invalid components");
			return std::declval<entity_t>();
		}),
		meta::fail_cond<AreCompsUnique>([](auto id) {
			static_assert(id(false), "create_entity called with non-unique tags");
			return std::declval<entity_t>();
		})
	);

}

ENTITY_MANAGER_TEMPS
void ENTITY_MANAGER_SPEC::destroy_entity(const entity_t &entity) {
#if !NDEBUG
	assert_entity(entity);
#endif

	if (eventManager) eventManager->broadcast(entity_destroyed<entity_t>{entity});

	meta::for_each(components, [&](auto &container, std::size_t idx, auto type_holder) {
		(void)type_holder;
		if (entity.compTags[idx]) {
			auto comp = container.find(entity.id);
			assert(comp != container.end());
			if (eventManager) 
				eventManager->broadcast(component_removed<entity_t, 
										typename decltype(type_holder)::type::mapped_type>{entity, comp->second});
			container.erase(comp);
		}
	});
	
	meta::for_each<ComponentCount>(entity.compTags, [&](std::size_t idx, auto type_holder) {
		(void)type_holder;
		if (entity.compTags[idx]) {
			if (eventManager)
				eventManager->broadcast(tag_removed<entity_t,
										typename decltype(type_holder)::type>{entity});
		}
	});

	for (auto &groupingEntry : groupings) {
		const auto &groupingBitset = groupingEntry.second.first;
		auto &groupingContainer = groupingEntry.second.second;
		if ((groupingBitset & entity.compTags) == groupingBitset) {
			auto er = groupingContainer.erase(entity);
			(void)er; assert(er == 1);
		}
	}

	auto er = entities.erase(entity); 
	(void)er; assert(er == 1);
}

ENTITY_MANAGER_TEMPS
bool ENTITY_MANAGER_SPEC::sync(entity_t &entity) const {
	auto entStatus = get_entity_and_status(entity);
	if (entStatus.second == entity_status::DELETED) return false;
	entity = *entStatus.first;
	return true;
}

ENTITY_MANAGER_TEMPS
template <typename... Ts>
auto ENTITY_MANAGER_SPEC::get_smallest_container() -> std::pair<entity_container&, bool> {
	if (sizeof...(Ts) == 0) return{entities, true};

	auto key = meta::make_key<meta::typelist<Ts...>, comp_tag_t>();
	auto & contPair = std::min_element(groupings.begin(), groupings.end(),
							[&key](const auto &lhs, const auto & rhs) {
		if ((lhs.second.first & key) == lhs.second.first && (rhs.second.first & key) == rhs.second.first)
			return lhs.second.second.size() < rhs.second.second.size();
		if ((lhs.second.first & key) == lhs.second.first && (rhs.second.first & key) != rhs.second.first)
			return true;
		return false;
	})->second;
	return{contPair.second, contPair.first == key};
}

ENTITY_MANAGER_TEMPS
template <typename... Ts>
auto ENTITY_MANAGER_SPEC::get_entities() -> return_container {
	constexpr auto ArgSize = sizeof...(Ts);
	using Typelist = meta::typelist<Ts...>;
	using IsTypelistUnique = meta::is_typelist_unique<Typelist>;
	using IsTypelistValid = meta::and_all<meta::typelist_has_type<Ts, comp_tag_t>...>;
	return meta::eval_if(
		[&](auto) {
			auto smallest = this->get_smallest_container<Ts...>();
			auto &smallestContainer = smallest.first;
			if (smallest.second) {
				return return_container{smallestContainer.begin(), smallestContainer.end()};
			}
			return_container ret;
			ret.reserve(smallestContainer.size());

			auto key = meta::make_key<Typelist, comp_tag_t>();

			for (const auto &ent : smallestContainer) {
				if ((ent.compTags & key) == key)
					ret.push_back(ent);
			}
			return ret; 
		},
		meta::fail_cond<IsTypelistValid>([](auto id) {
			static_assert(id(false), "get_entitites called with invalid typelist");
			return std::declval<return_container>();
		}),
		meta::fail_cond<IsTypelistUnique>([](auto id) {
			static_assert(id(false), "get_entitites called with a non-unique typelist");
			return std::declval<return_container>();
		})
	);
}

namespace detail{
template <typename T, typename U> struct func_sig_no_control;
template <typename T, typename... Ts> struct func_sig_no_control<T, meta::typelist<Ts...>> {
	using type = void(T, Ts&...);
};

template <typename T, typename U> struct func_sig_with_control;
template <typename T, typename... Ts> struct func_sig_with_control<T, meta::typelist<Ts...>> {
	using type = void(T, Ts&..., control_block_t &);
};

template <typename Iter>
struct data_t {
	Iter pos, end;
	bool useLinear;
};

template <typename Iter>
auto make_data(Iter begin, Iter end, bool use) {
	return data_t<Iter>{begin, end, use};
}

template <typename T, typename U> struct make_iters;
template <typename T, typename... Us> struct make_iters<T, meta::typelist<Us...>> {
	template <typename Container>
	auto operator()(Container &c, std::size_t smallestIdxSize, std::size_t maxLinearSearchDistance) const {
		(void)c; (void)smallestIdxSize; (void)maxLinearSearchDistance;
		return std::make_tuple(make_data(meta::get<Us, T>(c).begin(),
										 meta::get<Us, T>(c).end(),
										 meta::get<Us, T>(c).size()/smallestIdxSize < maxLinearSearchDistance)...);
	}
};

template <typename Func, typename Func2, typename T, typename... Ts, std::size_t... Is>
void deref_and_invoke_impl(Func &&func, Func2 &&func2, T &&t, std::tuple<Ts...> &iters,
						   control_block_t &, std::index_sequence<Is...>,
						   std::false_type) {
	(void)func; (void)func2; (void)iters;
	func(std::forward<T>(t), func2(std::get<Is>(iters))...);
}

template <typename Func, typename Func2, typename T, typename... Ts, std::size_t... Is>
void deref_and_invoke_impl(Func &&func, Func2 &&func2, T &&t, std::tuple<Ts...> &iters,
						   control_block_t &control, std::index_sequence<Is...>,
						   std::true_type) {
	(void)func; (void)func2; (void)iters; (void)control;
	func(std::forward<T>(t), func2(std::get<Is>(iters))..., control);
}

template <typename Func, typename Func2, typename T, typename... Ts, typename Cond>
void deref_and_invoke(Func &&func, Func2 &&func2, T &&t, std::tuple<Ts...> &iters,
					  control_block_t &control, Cond cond) {
	deref_and_invoke_impl(std::forward<Func>(func), std::forward<Func2>(func2), 
						  std::forward<T>(t), iters, control, 
						  std::index_sequence_for<Ts...>{}, cond);
}
} // namespace detail

ENTITY_MANAGER_TEMPS
template <typename... Ts, typename Func>
void ENTITY_MANAGER_SPEC::for_each(Func && func) {
	using Typelist = meta::typelist<Ts...>;
	using ComponentsPart = meta::typelist_intersection_t<Typelist, component_t>;
	using IsFuncNoControl = std::is_constructible<
		std::function<typename detail::func_sig_no_control<entity_t, ComponentsPart>::type>,
		Func>;
	using IsFuncWithControl = std::is_constructible<
		std::function<typename detail::func_sig_with_control<entity_t, ComponentsPart>::type>,
		Func>;
	using IsFunc = meta::or_<IsFuncNoControl, IsFuncWithControl>;
	using IsTypelistUnique = meta::is_typelist_unique<Typelist>;
	using IsTypelistValid = meta::and_all<meta::typelist_has_type<Ts, comp_tag_t>...>;
	meta::eval_if(
		[&](auto) {
			auto smallestData = this->get_smallest_container<Ts...>();
			auto &smallestContainer = smallestData.first;
			auto smallestSize = smallestContainer.size();
			if (smallestSize == 0) return;
			auto iters = detail::make_iters<component_list_t, ComponentsPart>{}(components, smallestSize, maxLinearSearchDistance);
			auto key = meta::make_key<Typelist, comp_tag_t>();
			control_block_t control;
			for (const auto &ent : smallestContainer) {
				if (!smallestData.second && (ent.compTags & key) != key) continue;
				meta::for_each(iters, [&](auto &iter, std::size_t, auto) {
					if (iter.useLinear) {
						while (iter.pos->first < ent.id) ++iter.pos;
					}
					else {
						iter.pos = std::lower_bound(iter.pos, iter.end, ent.id,
													[](const auto &it, const auto &val) {
							return it.first < val;
						});
					}
				});
				detail::deref_and_invoke(func,
										 [](auto &iter) -> auto & { return iter.pos->second; },
										 ent, iters, control, IsFuncWithControl{});
				if (control.breakout) break;
			}
		},
		meta::fail_cond<IsTypelistValid>([](auto id) {
			static_assert(id(false), "for_each called with invalid typelist");
		}),
		meta::fail_cond<IsTypelistUnique>([](auto id) {
			static_assert(id(false), "for_each called with a non-unique typelist"); 
		}),
		meta::fail_cond<IsFunc>([](auto id) {
			static_assert(id(false), "for_each called with invalid callable");
		})
	);
}

ENTITY_MANAGER_TEMPS
template <typename... Ts>
entity_grouping ENTITY_MANAGER_SPEC::create_grouping() {
	using Typelist = meta::typelist<Ts...>;
	using IsTypelistUnique = meta::is_typelist_unique<Typelist>;
	using IsTypelistValid = meta::and_all<meta::typelist_has_type<Ts, comp_tag_t>...>;
	using IsGoodGrouping = std::integral_constant<bool, (sizeof...(Ts) > 1)>;
	return meta::eval_if(
		[&](auto) {
			assert(std::numeric_limits<detail::entity_grouping_id_t>::max() != currentGroupingId);
			auto emp = groupings.emplace(currentGroupingId++, 
										 std::make_pair(meta::make_key<Typelist, comp_tag_t>(),
														entity_container::from_sorted_underlying(this->get_entities<Ts...>())));
			assert(emp.second);

			return entity_grouping{*this, emp.first->first};
		},
		meta::fail_cond<IsTypelistValid>([](auto id) {
			static_assert(id(false), "create_grouping called with invalid typelist");
			return std::declval<entity_grouping>();
		}),
		meta::fail_cond<IsTypelistUnique>([](auto id) {
			static_assert(id(false), "create_grouping called with a non-unique typelist");
			return std::declval<entity_grouping>();
		}),
		meta::fail_cond<IsGoodGrouping>([](auto id) {
			static_assert(id(false), "create_grouping called with 0 or 1 components/tags");
			return std::declval<entity_grouping>();
		})
	);
}

ENTITY_MANAGER_TEMPS
template <typename... Events>
void ENTITY_MANAGER_SPEC::set_event_manager(const event_manager<component_list_t, tag_list_t, Events...> &em) {
	eventManager = &em.get_entity_event_manager();
}

#undef ENTITY_MANAGER_TEMPS
#undef ENTITY_MANAGER_SPEC
}
