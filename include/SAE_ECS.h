#pragma once
#ifndef SAE_ECS_H
#define SAE_ECS_H

#include <cstdint>
#include <type_traits>
#include <vector>
#include <optional>
#include <cassert>
#include <tuple>

namespace sae
{
	
	using Entity = uint32_t;
	class EntityFactory
	{
	public:
		using entity_type = Entity;

		struct State
		{
		private:
			friend EntityFactory;
			State() = default;
			entity_type counter_ = 0;
		};
		
		entity_type make();
		void recycle(entity_type _entity);
		void reset();

		State get_state() const;
		void reset(State _state);

	private:
		entity_type entity_counter_ = 0;

	};

	constexpr static Entity NULL_ENTITY{ 0 };

	Entity new_entity();
	void recycle_entity(Entity _entity);
	void reset_entity_factory();
	EntityFactory::State get_entity_factory_state();
	void reset_entity_factory(EntityFactory::State _state);

	using ProcessorID = uint32_t;

	template <typename T>
	concept base_component_processor = true;

	template <base_component_processor ECSBaseProcessorT>
	class AbstractComponentProcessor : public ECSBaseProcessorT
	{
	public:
		using entity_type = typename ECSBaseProcessorT::entity_type;
		using base_processor_type = ECSBaseProcessorT;

		ProcessorID id() const noexcept { return this->id_; };

		virtual bool contains(entity_type _id) const = 0;
		virtual bool remove(entity_type _id) = 0;
		virtual void insert(entity_type _id) = 0;
		virtual void clear() noexcept = 0;

		virtual ~AbstractComponentProcessor() = default;

	protected:
		AbstractComponentProcessor(ProcessorID _id) noexcept :
			id_{ _id }
		{};

	private:
		const ProcessorID id_;
	};

	template <base_component_processor ECSBaseProcessorT, typename ComponentT> requires(std::is_default_constructible_v<ComponentT>)
	class ComponentProcessor : public AbstractComponentProcessor<ECSBaseProcessorT>
	{
	public:
		using component_type = ComponentT;
		using entity_type = typename AbstractComponentProcessor<ECSBaseProcessorT>::entity_type;

	private:
		struct Entry
		{
			friend bool operator==(const entity_type& _lhs, const entity_type& _rhs) noexcept
			{
				return _lhs.entity == _rhs;
			};
			entity_type entity;
			component_type data;
		};
		using container_type = std::vector<Entry>;

	public:
		using size_type = typename container_type::size_type;

		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;

		iterator begin() noexcept { return this->entries_.begin(); };
		const_iterator begin() const noexcept { return this->entries_.cbegin(); };
		const_iterator cbegin() const noexcept { return this->entries_.cbegin(); };

		iterator end() noexcept { return this->entries_.end(); };
		const_iterator end() const noexcept { return this->entries_.end(); };
		const_iterator cend() const noexcept { return this->entries_.cend(); };

		size_type size() const noexcept { return this->entries_.size(); };

		iterator find(entity_type _id) noexcept { return std::find(this->begin(), this->end(), _id); };
		const_iterator find(entity_type _id) const noexcept { return std::find(this->begin(), this->end(), _id); };

		auto& at(entity_type _id) { return *this->find(_id); };
		const auto& at(entity_type _id) const { return *this->find(_id); };

		bool contains(entity_type _id) const final { return this->find(_id) != this->end(); };
		bool remove(entity_type _id) final { return std::erase(this->entries_, _id) != 0; };
		void clear() noexcept final { this->entries_.clear(); };

		void insert(entity_type _id, const component_type& _component)
		{
			this->entries_.push_back(Entry{ std::move(_id), _component });
		};
		void insert(entity_type _id, component_type&& _component)
		{
			this->entries_.push_back(Entry{ std::move(_id), std::move(_component) });
		};

		void insert(entity_type _id) override
		{
			this->insert(std::move(_id), ComponentT{});
		};

		using AbstractComponentProcessor<ECSBaseProcessorT>::AbstractComponentProcessor;

	private:
		std::vector<Entry> entries_{};
	};

	template <ProcessorID ProcessorID, base_component_processor ECSBaseProcessorT, typename ComponentT, typename... RequireComponentTs>
	requires (std::is_default_constructible_v<ComponentT> && (std::is_default_constructible_v<RequireComponentTs> && ...))
		class ContractComponentProcessor : public ComponentProcessor<ECSBaseProcessorT, ComponentT>
	{
	private:
		using component_processor_type = ComponentProcessor<ECSBaseProcessorT, ComponentT>;

