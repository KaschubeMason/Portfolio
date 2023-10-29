/*****************************************************************//**
 * \file   CollisionManager.h
 * \author(s) Mason Kaschube mason.kaschube
 * \param  Return to the Skyway
 * \param  GAM200AF21
 *
 * \date   9/20/21
 *
 * \param Copyright 2021 DigiPen (USA) Corporation.
 *********************************************************************/

#pragma once

//---------------------------------------------------------------------
// Include Files
//--------------------------------------------------------------------
#include "ObjectManager.h"
#include "Collider.h"
#include "Object.h"
#include "GameObject.h"
#include "RigidBody.h"
#include "ISystem.h"
#include <vector>
//--------------------------------------------------------------------
// Forward References
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Public Class Definition
//--------------------------------------------------------------------
namespace CloudEngine
{
	namespace Physics
	{

		class PhysicsSystem : public Engine::ISystem 
		{
		public:
			/**
			 * Default constructor for the physics management system.
			 */
			PhysicsSystem();

			/**
			 * Destructor for the physics management system.
			 */
			~PhysicsSystem();

			/**
			 * Override of the ISystem's PreUpdate. Handles update logic of Colliders and RigidBodies
			 */
			void PreUpdate() override;

			/**
			 * Adds a RigidBody to the current list.
			 * 
			 * \param rb the RigidBody to add
			 */
			static void AddRigidBody(RigidBody* rb);

			/**
			 * Removes a RigidBody from the current list.
			 * 
			 * \param rb the RigidBody to remove
			 */
			static void RemoveRigidBody(RigidBody* rb);

			/**
			 * Adds a Collider to the current list.
			 * 
			 * \param c the Collider to add
			 */
			static void AddCollider(Collider* c);

			/**
			 * Removes a Collider from the current list.
			 * 
			 * \param c the Collider to remove
			 */
			static void RemoveCollider(Collider* c);

			/**
			 * Clears both lists. Used for transitioning scenes
			 */
			static void ClearPhysicsLists();

		private:
			/**
			 * Checks collisions between all colliders.
			 */
			void CheckCollisions();

			/**
			 * Resolves collisions that are present.
			 * 
			 */
			void ResolveCollisions();

			std::vector<Collider*> colliders_;		// List of all active Colliders in the scene
			std::vector<RigidBody*> rigidBodies_;	// List of all active RigidBodies in the scene
		};
	};
};


