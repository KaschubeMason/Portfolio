# Enemy Logic

The three screenshots in this folder relate to logic performed directly by the enemies.

The code snippets are:

## BehaviorTree_HurtInterruption
This logic is present at the root of each enemy's behavior tree. Whenever an enemy receives damage, they set a "hurt" boolean to true which interrupts the behavior tree to play the hurt animation.

Since this logic is at the root of the behavior tree, it can interrupt any other process in the tree itself, meaning the player can interrupt enemy attacks if they are able to trigger the hurt behavior. Not every enemy can easily reach this state, however, as each enemy has a damage threshold that must be reached before this hurt state is triggered. Damage dealt to the enemy is collected until the threshold is reached, at which point the value is reset and the hurt state is triggered.

## EnemyLogic_LaunchPlayer
This function is responsible for dealing knockback to the player on certain attacks. 
It finds a vector from the enemy to the player, scales it up by a fixed amount, and applies that impulse to the player.

One of the tricky things I ran into when making this function was that if the player was already mid-air when they got hit, oftentimes they would end up getting launched further into the air in a way that wasn't very believable. To fix this, I added separate logic for when the player has a non-zero Z-value on their velocity component.

## TankBehaviorTree_AttackLogic
This is the sequence of events for the Tank enemy responsible for attacking. The Behavior Tree selects which attack to play based on how close or far away the player is. 

For the Lunge Attack specifically, the way the "Play Lunge Attack" nodes are set up allows for the enemy to continuously turn to face the player while they charge up their launch.
Once they're mid-air, the animation pauses until they collide with a ground object, in which case they resume the animation until it finishes.
