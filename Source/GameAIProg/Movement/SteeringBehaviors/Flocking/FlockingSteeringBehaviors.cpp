#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"

SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    if (pFlock->GetNrOfNeighbors() == 0)
        return SteeringOutput{ FVector2D::ZeroVector, 0.f };

    FVector2D avgPos = pFlock->GetAverageNeighborPos();
    FTargetData target;
    target.Position = avgPos;

    this->SetTarget(target);
    return Seek::CalculateSteering(deltaT, Agent);
}

SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;

    const int count = pFlock->GetNrOfNeighbors();
    if (count == 0)
    {
        out.LinearVelocity = FVector2D::ZeroVector;
        return out;
    }

    FVector2D agentPos = Agent.GetPosition();
    FVector2D force = FVector2D::ZeroVector;

    for (int i = 0; i < count; i++)
    {
        ASteeringAgent* neighbor = pFlock->GetNeighbors()[i];
        FVector2D nPos = neighbor->GetPosition();

        FVector2D toAgent = agentPos - nPos;
        float dist = toAgent.Size();

        if (dist > 0.0001f)
            force += toAgent / (dist * dist);
    }

    out.LinearVelocity = force.GetSafeNormal();
    return out;
}

SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;

    const int count = pFlock->GetNrOfNeighbors();
    if (count == 0)
    {
        out.LinearVelocity = FVector2D::ZeroVector;
        return out;
    }

    FVector2D avgVel = pFlock->GetAverageNeighborVelocity();
    FVector2D myVel = Agent.GetLinearVelocity();

    out.LinearVelocity = (avgVel - myVel).GetSafeNormal();
    return out;
}
