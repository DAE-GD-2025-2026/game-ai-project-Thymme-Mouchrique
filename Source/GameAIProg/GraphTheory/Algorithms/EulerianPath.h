#pragma once
#include <stack>
#include "../../Shared/Graph/Graph.h"

namespace GameAI
{
    enum class Eulerianity
    {
        notEulerian,
        semiEulerian,
        eulerian,
    };

    class EulerianPath final
    {
    public:
        EulerianPath(Graph* const pGraph);

        Eulerianity IsEulerian() const;
        std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

    private:
        void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
        bool IsConnected() const;

        Graph* m_pGraph;
    };

    inline EulerianPath::EulerianPath(Graph* const pGraph)
        : m_pGraph(pGraph)
    {
    }

    inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& nodes, std::vector<bool>& visited, int startIndex) const
    {
        visited[startIndex] = true;
        int id = nodes[startIndex]->GetId();
        auto conns = m_pGraph->FindConnectionsFrom(id);

        for (auto c : conns)
        {
            int nextId = c->GetToId();
            for (int i = 0; i < nodes.size(); ++i)
            {
                if (nodes[i]->GetId() == nextId && !visited[i])
                    VisitAllNodesDFS(nodes, visited, i);
            }
        }
    }

    inline bool EulerianPath::IsConnected() const
    {
        auto nodes = m_pGraph->GetActiveNodes();
        if (nodes.empty())
            return false;

        int startIndex = -1;
        for (int i = 0; i < nodes.size(); ++i)
        {
            if (!m_pGraph->FindConnectionsFrom(nodes[i]->GetId()).empty())
            {
                startIndex = i;
                break;
            }
        }

        if (startIndex == -1)
            return true;

        std::vector<bool> visited(nodes.size(), false);
        VisitAllNodesDFS(nodes, visited, startIndex);

        for (int i = 0; i < nodes.size(); ++i)
        {
            bool hasEdges =
                !m_pGraph->FindConnectionsFrom(nodes[i]->GetId()).empty() ||
                !m_pGraph->FindConnectionsTo(nodes[i]->GetId()).empty();

            if (hasEdges && !visited[i])
                return false;
        }

        return true;
    }

    inline Eulerianity EulerianPath::IsEulerian() const
    {
        if (!IsConnected())
            return Eulerianity::notEulerian;

        auto nodes = m_pGraph->GetActiveNodes();
        int odd = 0;

        for (auto n : nodes)
        {
            int id = n->GetId();
            int degree =
                m_pGraph->FindConnectionsFrom(id).size() +
                m_pGraph->FindConnectionsTo(id).size();

            if (degree % 2 != 0)
                odd++;
        }

        if (odd > 2)
            return Eulerianity::notEulerian;
        if (odd == 2)
            return Eulerianity::semiEulerian;
        return Eulerianity::eulerian;
    }

    inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
    {
        eulerianity = IsEulerian();
        if (eulerianity == Eulerianity::notEulerian)
            return {};

        Graph graphCopy = m_pGraph->Clone();
        auto nodesCopy = graphCopy.GetActiveNodes();
        std::vector<Node*> path;
        std::stack<int> nodeStack;

        int startId = -1;
        if (eulerianity == Eulerianity::semiEulerian)
        {
            for (auto n : nodesCopy)
            {
                int id = n->GetId();
                int degree =
                    graphCopy.FindConnectionsFrom(id).size() +
                    graphCopy.FindConnectionsTo(id).size();

                if (degree % 2 != 0)
                {
                    startId = id;
                    break;
                }
            }
        }
        else
        {
            startId = nodesCopy[0]->GetId();
        }

        int current = startId;

        while (true)
        {
            auto conns = graphCopy.FindConnectionsFrom(current);
            if (!conns.empty())
            {
                nodeStack.push(current);
                int next = conns[0]->GetToId();
                graphCopy.RemoveConnection(current, next);
                current = next;
            }
            else
            {
                path.push_back(m_pGraph->GetNode(current).get());
                if (nodeStack.empty())
                    break;
                current = nodeStack.top();
                nodeStack.pop();
            }
        }

        std::reverse(path.begin(), path.end());
        return path;
    }
}
