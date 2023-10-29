/*****************************************************************//**
 * \file   CollisionManager.cpp
 * \author Mason Kaschube mason.kaschube
 * \param  Return to the Skyway
 * \param  GAM200AF21
 *
 * \date   9/20/21
 *
 * \param Copyright © 2021 DigiPen (USA) Corporation.
 *********************************************************************/

 //--------------------------------------------------------------------
 // Include Files
 //--------------------------------------------------------------------
#include "CollisionManager.h"
#include "Collider.h"
#include "RigidBody.h"

//#define MAX_DYNAMIC 100
//#define MAX_STATIC 100
//#define PLACEHOLDER 1
//--------------------------------------------------------------------
// Forward References
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Public Class Functions
//--------------------------------------------------------------------

static CloudEngine::Physics::PhysicsSystem* physicsInstance_;

CloudEngine::Physics::PhysicsSystem::PhysicsSystem()
{
	// create a new physics instance
	physicsInstance_ = this;
}

CloudEngine::Physics::PhysicsSystem::~PhysicsSystem()
{
	rigidBodies_.clear();	// clear the list of active RigidBodies
	colliders_.clear();		// clear the list of active Colliders
}

void CloudEngine::Physics::PhysicsSystem::PreUpdate()
{
	// loop through the RigidBodies
	for (RigidBody* rb : rigidBodies_)
	{
		if (!rb) continue;
		rb->SetGrounded(false);	// Set them to not grounded
		rb->ClearCollisions();	// Remove all the collision events
	}

	// check for collisions
	CheckCollisions();

	// resolve the ones that are colliding
	ResolveCollisions();
}

void CloudEngine::Physics::PhysicsSystem::AddRigidBody(RigidBody* rb)
{
	// loop through the RigidBody list to try and find the passed in RigidBody
	std::vector<RigidBody*>::iterator it = std::find(physicsInstance_->rigidBodies_.begin(), physicsInstance_->rigidBodies_.end(), rb);

	// if the RigidBody isn't already in the list
	if (it == physicsInstance_->rigidBodies_.end())
	{
		// add it
		physicsInstance_->rigidBodies_.push_back(rb);
	}
}

void CloudEngine::Physics::PhysicsSystem::AddCollider(Collider* c)
{
	// Loop through the Collider list and look for the passed in collider
	std::vector<Collider*>::iterator it = std::find(physicsInstance_->colliders_.begin(), physicsInstance_->colliders_.end(), c);

	// if we didn't find the collider
	if (it == physicsInstance_->colliders_.end())
	{
		// add it to the list
		physicsInstance_->colliders_.push_back(c);
	}
}

void CloudEngine::Physics::PhysicsSystem::RemoveCollider(Collider* c)
{
	// look for the Collider in the list and delete it if we ind it
	auto it = std::find(physicsInstance_->colliders_.begin(), physicsInstance_->colliders_.end(), c);
	if (it != physicsInstance_->colliders_.end())
		physicsInstance_->colliders_.erase(it);
}

void CloudEngine::Physics::PhysicsSystem::RemoveRigidBody(RigidBody* c)
{
	// look for the RigidBody in the list and delete it if we find it
	auto it = std::find(physicsInstance_->rigidBodies_.begin(), physicsInstance_->rigidBodies_.end(), c);
	if(it != physicsInstance_->rigidBodies_.end())
		physicsInstance_->rigidBodies_.erase(it);
}

void CloudEngine::Physics::PhysicsSystem::ClearPhysicsLists()
{
	// clear the Colliders list
	physicsInstance_->colliders_.clear();

	// clear the RigidBodies list
	physicsInstance_->rigidBodies_.clear();
}

void CloudEngine::Physics::PhysicsSystem::CheckCollisions()
{
	// if the colliders list is empty, quit out
	if (colliders_.empty()) return;

	// grab the length of the Colliders list
	int i, j, len = colliders_.size();

	// loop through it
	for (i = 0; i < len; i++)
	{
		// grab the collider
		Collider* a = colliders_[i];

		// loop through all colliders after it
		for (j = i + 1; j < len; j++)
		{
			// grab the next collider
			Collider* b = colliders_[j];

			// check the first against the next
			a->CheckCollision(*b);
		}
	}
}

void CloudEngine::Physics::PhysicsSystem::ResolveCollisions()
{
	// if the RigidBodies list is empty, quit out
	if (rigidBodies_.empty()) return;

	// loop through all the RigidBodies
	for (RigidBody* rb : rigidBodies_)
	{
		if (!rb) continue;
		// resolve their collisions
		rb->ResolveCollisions();
	}
}
