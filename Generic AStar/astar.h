// Mason Kaschube (mason.kaschube)
// Cs381    Spring 2024
// Generic A* Pathfinding

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>
#ifndef ASTAR
#define ASTAR

//callback object for Astar
template <typename GraphType, typename AstarType>
class Callback {
    protected:
        GraphType const& g;
    public:
        Callback( GraphType const& _g) : g(_g) {}
        virtual ~Callback() {}
        virtual void OnIteration( AstarType const& ) { }
        virtual void OnFinish( AstarType const& )    { }
};

// struct for Nodes that are stored in our container
template <typename GraphType, typename Heuristic>
struct Node
{
    typename Heuristic::ReturnType _heuristic;
    Node* _parent;
    typename GraphType::Vertex _currNode;
    float _costToStart;

    Node() : 
        _heuristic(0), 
        _parent(NULL), 
        _currNode(0), 
        _costToStart(0)
    {}

    Node(const typename Heuristic::ReturnType h, 
         Node* p, 
         typename GraphType::Vertex cn, 
         float cts) : 
        _heuristic(h),
        _parent(p),
        _currNode(cn),
        _costToStart(cts)
    {}

    // operator overloads for given node and a node ID
    bool operator<(const Node& b) const
    {
        return _costToStart + _heuristic < b._costToStart + b._heuristic;
    }

    bool operator<(const size_t& id) const
    {
        return _currNode.ID() < id;
    }

    // more operator overloads for comparing node IDs
    bool operator!=(const size_t& id) const
    {
        return _currNode.ID() != id;
    }

    bool operator!=(const Node& b) const
    {
        return _currNode.ID() != b._currNode.ID();
    }

    // less-than method for predicate function
    struct less_than{
        bool operator()(Node* a, Node* b)
        {
            return (a->_costToStart + a->_heuristic) < (b->_costToStart + b->_heuristic);
        }
    };

    // greater-than method for predicate function
    struct greater_than{
        bool operator()(Node* a, Node* b)
        {
            return (a->_costToStart + a->_heuristic) > (b->_costToStart + b->_heuristic);
        }
    };

    // iterator's predicate function for finding an element 
    struct find_id : std::unary_function<Node<GraphType, Heuristic>, bool>
    {
        size_t id;
        find_id(size_t id):id(id) {}
        bool operator()(Node<GraphType, Heuristic>* n) const
        {
            return n->_currNode.ID() == id;
        }
    };
};

// wrapper struct for multimap 
template <typename GraphType, typename Heuristic>
struct Container
{
    // readability sake
    typedef Node<GraphType, Heuristic>* NODE;

    Container() : 
        list()
    {}

    // cleanup
    void clear()
    {
        // iterate through each key
        typename std::map<size_t, std::vector<NODE>>::iterator it = list.begin();
        while (it != list.end())
        {
            // if we have a vector, delete all elements and move on
            if (!(*it).second.empty())
            {
                for (unsigned i = 0; i < (*it).second.size(); i++)
                {
                    delete (*it).second[i];
                }
                (*it).second.clear();
            }
            std::advance(it, 1);
        }
        // cleanup the list
        list.clear();
    }

    // access an element in the list with a key
    std::vector<NODE> operator[](const unsigned key)
    {
        return list[key];
    }

    // return the cheapest node
    NODE cheapest()
    {
        //typename std::map<size_t, std::vector<NODE>>::iterator it = list.begin();
        // if our list is empty or the vector at the key is empty, return nothing
        if (list.empty() || (*list.begin()).second.empty())
            return NULL;

        // grab the first element of our vector
        NODE res = (*list.begin()).second[0];

        // if it doesn't exist, don't return anything
        if (!res)
            return NULL;

        // remove the element from the vector
        (*list.begin()).second.erase((*list.begin()).second.begin());

        // if our vector is now empty
        if ((*list.begin()).second.empty())
        {
            // erase this key from the map
            list.erase(list.begin());
        }

        // return our cheapest node
        return res;
    }

    // find and take a node from the list
    NODE get(size_t currCost, NODE n)
    {
        // can't find a node if there is no list
        if (list[currCost].empty())
            return NULL;

        // grab an iterator pointing to the node in our current key
        auto it = std::find_if(list[currCost].begin(), list[currCost].end(), typename Node<GraphType, Heuristic>::find_id(n->_currNode.ID()));

        // if we don't have that node in this key, return nothing
        if (it == list[currCost].end())
            return NULL;

        // remove from the vector and return
        NODE temp = *it;
        erase(currCost, n);
        return temp;
    }

    // find but don't erase a node from the list
    NODE find(size_t id)
    {
        // grab an iterator to our list
        typename std::map<size_t, std::vector<NODE>>::iterator it = list.begin();

        // if there isn't anything in the map, return nothing
        if (it == list.end())
            return NULL;

        // loop through each key in the map
        while (it != list.end())
        {
            // try to find the node in each vector
            auto itVec = std::find_if((*it).second.begin(), (*it).second.end(), typename Node<GraphType, Heuristic>::find_id(id));

            // if we find the node, return it
            if (itVec != (*it).second.end())
                return (*itVec);

            // otherwise, advance our iterator
            std::advance(it, 1);
        }
        // we didn't find the node in the list
        return NULL;
    }

