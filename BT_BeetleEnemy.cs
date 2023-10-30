using KBT;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;
using UnityEngine.Events;

public class BT_BeetleEnemy : Tree
{
    private GameObject player;                  // player reference
    private GameObject self;                    // self reference
    private NavMeshAgent agent;                 // our navmesh agent
    private Vector3 goalPos;                    // where we want to go to
    private List<UnityEvent> eventsToCall;      // what events do we call
    public float dist;                          // how close to the player before we charge
    public Vector2 clampVals;                   // clamp for turn speed values
    public float speed;                         // how far should we go past the player when we charge
    public List<float> waitTimes;               // wait times for each wait node
    public List<float> waitTimesDeviation;      // wait random deviations for each wait node
    public float moveSpeed;                     // how fast do we actually charge the player

    protected override void SetupBlackboard(Node root)
    {
        // Add data you want to be accessible to all nodes
        blackboard.SetVar(player, "player");
        blackboard.SetVar(self, "self");
        blackboard.SetVar(agent, "agent");
        blackboard.SetVar(goalPos, "goalPos");

        Debug.Log(blackboard.GetVar("player"));
    }

    protected override Node SetupTree()
    {
        // create a new tree
        Node root = new Selector(new List<Node>
            {
                // Bool condtion -> Distance check -> Move Nav Agent
                new C_BoolCondition("interrupt", true,
                    new C_DistanceCheck(C_DistanceCheck.ValueComparison.GreaterThan, dist, self, player,
                        new L_MoveNavAgent(player.transform, agent, dist)
                    )
                ),
                    new Sequence(new List<Node>
                    {
                        new L_SetVar("interrupt", true),
                        new L_InvokeEvent(eventsToCall[0]),
                        new L_FaceObject(self, player, clampVals, 0.0f, 5.0f),
                        new L_SetChargePos("goalPos", goalPos, speed, self, player, goalPos),
                        new L_Wait(waitTimes[0], waitTimesDeviation[0]),
                        new L_InvokeEvent(eventsToCall[1]),
                        new C_Timeout(3.0f, new L_MoveTo("goalPos", agent.gameObject, moveSpeed, 5f)),
                        new L_InvokeEvent(eventsToCall[2]),
                        new L_Wait(waitTimes[1], waitTimesDeviation[1]),
                        new L_SetVar("interrupt", false)
                    })
            });

        // give all nodes access to our blackboard
        root.FillInformation(this);

        return root;
    }
}
