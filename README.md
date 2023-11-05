# Return to the Skyway
This project was my first real experience working with a large cross-disciplinary team of 13 members (7 programmers, 2 designers, 3 artists, 1 sound designer) as a Sophomore student at DigiPen Institute of Technology.

This project saw us creating our own custom engine in C++ and building a game on top of it. Most of the work I did for this project was on creating and managing our custom 2D physics engine.

**High concept:** Traverse through a fantastical island and utilize the local fauna to destroy obstacles. In this 2D platform, the player controls a character that has crash-landed on a tropical island. 
The player must complete puzzles utilizing the creatures' unique abilities in order to fix their ship and return to the Skyway!

**Role:** Create and manage a 2D physics engine that is optimized and capable of handling and resolving multiple collisions at a given time.

Copyright © 2020 DigiPen Institute of Technology, All rights reserved.

Link to the game: https://games.digipen.edu/games/return-to-the-skyway
Alternative link: https://store.steampowered.com/app/2066270/Return_to_the_Skyway/
---

the files contained are:

**Collider.cpp/h:** These files provide all the logic used to define circle and AABB colliders, as well as how to resolve collisions between each. This file also contains a scrapped implementation of OOBB logic.

**CollisionManager.cpp/h:** These files are designed to manage all collision events and resolve them in the order they were created.

**RigidBody.cpp/h:** These files contain logic for rigid body components with features such as mass, drag, gravity, and friction.
