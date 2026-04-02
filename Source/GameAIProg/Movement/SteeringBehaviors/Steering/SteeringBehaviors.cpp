#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)

SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    return Target.Position - Agent.GetPosition();
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    return Agent.GetPosition() - Target.Position;
}

SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    if (m_OriginalMaxSpeed < 0.f)
        m_OriginalMaxSpeed = Agent.GetMaxLinearSpeed();

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
        Agent.SetMaxLinearSpeed(0.f);
        out.LinearVelocity = FVector2D::ZeroVector;
        return out;
    }
    
    float newMaxSpeed = m_OriginalMaxSpeed;

    if (dist < m_SlowRadius)
    {
        newMaxSpeed = m_OriginalMaxSpeed * (dist / m_SlowRadius);
        newMaxSpeed = FMath::Max(newMaxSpeed, 5.f);
    }

    Agent.SetMaxLinearSpeed(newMaxSpeed);

    out.LinearVelocity = toTarget.GetSafeNormal();
    return out;
}

SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput out{};
    out.LinearVelocity = FVector2D::ZeroVector;
    out.IsValid = true;

    const FVector2D agentPos = Agent.GetPosition();
    const FVector2D targetPos = Target.Position;

    const FVector2D toTarget = targetPos - agentPos;

    if (toTarget.IsNearlyZero())
    {
        out.AngularVelocity = 0.f;
        return out;
    }

    const float desiredAngle = FMath::RadiansToDegrees(FMath::Atan2(toTarget.Y, toTarget.X));

    const float currentAngle = Agent.GetRotation();

    float deltaAngle = desiredAngle - currentAngle;
    deltaAngle = FMath::FindDeltaAngleDegrees(currentAngle, desiredAngle);

    const float maxAngular = Agent.GetMaxAngularSpeed();

    out.AngularVelocity = FMath::Clamp(deltaAngle, -maxAngular, maxAngular);

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

    const float pursuerSpeed = FMath::Max(Agent.GetMaxLinearSpeed(), 1.f);
    const float timeToReach = dist / pursuerSpeed;

    const FVector2D predictedPos = targetPos + Target.LinearVelocity * timeToReach;
    const FVector2D desired = predictedPos - agentPos;

    if (desired.IsNearlyZero())
        out.LinearVelocity = FVector2D::ZeroVector;
    else
        out.LinearVelocity = desired.GetSafeNormal(); 

    // Debug line
    if (Agent.GetDebugRenderingEnabled())
    {
        if (UWorld* World = Agent.GetWorld())
        {
            const FVector a3(agentPos.X, agentPos.Y, 20.f);
            const FVector p3(predictedPos.X, predictedPos.Y, 20.f);
            DrawDebugLine(World, a3, p3, FColor::Cyan, false, 0.f, 0, 1.5f);
            DrawDebugPoint(World, p3, 8.f, FColor::Cyan, false, 0.f);
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

    constexpr float evadeRadius = 600.f;          
    if (dist > evadeRadius)
    {
        out.IsValid = false;                     
        out.LinearVelocity = FVector2D::ZeroVector;
        return out;
    }

    const float evaderSpeed = FMath::Max(Agent.GetMaxLinearSpeed(), 1.f);
    const float timeToReach = dist / evaderSpeed;

    const FVector2D predictedPos = targetPos + Target.LinearVelocity * timeToReach;
    const FVector2D desired = agentPos - predictedPos;

    out.LinearVelocity = desired.IsNearlyZero() ? FVector2D::ZeroVector : desired.GetSafeNormal();
    return out;
}
SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    float randomDelta = FMath::FRandRange(-MaxAngleChange, MaxAngleChange);
    WanderAngle += randomDelta * DeltaT;
    
    float yawRad = FMath::DegreesToRadians(Agent.GetRotation());
    FVector2D forward(FMath::Cos(yawRad), FMath::Sin(yawRad));

    FVector2D circleCenter = Agent.GetPosition() + forward * Offset;

    float angleRad = FMath::DegreesToRadians(WanderAngle);
    FVector2D circleOffset(FMath::Cos(angleRad), FMath::Sin(angleRad));

    FVector2D wanderTarget = circleCenter + circleOffset * Radius;

    FTargetData newTarget = Target;
    newTarget.Position = wanderTarget;
    SetTarget(newTarget);

    if (Agent.GetDebugRenderingEnabled())
    {
        if (UWorld* World = Agent.GetWorld())
        {
            FVector center3(circleCenter.X, circleCenter.Y, 20.f);
            FVector target3(wanderTarget.X, wanderTarget.Y, 20.f);

            DrawDebugCircle(World, center3, Radius, 32, FColor::Green, false, 0.f, 0, 1.5f, FVector(1, 0, 0), FVector(0, 1, 0), false);

            DrawDebugPoint(World, target3, 8.f, FColor::Yellow, false, 0.f);
        }
    }
    return Seek::CalculateSteering(DeltaT, Agent);
}