	public:
		using entity_type = typename component_processor_type::entity_type;
		using component_type = typename component_processor_type::component_type;

	private:
		using tuple_type = std::tuple<ComponentProcessor<ECSBaseProcessorT, RequireComponentTs>*...>;

	protected:
		template <typename T> requires(std::is_same_v<T, RequireComponentTs> || ...)
			ComponentProcessor<ECSBaseProcessorT, T>* get_contract_processor() const noexcept
		{
			return std::get<ComponentProcessor<ECSBaseProcessorT, T>*>(this->contract_processors_);
		};

	public:
		bool has_all_required_components(entity_type _entity) const
		{
			auto _out = (this->get_contract_processor<RequireComponentTs>()->contains(_entity) && ...);
			return _out;
		};

		template <typename T> requires(std::is_same_v<T, RequireComponentTs> || ...)
			bool has_required_component(entity_type _entity) const
		{
			return this->get_contract_processor<T>()->contains(_entity);
		};

	private:
		template <typename T> requires(std::is_same_v<T, RequireComponentTs> || ...)
			void add_required_components_helper(entity_type _entity) const
		{
			if (!this->has_required_component<T>(_entity))
			{
				this->get_contract_processor<T>()->insert(_entity);
			};
		};

	public:

		void add_required_components(entity_type _entity) const
		{
			(add_required_components_helper<RequireComponentTs>(_entity), ...);
		};

		void insert(entity_type _entity, const component_type& _component)
		{
			assert(this->has_required_components(_entity));
			component_processor_type::insert(std::move(_entity), _component);
		};
		void insert(entity_type _entity, component_type&& _component)
		{
			assert(this->has_all_required_components(_entity));
			component_processor_type::insert(std::move(_entity), std::move(_component));
		};

		void insert(entity_type _entity) override
		{
			this->insert(std::move(_entity), ComponentT{});
		};

		ContractComponentProcessor(ComponentProcessor<ECSBaseProcessorT, RequireComponentTs>&... _contractProcessors) :
			ComponentProcessor<ECSBaseProcessorT, ComponentT>{ ProcessorID }, contract_processors_{ &_contractProcessors... }
		{};

	private:
		tuple_type contract_processors_{};

	};

	template <base_component_processor ECSBaseProcessorT>
	class EntityComponentSystem
	{
	private:
		class Entry
		{
		public:
			auto processor_id() const noexcept { return this->id_; };

			auto* get() const noexcept { return this->processor_; }
			auto* operator->() const noexcept { return this->get(); };

			bool operator==(const ProcessorID& _id) const noexcept { return this->processor_id() == _id; };
			bool operator!=(const ProcessorID& _id) const noexcept { return this->processor_id() != _id; };

			bool operator==(const AbstractComponentProcessor<ECSBaseProcessorT>*& _ptr) const noexcept { return this->get() == _ptr; };
			bool operator!=(const AbstractComponentProcessor<ECSBaseProcessorT>*& _ptr) const noexcept { return this->get() != _ptr; };

			Entry(AbstractComponentProcessor<ECSBaseProcessorT>* _ptr) :
				processor_{ std::move(_ptr) }, id_{ this->get()->id() }
			{};

		private:
			AbstractComponentProcessor<ECSBaseProcessorT>* processor_ = nullptr;
			ProcessorID id_;
		};
		using container_type = std::vector<Entry>;

	public:
		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;

		iterator begin() noexcept { return this->processors_.begin(); };
		const_iterator begin() const noexcept { return this->processors_.cbegin(); };
		const_iterator cbegin() const noexcept { return this->processors_.cbegin(); };

		iterator end() noexcept { return this->processors_.end(); };
		const_iterator end() const noexcept { return this->processors_.end(); };
		const_iterator cend() const noexcept { return this->processors_.cend(); };

		iterator find(ProcessorID _id)
		{
			return std::find(this->begin(), this->end(), _id);
		};
		const_iterator find(ProcessorID _id) const
		{
			return std::find(this->begin(), this->end(), _id);
		};

		size_t size() const noexcept
		{
			return this->processors_.size();
		};

		bool contains(ProcessorID _id) const
		{
			return this->find(std::move(_id)) != this->end();
		};

		ProcessorID insert(AbstractComponentProcessor<ECSBaseProcessorT>* _processor)
		{
			this->processors_.push_back(std::move(_processor));
		};
		auto* at(ProcessorID _id) const
		{
			return this->find(_id)->get();
		};

		void clear() noexcept
		{
			this->processors_.clear();
		};

	private:
		std::vector<Entry> processors_{};

	};


};

#endif