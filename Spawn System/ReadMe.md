# Spawn System

These code snippets are used in the Spawn System for our game.
The spawn system allows designers to control aspects of encounters, including how many waves each encounter has, what enemies spawn in each wave, and where each of those enemies spawn.

## SpawnSystemExample

This screenshot shows how the Spawn System would look when working in editor. Some of the available aspects to modify (see the Outliner window on the right) are as follows:
- **Max Enemy Aggros**: This value controls how many enemies at one time will gain aggro on the player for the encounter.
Enemies with aggro will charge the player for attack, while enemies without aggro will hang back until the closer enemies are defeated or out of range.
Regardless of if the enemy has aggro, if the player gets within their attack range, they will attack
- **Next Wave Delay**: The time, in seconds, between waves
- **Spawn Locations**: This array of transforms controls where the enemies spawn. Each element is represented by the purple octahedron outline in the playable area (Spawn Locations[0], [1], etc.).
- **Wave 1,2,3,4**: These arrays correspond to the enemy composition for each wave. The position of the enemy in the array directly corresponds to where they will spawn in the wave
(the first enemy spawns at Spawn Locations[0], the second at Spawn Locations[1], etc.). If desired, you could input "None" in the Wave array and the corresponding Spawn Location would be skipped over.
If the Wave array is larger than the Spawn Locations array, enemy spawns will start looping back to 0, 1, 2, etc. but be offset slightly so as to not intersect.

## SpawnSystem_AggroEnemies

This function is responsible for determining which enemies hold aggro toward the player. It starts by finding the closest enemy to the player, finding that enemy's index in the array of alive enemies, and saving
out that index. It repeats this process, ignoring enemies who already hold aggro, until it has reached the specified amount of enemies to aggro on the player. 

## SpawnSystem_SpawnEnemies

This function is what actually spawns the enemies at their locations. It first figures out what spawn location to be used to spawn, and just in case there are more enemies than spawn positions, this is done
using modulo, so any extra enemies will start spawning at the first positions again. Once it finds its corresponding spawn position, it shifts that position up 3 units on the Z axis in order to prevent enemies 
from spawning in the floor. It then spawns the specified actor, adds it to the list of alive enemies, and plays a corresponding audio clip depending on what enemy spawned.
