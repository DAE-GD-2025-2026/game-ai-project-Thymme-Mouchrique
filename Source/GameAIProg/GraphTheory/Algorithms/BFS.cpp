#include "BFS.h"

#include <map>
#include <queue>

#include "../../Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	std::vector<Node*> path{};

	if (!pGraph || !pStartNode || !pDestinationNode)
	{
		return path;
	}

	std::queue<Node*> openList{};
	std::map<int, int> parentByNodeId{};
	std::map<int, bool> visited{};

	openList.push(pStartNode);
	visited[pStartNode->GetId()] = true;

	bool foundGoal = false;

	while (!openList.empty())
	{
		Node* const currentNode = openList.front();
		openList.pop();

		if (currentNode->GetId() == pDestinationNode->GetId())
		{
			foundGoal = true;
			break;
		}

		const std::vector<Connection*> connections = pGraph->FindConnectionsFrom(currentNode->GetId());
		for (Connection* const connection : connections)
		{
			Node* const nextNode = pGraph->GetNode(connection->GetToId()).get();
			if (!nextNode)
			{
				continue;
			}

			if (!visited[nextNode->GetId()])
			{
				visited[nextNode->GetId()] = true;
				parentByNodeId[nextNode->GetId()] = currentNode->GetId();
				openList.push(nextNode);
			}
		}
	}

	if (!foundGoal)
	{
		return path;
	}

	int currentNodeId = pDestinationNode->GetId();
	path.push_back(pGraph->GetNode(currentNodeId).get());

	while (currentNodeId != pStartNode->GetId())
	{
		currentNodeId = parentByNodeId[currentNodeId];
		path.push_back(pGraph->GetNode(currentNodeId).get());
	}

	std::reverse(path.begin(), path.end());
	return path;
}