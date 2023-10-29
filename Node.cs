// Node.cs
// Mason Kaschube
// 10/01/2023
// 
// This class acts as the parent class for all nodes in the behavior tree

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace KBT
{
    // possible node states for behaivor tree
    public enum NodeState
    {
        RUNNING,
        SUCCESS,
        FAILURE
    }

    public abstract class Node
    {
        protected NodeState state;                              // current state of the node

        public Node parent;                                     // this node's parent
        protected List<Node> children = new List<Node>();       // this node's children

        public Tree parentTree;                                 // the parent tree this node resides in


        // default constructor
        public Node()
        {
            parent = null;
        }

        // constructor for children
        public Node(List<Node> children)
        {
            foreach (Node child in children)
                Attach(child);
        }

        // get the current node's state
        public NodeState GetState() 
		{ 
			return state; 
		}
		
		// set a Node's state
        public void SetState(NodeState _state)
        {
            state = _state;
        }

        // attach node to the tree
        protected void Attach(Node node)
        {
            node.parent = this;
            children.Add(node);
        }

        // Runs the node with support for entering and exiting logic
        public NodeState Execute()
        {
            NodeEnter();
            var result = Evaluate();
            NodeExit();
            return result;
        }

        // Enter logic
        public virtual void NodeEnter()
        {
            return;
        }

        // Node body logic
        public virtual NodeState Evaluate()
        {
            return state = NodeState.FAILURE;
        }

        // Exit logic
        public virtual void NodeExit()
        {
            // prepare this node for the next iteration of the tree
            if (state == NodeState.SUCCESS)
                ResetNode();
            return;
        }

		// Cleanup logic for if this node gets interrupted
        public virtual void NodeInterrupted()
        {
            return;
        }

        // recursive function to fill important information for each node in the tree
        public void FillInformation(Tree _parentTree)
        {
			// assign a reference to the tree this node is a child of
            parentTree = _parentTree;
            state = NodeState.RUNNING;
			
			// call this function for each child
            if (children.Count != 0)
            {
                foreach (Node child in children)
                    child.FillInformation(parentTree);
            }
        }

        // reset the current node's values for when the tree restarts
        public virtual void ResetNode()
        {
            return;
        }

        // recursive function to reset every node's values for the tree's next iteration
        public void ResetTree()
        {
			// reset values for next iteration
            ResetNode();
            state = NodeState.RUNNING;
            foreach (Node child in children)
            {
                child.ResetTree();
            }
        }
    }
}

