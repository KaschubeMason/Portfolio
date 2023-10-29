/*****************************************************************//**
 * \file   RigidBody.h
 * \author Mason Kaschube (mason.kaschube)
 * \param  Return to the Skyway
 * \param  GAM200AF21
 *
 * \date   September 5th 2021
 *
 * \param Copyright © 2021 DigiPen (USA) Corporation.
 *********************************************************************/

#pragma once

// Important Equations:
// F = m * a	Force = Mass * Acceleration
// T = r * w	Torque = (vector from center of mass to point on object) * rotational velocity
// w = r * v	Rotational Velocity = (vector from COM to point on object) * linear velocity
// v = w * r	Linear Velocity = rotational velocity * (vector from COM to point on object)
// pi = pf		Initial momentum = final momentum	Conservation of Momentum
// vi1 - vi2 = -vf1 + vf2	
// vf1 = -(vi1 - vi2 + vf2)
// pi = -m1(vi1 - vi2) + (m1 * vf1) + (m2 * vf2)
// vf2 = (-m1 * (vi1 - vi2) - pi) / (-m1 + m2)
// plug the vf2 into the vf1 equation and solve
// vf1 = -(vi1 - vi2 + ((-m1 * (vi1 - vi2) - pi) / (-mi + m2))

//---------------------------------------------------------------------
// Include Files
//--------------------------------------------------------------------
#include "Component.h"
#include "Collider.h"
#include "Transform.h"
#include "rttr/registration_friend.h"
#include <glm.hpp>


//--------------------------------------------------------------------
// Forward References
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Public Constants
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Pubilc Variables
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Public Functions
//--------------------------------------------------------------------
namespace CloudEngine
{
	namespace Physics
	{
		/**
		 * The type of RigidBody current body is. Determines behavior
		 */
		typedef enum {
			pInvalid = -1
			, pDynamic
			, pKinematic
			, pStatic
		} BodyEnum;

		class RigidBody : public Components::Component
		{
			// Public Functions
		public:

			/**
			 * Default constructor for RigidBody object.
			 * 
			 */
			RigidBody();

			/**
			 * Initialization constructor for RigidBody.
			 * 
			 * \param type the body type we want (static, dynamic, kinematic)
			 */
			RigidBody(BodyEnum type);

			/**
			 * Copy constructor.
			 * 
			 * \param other the RigidBody to copy
			 */
			RigidBody(const RigidBody& other);

			/**
			 * Destructor.
			 */
			~RigidBody();

			void Serialize(Serialization& serialization);
			void Deserialize(Serialization& serialization);

			/**
			 * Overrides the Component's base Start function.
			 */
			void Start() override;

			/**
			 * Overrides the Component's base Stop function.
			 */
			void Stop() override;

			/**
			 * Update function for the RigidBody.
			 *
			 * \param dt DeltaTime, passed in but can also be found in Time::DeltaTime()
			 */
			void Update(float dt) override;

			/**
			 * Draws the wireframe around the object.
			 */
			void Render() override;

			/**
			 * Adds a collision to the list of current collisions.
			 * 
			 * \param add the collision event to add
			 */
			void AddCollision(Collision& add);
			
			/**
			 * Grabs the list of current collisions.
			 * 
			 * \return list of collisions
			 */
			std::vector<Collision> GetCollisions();

			/**
			 * Finds a collision event in the list.
			 * 
			 * \param find The collision event we want
			 * \return pointer to the collision event
			 */
			Collision* FindCollision(Collision find);

			/**
			 * Resolves the collisions in the collision list.
			 */
			void ResolveCollisions();

			/**
			 * Clones a RigidBody object.
			 *
			 * \return a clone of the RigidBody
			 */
			Components::Component* Clone(void) const override;

			/**
			 * Set the body type of the RigidBody.
			 * 
			 * \param type the body we want to assign
			 */
			void SetBody(BodyEnum type);

			/**
			 * Set the mass of the RigidBody.
			 * 
			 * \param mass the value to set
			 */
			void SetMass(float mass);

			/**
			 * Sets the velocity of the RigidBody.
			 * 
			 * \param velocity the velocity we want to set
			 */
			void SetVelocity(glm::vec2 velocity);

			/**
			 * Set whether or not this object experiences gravity.
			 * 
			 * \param gravity do we have gravity?
			 */
			void SetGravity(bool gravity);

			/**
			 * Gets the body type of this RigidBody.
			 * 
			 * \return the body type (dynamic, static, kinematic)
			 */
			BodyEnum GetBody() const;

			/**
			 * Gets the mass of the RigidBody.
			 * 
			 * \return the RigidBody's mass
			 */
			float GetMass() const;

			/**
			 * Gets the velocity of the RigidBody.
			 * 
			 * \return the RigidBody's velocity
			 */
			glm::vec2 GetVelocity() const;

			/**
			 * Set whether or not this object is moving.
			 * 
			 * \param are we moving?
			 */
			void SetMoving(bool);

			/**
			 * Determines if the object is moving.
			 * 
			 * \return true if we're moving, false if we're stationary
			 */
			bool IsMoving();

			/**
			 * Is this object experiencing gravity?.
			 * 
			 * \return true if we are, false if not
			 */
			bool IsGravity() const;

			/**
			 * Are we on the ground.
			 * 
			 * \return true if we are, false if not
			 */
			bool IsGrounded() const;

			/**
			 * Set whether or not we're on the ground.
			 * 
			 * \param are we on the ground
			 */
			void SetGrounded(bool);

			/**
			 * Adds a force to the object.
			 * 
			 * \param force the force to apply
			 */
			void AddForce(glm::vec2 force);

			void SetCollisionResolve(bool toggle);

			bool GetResolveCollision() const;

			void AddVelocity(glm::vec2);

			void AddAcceleration(glm::vec2);

			/**
			 * Clears the list of all collisions.
			 */
			void ClearCollisions();

			/**
			 * Change the color of the line.
			 * 
			 * \param col the color of the line
			 */
			void SetLineColor(glm::vec4 col) { lineCol = col; }

			/**
			 * Adds an impulse to an object (no longer used).
			 */
			//void ApplyImpulse(glm::vec2 impulse);

			// Private Variables
		private:
			void ResolveCollision(Collision& collision);

			BodyEnum bodyType_;			// Type of RigidBody (Dynamic, Static, Kinematic)
			glm::vec2 velocity_;		// The Velocity of our RigidBody
			glm::vec2 maxVel_;
			float mass_, imass_;		// Mass and inverse mass of our object
			float friction_;			// The percent of friciton we want. From 0 (no stop) to 1 (full stop)
			float drag_;				// How quickly we want to slow down when we're not moving	
			bool resolveCollisions_ = true;	// Do we resolve collisions?
			bool hasGravity_;			// Does this object experience gravity?
			bool locGrav_;
			float gravityMult_;
			bool disableVel_ = false;
			glm::vec4 lineCol = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);	// color of the debug draw line

			std::vector<Collision> collisions_;		// List of all collisions in the scene

			static float gravity_;		// The strength of the gravity
			bool isMoving_;				// Whether or not we're moving
			bool grounded_;				// Are we on the ground?

			RTTR_ENABLE(Component);
			RTTR_REGISTRATION_FRIEND;
		};
		typedef RigidBody* RigidBodyPtr;
	}
}