    // add a node to the list
    void insert(NODE n)
    {
        // if we already have a node with this same cost
        if (list.find(n->_costToStart + n->_heuristic) != list.end())
        {
            // add the new node to the front of the vector
            list[n->_costToStart + n->_heuristic].insert(list[n->_costToStart + n->_heuristic].begin(), n);
        }
        // if we don't already have a node with this same cost
        else
        {
            // add a new element for it and add our node
            list.emplace( std::pair<size_t, std::vector<NODE>>{n->_costToStart + n->_heuristic, std::vector<NODE>() } );
            list[n->_costToStart + n->_heuristic].push_back(n);
        }
    }

    // return an iterator to the beginning of our list
    typename std::map<size_t, std::vector<NODE>>::iterator begin()
    {
        return list.begin();
    }

    // return an iterator to the end of our list
    typename std::map<size_t, std::vector<NODE>>::iterator end()
    {
        return list.end();
    }

    // return the size of our map
    unsigned size()
    {
        return list.size();
    }

    // erase an element from the list
    void erase(size_t currCost, NODE n)
    {
        // grab an iterator to the given node in the current key
        auto itVec = std::find_if(list[currCost].begin(), list[currCost].end(), typename Node<GraphType, Heuristic>::find_id(n->_currNode.ID()));

        // if there is no node in this list, we can't erase it
        if (itVec == list[currCost].end())
            return;
        // if there is a node, remove it from the vector
        else
            list[currCost].erase(itVec);

        // if this list is now empty, remove the element from the map
        if (list[currCost].size() == 0)
        {
            list.erase(currCost);
        }
    }

    // map of nodes sorted by F cost
    std::map<size_t, std::vector<NODE>> list;
};

template <typename GraphType, typename Heuristic>
class Astar {
    public:
        ////////////////////////////////////////////////////////////
        Astar( GraphType const& _graph, Callback<GraphType,Astar> & cb ) : 
            graph(_graph),
            callback(cb),
            openlist(),
            openListDict(),
            closedlist(),
            solution(),
            start_id(0),
            goal_id(0)
        {}
        ////////////////////////////////////////////////////////////
        std::vector<typename GraphType::Edge> search(size_t s, size_t g) 
        {
            start_id = s;
            goal_id  = g;
            openlist.clear();
            openListDict.clear();
            closedlist.clear();
            solution.clear();
            Heuristic heuristic;
            //heuristic from start to goal
            typename Heuristic::ReturnType h = heuristic( graph,graph.GetVertex(start_id),graph.GetVertex(goal_id) );

            // typedef the Node data type for readability sake
            typedef Node<GraphType, Heuristic> NODE;

            // add the starting node to the open list
            NODE* startNode = new NODE(h, NULL, graph.GetVertex(start_id), 0);
            AddToOpen(startNode);

            while ( openlist.size() > 0 ) 
            {
                callback.OnIteration( *this );

                // grab the cheapest node from our open list and add it to our closed list
                NODE* currNode = openlist.cheapest();
                AddToClosed(currNode);

                // grab the children of our current node
                std::vector<typename GraphType::Edge> const& edges = graph.GetOutEdges(currNode->_currNode);
                
                if (currNode->_currNode.ID() != goal_id)
                {
                    // loop through all the edges to this current vertex
                    for(unsigned i = 0; i < edges.size(); i++)
                    {
                        // calculate this child's heuristic to the goal
                        typename Heuristic::ReturnType newHeuristic = heuristic(graph, graph.GetVertex(edges[i].GetID2()), graph.GetVertex(goal_id));

                        // grab the ID of our current child
                        size_t currID = edges[i].GetID2();
                        NODE* newNode = NULL;

                        // try to find the child on the open and closed lists
                        NODE* openResult = FindOnOpen(currID);
                        typename std::vector<NODE*>::iterator closedResult = FindOnClosed(currID);
                        
                        // if this child isn't on either list
                        if (openResult == NULL && closedResult == closedlist.end())
                        {
                            // create a new node for it and add it to the open list
                            newNode = new NODE(newHeuristic, currNode, graph.GetVertex(currID), currNode->_costToStart + edges[i].GetWeight());
                            AddToOpen(newNode);
                            continue;
                        }

                        // if this node is already on the open list but this path is cheaper, replace it
                        if (openResult != NULL)
                        {
                            if (openResult->_costToStart + openResult->_heuristic > currNode->_costToStart + edges[i].GetWeight() + newHeuristic)
                            {
                                // remove the node from the open list, modify the parent and cost and add it back to the open list
                                newNode = RemoveOnOpen(openResult->_costToStart + openResult->_heuristic, openResult);
                                newNode->_parent = currNode;
                                newNode->_costToStart = currNode->_costToStart + edges[i].GetWeight();
                                AddToOpen(newNode);
                            }
                            continue;
                        }

                        // if this node is already on the closed list but this path is cheaper, add it to the open list
                        if (closedResult != closedlist.end())
                        {
                            if ((*closedResult)->_costToStart + (*closedResult)->_heuristic > currNode->_costToStart + edges[i].GetWeight() + newHeuristic)
                            {
                                // grab the node off the closed list, change its parent and cost, and add it back to the open list
                                newNode = RemoveOnClosed(closedResult);
                                newNode->_parent = currNode;
                                newNode->_costToStart = currNode->_costToStart + edges[i].GetWeight();
                                AddToOpen(newNode);
                            }
                            continue;
                        }
                    }
                }

                // if this node is the goal
                if (currNode->_currNode.ID() == goal_id)
                {
                    // start backtracking on our solution path
                    NODE* backtrack = currNode;
                    while (backtrack->_parent != NULL)
                    {
                        // check just in case our data got messed up
                        bool validPath = false;
                        std::vector<typename GraphType::Edge> outE = graph.GetOutEdges(backtrack->_currNode.ID());
                        // find the edge that connects this node to the parent
                        for (unsigned i = 0; i < outE.size(); i++)
                        {
                            if (outE[i].GetID2() == backtrack->_parent->_currNode.ID())
                            {
                                // add the edge to the solution path
                                solution.push_back(outE[i]);
                                backtrack = backtrack->_parent;
                                validPath = true;
                                break;
                            }
                        }

                        // quit out if we couldn't create a valid path
                        if (!validPath)
                        {
                            break;
                        }
                    }

                    // cleanup the open and closed lists
                    openlist.clear();

                    for (typename std::vector<Node<GraphType, Heuristic>*>::const_iterator it = closedlist.begin(); it != closedlist.end(); it++)
                    {
                        delete *it;
                    }
                    closedlist.clear();
                }
            }

            callback.OnFinish( *this );
            return solution;
        }
        ////////////////////////////////////////////////////////////////////////
    public:
        const GraphType &            graph;
        Callback<GraphType,Astar>  & callback;

