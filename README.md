# Steel Reaper
This project is my Senior year game project at DigiPen Institute of Technology. 

Steel Reaper was built in Unity by a small team of 3 programmers. My primary role in this project is to create and manage all the AI systems. These include the spawning system, the framework for enemy logic, creating and maintaining my custom Behavior Tree architecture, and modifying and testing the behavior of our first enemy. This project is still in progress and as such the code samples here will be updated as necessary.

**High Concept:** You are Steel Reaper, an elite mech soldier tasked with fending off an impending alien invasion. Blast your way through endless waves of enemies in this over-the-top, action-packed third-person shooter. 

**Role:** Design, implement, and maintain all systems related to AI

---

The files present are:

**BT_BeetleEnemy.cs:** This file is the Behavior Tree script for our first enemy we call "The Beetle". This file mostly shows how a behavior tree is made using the architecture, with all of the nodes in the "SetupTree" function being custom logic.

**Node.cs:** This file showcases the base class for all the nodes in the behavior tree. All leaf nodes inherit from this base class

**EnemyLogic.cs:** This file is the base class for all enemy logic. It contains shared features like VFX objects, health, damage, and other stat variables, and spawn/death logic. 
