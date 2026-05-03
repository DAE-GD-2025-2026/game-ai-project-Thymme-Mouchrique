#include "PathFollowSteeringBehavior.h"
#include "../SteeringAgent.h"

PathFollow::PathFollow()
{
	pSeek = new Seek();
	pArrive = new Arrive();
	pArrive->SetTargetRadius(10.0f);
}

PathFollow::~PathFollow()
{
	delete pArrive;
	delete pSeek;
}

void PathFollow::SetPath(std::vector<FVector2D>& path)
{
	Reset(); 

	pathVec = path;

	if (!pathVec.empty())
	{
		GotoNextPathPoint();
	}
}
void PathFollow::Reset()
{
	currentPathIndex = -1;
	pathVec.clear();

	pCurrentSteering = nullptr;

	pSeek->SetTarget(FTargetData{});
	pArrive->SetTarget(FTargetData{});
}
SteeringOutput PathFollow::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	if (currentPathIndex < static_cast<int>(pathVec.size()))
	{
		float agentRadius = Agent.GetCapsuleRadius();
		FVector2D ToPathPoint{pathVec[currentPathIndex] - Agent.GetPosition()};
		
		if (ToPathPoint.SizeSquared() < agentRadius * agentRadius)
		{
			GotoNextPathPoint();
		}
	}

	if (pCurrentSteering != nullptr)
	{
		return pCurrentSteering->CalculateSteering(DeltaTime, Agent);
	}
	return SteeringOutput{};
}

void PathFollow::GotoNextPathPoint()
{
	++currentPathIndex;

	if (currentPathIndex >= static_cast<int>(pathVec.size()))
	{
		pCurrentSteering = nullptr;
		return;
	}

	FTargetData PathTarget{ pathVec[currentPathIndex] };

	if (currentPathIndex == static_cast<int>(pathVec.size()) - 1)
	{
		pArrive->SetTarget(PathTarget);
		pCurrentSteering = pArrive;
	}
	else
	{
		pSeek->SetTarget(PathTarget);
		pCurrentSteering = pSeek;
	}
}