        // typedef for our open, closed, and solution containers
        typedef Container<GraphType, Heuristic> OpenListContainer;
        typedef std::vector<Node<GraphType, Heuristic>*> ClosedListContainer;
        typedef std::vector<typename GraphType::Edge> SolutionContainer;
        
        OpenListContainer            openlist;
        Container<GraphType, Heuristic> openListDict;
        ClosedListContainer          closedlist;
        SolutionContainer            solution;
        size_t                       start_id,goal_id;


        //////////////////////////////////////////////////////////////////////////////////
        //                             Helper Functions                                 //
        //////////////////////////////////////////////////////////////////////////////////

        // find a node in the open lsit
        bool InOpenList(size_t id)
        {
            auto it = std::find_if(openlist.begin(), openlist.end(), typename Node<GraphType, Heuristic>::find_id(id));
            if (it == openlist.end())
            {
                return false;
            }
            return true;
        }

        // add a node to the open list
        bool AddToOpen(Node<GraphType, Heuristic>* n)
        {
            openlist.insert(n);
            return true;
        }

        // Remove a node off the open list
        Node<GraphType, Heuristic>* RemoveOnOpen(size_t currCost, Node<GraphType, Heuristic>* n)
        {
            // only try to remove if the vector for the key isn't empty
            if (!openlist[currCost].empty())
            {
                return openlist.get(currCost, n);
            }
            return NULL;
        }

        // find a node on the open list (but don't remove it)
        Node<GraphType, Heuristic>* FindOnOpen(size_t id)
        {
            return openlist.find(id);
        }

        // check if a node is on the closed list
        bool InClosedList(size_t id)
        {
            // I know auto is bad but it's more readable than typing out the entire iterator variable
            auto it = std::find_if(closedlist.begin(), closedlist.end(), typename Node<GraphType, Heuristic>::find_id(id));
            if (it == closedlist.end())
            {
                return false;
            }
            return true;
        }

        // add a node to the closed list
        bool AddToClosed(Node<GraphType, Heuristic>* n)
        {
            // don't add the node if we already have it
            if (InClosedList(n->_currNode.ID()))
            {
                return true;
            }
            // add the node to the closed list in whatever order
            closedlist.push_back(n);
            return true;
        }

        // remove a given node from the closed list
        Node<GraphType, Heuristic>* RemoveOnClosed(typename std::vector<Node<GraphType, Heuristic>*>::iterator it)
        {
            // if the iterator is valid, remove the node it points to and return it
            if (it != closedlist.end())
            {
                Node<GraphType, Heuristic>* solution = *it;
                closedlist.erase(it);
                return solution;
            }
            return NULL;
        }

        // find a node on the closed list (but don't remove it)
        typename std::vector<Node<GraphType, Heuristic>*>::iterator FindOnClosed(size_t id)
        {
            // again, auto bad don't use, but it's better looking than the actual type. 
            auto it = std::find_if(closedlist.begin(), closedlist.end(), typename Node<GraphType, Heuristic>::find_id(id));
            if (it == closedlist.end())
            {
                return closedlist.end();
            }
            return it;
        }
};

#endif
