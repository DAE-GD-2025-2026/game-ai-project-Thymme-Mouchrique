#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;
    out.AngularVelocity = 0.f;

    const FVector2D desired = Target.Position - Agent.GetPosition();
    out.LinearVelocity = desired.IsNearlyZero() ? FVector2D::ZeroVector : desired.GetSafeNormal();

    return out;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;
    out.AngularVelocity = 0.f;

    const FVector2D desired = Agent.GetPosition() - Target.Position;
    out.LinearVelocity = desired.IsNearlyZero() ? FVector2D::ZeroVector : desired.GetSafeNormal();

    return out;
}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;
    out.AngularVelocity = 0.f;

    const FVector2D agentPos = Agent.GetPosition();
    const FVector2D targetPos = Target.Position;

    const FVector2D toTarget = targetPos - agentPos;
    const float dist = toTarget.Size();

    if (Agent.GetDebugRenderingEnabled())
    {
        if (UWorld* World = Agent.GetWorld())
        {
            const FVector agent3D(agentPos.X, agentPos.Y, 20.f);

            DrawDebugCircle(World, agent3D, m_SlowRadius, 64, FColor::Blue, false, 0.f, 0, 2.f,
                FVector(1, 0, 0), FVector(0, 1, 0), false);

            DrawDebugCircle(World, agent3D, m_TargetRadius, 64, FColor::Red, false, 0.f, 0, 2.f,
                FVector(1, 0, 0), FVector(0, 1, 0), false);
        }
    }

    if (dist <= m_TargetRadius || toTarget.IsNearlyZero())
    {
        out.LinearVelocity = FVector2D::ZeroVector;
        return out;
    }

    float speedFactor = 1.f;

    if (dist < m_SlowRadius)
    {
        speedFactor = dist / m_SlowRadius;
        speedFactor = FMath::Clamp(speedFactor, 0.05f, 1.f);
    }

    out.LinearVelocity = toTarget.GetSafeNormal() * speedFactor;
    return out;
}

SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;
    out.LinearVelocity = FVector2D::ZeroVector;

    const FVector2D toTarget = Target.Position - Agent.GetPosition();

    if (toTarget.IsNearlyZero())
    {
        out.AngularVelocity = 0.f;
        return out;
    }

    const float desiredAngle = FMath::RadiansToDegrees(FMath::Atan2(toTarget.Y, toTarget.X));
    const float currentAngle = Agent.GetRotation();
    const float deltaAngle = FMath::FindDeltaAngleDegrees(currentAngle, desiredAngle);

    out.AngularVelocity = FMath::Clamp(deltaAngle, -Agent.GetMaxAngularSpeed(), Agent.GetMaxAngularSpeed());

    return out;
}

SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;
    out.AngularVelocity = 0.f;

    const FVector2D agentPos = Agent.GetPosition();
    const FVector2D targetPos = Target.Position;

    const FVector2D toTarget = targetPos - agentPos;
    const float dist = toTarget.Size();

    const float speed = FMath::Max(Agent.GetMaxLinearSpeed(), 1.f);
    const float timeToReach = dist / speed;

    const FVector2D predictedPos = targetPos + Target.LinearVelocity * timeToReach;
    const FVector2D desired = predictedPos - agentPos;

    out.LinearVelocity = desired.IsNearlyZero() ? FVector2D::ZeroVector : desired.GetSafeNormal();

    if (Agent.GetDebugRenderingEnabled())
    {
        if (UWorld* World = Agent.GetWorld())
        {
            const FVector agent3D(agentPos.X, agentPos.Y, 20.f);
            const FVector predicted3D(predictedPos.X, predictedPos.Y, 20.f);

            DrawDebugLine(World, agent3D, predicted3D, FColor::Cyan, false, 0.f, 0, 1.5f);
            DrawDebugPoint(World, predicted3D, 8.f, FColor::Cyan, false, 0.f);
        }
    }

    return out;
}

SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.IsValid = true;
    out.AngularVelocity = 0.f;

    const FVector2D agentPos = Agent.GetPosition();
    const FVector2D targetPos = Target.Position;

    const FVector2D toTarget = targetPos - agentPos;
    const float dist = toTarget.Size();

    constexpr float EvadeRadius = 600.f;

    if (dist > EvadeRadius)
    {
        out.IsValid = false;
        out.LinearVelocity = FVector2D::ZeroVector;
        return out;
    }

    const float speed = FMath::Max(Agent.GetMaxLinearSpeed(), 1.f);
    const float timeToReach = dist / speed;

    const FVector2D predictedPos = targetPos + Target.LinearVelocity * timeToReach;
    const FVector2D desired = agentPos - predictedPos;

    out.LinearVelocity = desired.IsNearlyZero() ? FVector2D::ZeroVector : desired.GetSafeNormal();

    return out;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    const float randomDelta = FMath::FRandRange(-MaxAngleChange, MaxAngleChange);
    WanderAngle += randomDelta * DeltaT;

    const float yawRad = FMath::DegreesToRadians(Agent.GetRotation());
    const FVector2D forward(FMath::Cos(yawRad), FMath::Sin(yawRad));

    const FVector2D circleCenter = Agent.GetPosition() + forward * Offset;

    const float angleRad = FMath::DegreesToRadians(WanderAngle);
    const FVector2D circleOffset(FMath::Cos(angleRad), FMath::Sin(angleRad));

    const FVector2D wanderTarget = circleCenter + circleOffset * Radius;

    FTargetData newTarget = Target;
    newTarget.Position = wanderTarget;
    SetTarget(newTarget);

    if (Agent.GetDebugRenderingEnabled())
    {
        if (UWorld* World = Agent.GetWorld())
        {
            const FVector center3D(circleCenter.X, circleCenter.Y, 20.f);
            const FVector target3D(wanderTarget.X, wanderTarget.Y, 20.f);

            DrawDebugCircle(World, center3D, Radius, 32, FColor::Green, false, 0.f, 0, 1.5f,
                FVector(1, 0, 0), FVector(0, 1, 0), false);

            DrawDebugPoint(World, target3D, 8.f, FColor::Yellow, false, 0.f);
        }
    }

    return Seek::CalculateSteering(DeltaT, Agent);
}