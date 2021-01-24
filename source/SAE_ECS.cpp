#include "SAE_ECS.h"

namespace sae
{
	EntityFactory::entity_type EntityFactory::make()
	{
		return ++this->entity_counter_;
	};
	void EntityFactory::recycle(entity_type _entity)
	{

	};
	void EntityFactory::reset()
	{
		this->entity_counter_ = 0;
	};

	EntityFactory::State EntityFactory::get_state() const
	{
		State _out{};
		_out.counter_ = this->entity_counter_;
		return _out;
	};
	void EntityFactory::reset(State _state)
	{
		this->entity_counter_ = _state.counter_;
	};

	namespace impl
	{
		std::optional<EntityFactory> ENTITY_FACTORY_IMPL{ std::nullopt };
		EntityFactory& get_entity_factory()
		{
			return (impl::ENTITY_FACTORY_IMPL.has_value()) ? impl::ENTITY_FACTORY_IMPL.value() : impl::ENTITY_FACTORY_IMPL.emplace();
		};
	};

	Entity new_entity()
	{
		return impl::get_entity_factory().make();
	};
	void recycle_entity(Entity _entity)
	{
		impl::get_entity_factory().recycle(std::move(_entity));
	};
	void reset_entity_factory()
	{
		impl::get_entity_factory().reset();
	};
	EntityFactory::State get_entity_factory_state()
	{
		return impl::get_entity_factory().get_state();
	};
	void reset_entity_factory(EntityFactory::State _state)
	{
		impl::get_entity_factory().reset(std::move(_state));
	};


}

namespace sae
{
	

}
