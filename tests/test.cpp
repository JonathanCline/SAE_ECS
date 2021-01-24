#include <SAE_ECS.h>

struct ComponentProcBase
{
	using entity_type = sae::Entity;
	virtual void update() = 0;
	ComponentProcBase() = default;
	virtual ~ComponentProcBase() = default;
};

using AbstractProcessor = sae::AbstractComponentProcessor<ComponentProcBase>;

template <typename ComponentT>
using ComponentProcessor = sae::ComponentProcessor<ComponentProcBase, ComponentT>;

template <auto ProcID, typename ComponentT, typename... RequiredTs>
using ContractComponentProcessor = sae::ContractComponentProcessor<ProcID, ComponentProcBase, ComponentT, RequiredTs...>;

using EntityComponentSystem = sae::EntityComponentSystem<ComponentProcBase>;

int main()
{
	EntityComponentSystem _ecs{};



	return 0;
};
