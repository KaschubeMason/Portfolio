# Enemy Logic

The three screenshots in this folder relate to logic performed directly by the enemies.

The code snippets are:

## BehaviorTree_HurtInterruption
This logic is present at the root of each enemy's behavior tree. Whenever an enemy receives damage, they set a "hurt" boolean to true which interrupts the behavior tree to play the hurt animation.

## TankBehaviorTree_AttackLogic
This is the sequence of events for the Tank enemy responsible for attacking. The Behavior Tree selects which attack to play based off how close or far away the player is. 

For the Lunge Attack specifically, the way the "Play Lunge Attack" nodes are set up allows for the enemy to continuously turn to face the player while they charge up their launch.
Once they're mid-air, the animation pauses until they collide with a ground object, in which case they resume the animation until it finishes.
