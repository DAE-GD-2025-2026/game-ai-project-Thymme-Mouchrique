#pragma once
#include <vector>

#include "NavGraphPathfinding.h"

#include "../../Movement/Pathfinding/Navmesh/TriPolygon.h"

#include "../../Shared/Graph/Graph.h"
#include "../../Shared/Graph/NavGraphNode.h"


namespace GameAI
{
	class SSFA final
	{
	public:
		static std::vector<NavLine> FindPortals(std::vector<Node*> const& Path, TriPolygon const& NavPoly)
		{
			std::vector<NavLine> portals{};

			if (Path.empty())
			{
				return portals;
			}

			// Start degenerate portal
			portals.push_back(NavLine{ Path.front()->GetPosition(), Path.front()->GetPosition() });

			// Interior path nodes correspond to crossed navmesh edges
			for (int i = 1; i < static_cast<int>(Path.size()) - 1; ++i)
			{
				const auto* pNavNode = static_cast<const NavGraphNode*>(Path[i]);
				const int edgeIdx = pNavNode->GetEdgeIdx();

				if (edgeIdx < 0)
				{
					continue;
				}

				const TriPolygon::Edge& edge = NavPoly.GetEdges()[edgeIdx];
				const FVector2D edgeP1{ edge.GetP1(NavPoly) };
				const FVector2D edgeP2{ edge.GetP2(NavPoly) };

				const FVector2D prevPos = Path[i - 1]->GetPosition();
				const FVector2D nextPos = Path[i + 1]->GetPosition();
				const FVector2D travelDir = (nextPos - prevPos).GetSafeNormal();

				auto Cross = [](const FVector2D& a, const FVector2D& b) -> float
					{
						return a.X * b.Y - a.Y * b.X;
					};

				// Determine left/right relative to path direction.
				// P1 must be right point, P2 must be left point.
				const FVector2D midpoint = Path[i]->GetPosition();
				const FVector2D toP1 = edgeP1 - midpoint;

				if (Cross(travelDir, toP1) < 0.0f)
				{
					portals.push_back(NavLine{ edgeP1, edgeP2 });
				}
				else
				{
					portals.push_back(NavLine{ edgeP2, edgeP1 });
				}
			}

			// End degenerate portal
			portals.push_back(NavLine{ Path.back()->GetPosition(), Path.back()->GetPosition() });

			return portals;
		}

		static std::vector<FVector2D> OptimizePortals(std::vector<NavLine> const& Portals, TriPolygon const& NavPoly)
		{
			std::vector<FVector2D> path{};

			if (Portals.empty())
			{
				return path;
			}

			auto Cross = [](const FVector2D& a, const FVector2D& b) -> float
				{
					return a.X * b.Y - a.Y * b.X;
				};

			auto IsNearlyZero = [](const FVector2D& v) -> bool
				{
					return v.SquaredLength() <= KINDA_SMALL_NUMBER;
				};

			FVector2D apex = Portals[0].P1;
			path.push_back(apex);

			if (Portals.size() == 1)
			{
				return path;
			}

			int apexIndex = 0;
			int leftLegIndex = 1;
			int rightLegIndex = 1;

			FVector2D rightLeg = Portals[1].P1 - apex;
			FVector2D leftLeg = Portals[1].P2 - apex;

			int portalIdx = 2;
			const int portalCount = static_cast<int>(Portals.size());

			while (portalIdx < portalCount)
			{
				const NavLine& portal = Portals[portalIdx];

				// --- RIGHT CHECK ---
				{
					const FVector2D newRightLeg = portal.P1 - apex;

					// Is the right boundary moving inward? (CCW)
					if (Cross(rightLeg, newRightLeg) >= 0.0f)
					{
						// Still inside funnel or right leg degenerate
						if (IsNearlyZero(rightLeg) || Cross(newRightLeg, leftLeg) >= 0.0f)
						{
							rightLeg = newRightLeg;
							rightLegIndex = portalIdx;
						}
						else
						{
							// Crossed left leg -> left point becomes new apex
							apex += leftLeg;
							path.push_back(apex);

							apexIndex = leftLegIndex;
							portalIdx = apexIndex + 1;
							leftLegIndex = portalIdx;
							rightLegIndex = portalIdx;

							if (portalIdx < portalCount)
							{
								rightLeg = Portals[rightLegIndex].P1 - apex;
								leftLeg = Portals[leftLegIndex].P2 - apex;
							}

							continue;
						}
					}
				}

				// --- LEFT CHECK ---
				{
					const FVector2D newLeftLeg = portal.P2 - apex;

					// Is the left boundary moving inward? (CW)
					if (Cross(leftLeg, newLeftLeg) <= 0.0f)
					{
						// Still inside funnel or left leg degenerate
						if (IsNearlyZero(leftLeg) || Cross(rightLeg, newLeftLeg) >= 0.0f)
						{
							leftLeg = newLeftLeg;
							leftLegIndex = portalIdx;
						}
						else
						{
							// Crossed right leg -> right point becomes new apex
							apex += rightLeg;
							path.push_back(apex);

							apexIndex = rightLegIndex;
							portalIdx = apexIndex + 1;
							leftLegIndex = portalIdx;
							rightLegIndex = portalIdx;

							if (portalIdx < portalCount)
							{
								rightLeg = Portals[rightLegIndex].P1 - apex;
								leftLeg = Portals[leftLegIndex].P2 - apex;
							}

							continue;
						}
					}
				}

				++portalIdx;
			}

			// Add final end point
			path.push_back(Portals.back().P1);

			return path;
		}

	private:
		SSFA() = default;
		~SSFA() = default;
	};
}