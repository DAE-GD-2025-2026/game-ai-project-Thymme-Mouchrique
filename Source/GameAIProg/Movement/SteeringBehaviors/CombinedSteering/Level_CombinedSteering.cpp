#include "Level_CombinedSteering.h"

#include "imgui.h"

ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	// Spawn the wanderer (target to evade)
	WandererAgent = GetWorld()->SpawnActor<ASteeringAgent>(
		SteeringAgentClass, FVector{ 600.f, 0.f, 90.f }, FRotator::ZeroRotator);

	// Spawn the combined steering agent
	CombinedAgent = GetWorld()->SpawnActor<ASteeringAgent>(
		SteeringAgentClass, FVector{ 0.f, 0.f, 90.f }, FRotator::ZeroRotator);

	if (!IsValid(WandererAgent) || !IsValid(CombinedAgent))
		return;

	// Wanderer: just wander around
	WandererWander = std::make_unique<Wander>();
	WandererAgent->SetSteeringBehavior(WandererWander.get());

	// Combined agent: build behaviors
	SeekBehavior = std::make_unique<Seek>();
	WanderBehavior = std::make_unique<Wander>();
	EvadeBehavior = std::make_unique<Evade>();

	// Blended = Seek + Wander (weights adjustable in UI)
	Blended = std::make_unique<BlendedSteering>(
		std::vector<BlendedSteering::WeightedBehavior>
	{
		{ SeekBehavior.get(), 0.5f },
		{ WanderBehavior.get(), 0.5f }
	}
	);

	// Priority = Evade first, else Blended
	Priority = std::make_unique<PrioritySteering>(
		std::vector<ISteeringBehavior*>
	{
		EvadeBehavior.get(),
			Blended.get()
	}
	);

	CombinedAgent->SetSteeringBehavior(Priority.get());

	// Initial debug state
	WandererAgent->SetDebugRenderingEnabled(CanDebugRender);
	CombinedAgent->SetDebugRenderingEnabled(CanDebugRender);

	// Initial targets
	SeekBehavior->SetTarget(MouseTarget);
	EvadeBehavior->SetTarget(MakeTargetFromAgent(WandererAgent));
}

void ALevel_CombinedSteering::BeginDestroy()
{
	// release in "top-down" order
	Priority.reset();
	Blended.reset();

	EvadeBehavior.reset();
	WanderBehavior.reset();
	SeekBehavior.reset();

	WandererWander.reset();

	Super::BeginDestroy();
}

FTargetData ALevel_CombinedSteering::MakeTargetFromAgent(ASteeringAgent* Agent) const
{
	FTargetData t{};
	if (!IsValid(Agent))
		return t;

	t.Position = Agent->GetPosition();
	t.Orientation = Agent->GetRotation();
	t.LinearVelocity = Agent->GetLinearVelocity();
	t.AngularVelocity = Agent->GetAngularVelocity();
	return t;
}

void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#pragma region UI
	{
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("COMBINED STEERING", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
			if (IsValid(CombinedAgent)) CombinedAgent->SetDebugRenderingEnabled(CanDebugRender);
			if (IsValid(WandererAgent)) WandererAgent->SetDebugRenderingEnabled(CanDebugRender);
		}

		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		if (Blended)
		{
			auto& weights = Blended->GetWeightedBehaviorsRef();
			if (weights.size() >= 2)
			{
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
					weights[0].Weight, 0.f, 1.f,
					[&](float v) { weights[0].Weight = v; }, "%.2f");

				ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
					weights[1].Weight, 0.f, 1.f,
					[&](float v) { weights[1].Weight = v; }, "%.2f");
			}
		}

		ImGui::Spacing();
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Evade Radius",
			EvadeRadius, 0.f, 2000.f,
			[this](float v) { EvadeRadius = v; }, "%.0f");

		ImGui::End();
	}
#pragma endregion

	// --- Combined Steering Update ---
	if (!IsValid(CombinedAgent) || !IsValid(WandererAgent))
		return;

	// Seek uses the mouse target (MouseTarget gets updated by the BP on click in this template)
	if (SeekBehavior)
		SeekBehavior->SetTarget(MouseTarget);

	if (EvadeBehavior)
	{
		FTargetData t = MakeTargetFromAgent(WandererAgent);

		const FVector2D delta = t.Position - CombinedAgent->GetPosition();
		const float distSq = delta.SizeSquared();

		if (distSq > EvadeRadius * EvadeRadius)
		{
			SteeringOutput forceInvalid{};
			forceInvalid.IsValid = false;

			t.Position = CombinedAgent->GetPosition();
			t.LinearVelocity = FVector2D::ZeroVector;
		}

		EvadeBehavior->SetTarget(t);
	}
}