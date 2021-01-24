#pragma once

#include "SAE_ECS.h"

#include <optional>

namespace sae::impl
{
	template <typename T>
	concept entity_factory = requires(T a, typename T::entity_type _entity)
	{
		typename T::entity_type;
		{ a.make() } -> std::same_as<typename T::entity_type>;
		a.recycle(_entity);
		a.reset();
	};

	extern std::optional<EntityFactory> ENTITY_FACTORY_IMPL;
	EntityFactory& get_entity_factory();

};