/*****************************************************************//**
 * \file   RigidBody.cpp
 * \author Mason Kaschube (mason.kaschube)
 * \param  Return to the Skyway
 * \param  GAM200AF21
 *
 * \date   September 5th 2021
 *
 * \param Copyright © 2021 DigiPen (USA) Corporation.
 *********************************************************************/

 //---------------------------------------------------------------------
 // Include Files
 //--------------------------------------------------------------------
#include <glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include "glm/gtx/transform.hpp"
#include "RigidBody.h"
#include "InstantDeath.h"
#include "GameObject.h"
#include "Time.h"
#include <iostream>
#include "rttr/registration.h"
#include "CollisionManager.h"
#include "PlayerController.h"
#include "BehaviorWall.h"
#include "BehaviorPickUp.h"
#include "BehaviorCheckpoint.h"
#include "BehaviorHostile.h"
#include "BehaviorTriggerBox.h"
#include "BehaviorCameraSwitch.h"
#include "LevelEnd.h"


using namespace CloudEngine::Physics;
//--------------------------------------------------------------------
// Forward References
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Public Constants
//--------------------------------------------------------------------

RTTR_REGISTRATION
{
	//RigidBody

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Velocity", &CloudEngine::Physics::RigidBody::velocity_)
	.constructor<>();

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Max Velocity", &CloudEngine::Physics::RigidBody::maxVel_);

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Mass", &CloudEngine::Physics::RigidBody::mass_);

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Drag", &CloudEngine::Physics::RigidBody::drag_);

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Gravity", &CloudEngine::Physics::RigidBody::hasGravity_);

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Gravity Multiplier", &CloudEngine::Physics::RigidBody::gravityMult_);

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Resolve Collision", &CloudEngine::Physics::RigidBody::resolveCollisions_);

rttr::registration::class_<CloudEngine::Physics::RigidBody>
	("Rigid Body").property("Update Velocity", &CloudEngine::Physics::RigidBody::disableVel_);
}

// gravity constant
float RigidBody::gravity_ = 9.84f;

// epsilon for the x-axis
static float x_epsilon = 25.0f;
//static float x_epsilon = 0.1f;

// epsilon for the y-axis
static float y_epsilon = 0.5f;
//static float y_epsilon = 0.1f;

RigidBody::RigidBody() : collisions_()
{
	bodyType_ = pDynamic;					// dynamic body
	mass_ = 10;								// default 10 mass
	imass_ = mass_ == 0 ? 0 : 1.0f / mass_;	// calculate the inverse mass
	velocity_ = glm::vec2(0.0f, 0.0f);		// 0 velocity
	friction_ = 0.9f;						// default friction
	hasGravity_ = true;					// no gravity
	grounded_ = false;						// not grounded
	isMoving_ = false;						// not moving	
	drag_ = 0.9f;							// default drag
	resolveCollisions_ = true;
	maxVel_ = glm::vec2(1000.0f, 4000.0f);
	locGrav_ = false;
	gravityMult_ = 100.0f;
}

RigidBody::RigidBody(BodyEnum bodyType) : collisions_()
{
	bodyType_ = bodyType;					// copy over the body type
	mass_ = bodyType == pStatic ? 0 : 10;	// calculate the mass
	imass_ = mass_ == 0 ? 0 : 1.0f / mass_;	// calculate the inverse mass
	velocity_ = glm::vec2(0.0f, 0.0f);		// 0 velocity
	friction_ = 0.9f;						// default friction
	hasGravity_ = true;					// no gravity
	grounded_ = false;						// not grounded
	isMoving_ = false;						// not moving
	drag_ = 0.7f;							// default drag
	resolveCollisions_ = true;
	maxVel_ = glm::vec2(1000.0f, 4000.0f);
	locGrav_ = false;
	gravityMult_ = 100.0f;
}

RigidBody::RigidBody(const RigidBody& other) : collisions_()
{
	bodyType_ = other.bodyType_;		// Copy over the body type
	mass_ = other.mass_;				// copy over the mass
	imass_ = other.imass_;				// copy over the inverse mass
	velocity_ = other.velocity_;		// copy over the velocity
	hasGravity_ = other.hasGravity_;	// vopy over the gravity state
	grounded_ = other.grounded_;		// copy over the grounded state
	isMoving_ = other.isMoving_;		// copy over the isMoving state
	drag_ = other.drag_;				// copy over the drag
	friction_ = other.friction_;		// copy over the friction
	maxVel_ = other.maxVel_;
	locGrav_ = other.locGrav_;
	gravityMult_ = other.gravityMult_;
}

RigidBody::~RigidBody()
{
	// Clear the collisions list
	collisions_.clear();

	// If the RigidBody has no parent, don't do anything
	if (this->Parent() == NULL) return;

	// remove the RigidBody from the list
	PhysicsSystem::RemoveRigidBody(this);
}

void RigidBody::Serialize(CloudEngine::Serialization& serialization)
{
	serialization.GetOutputArchive()->setNextName("rigid_body");
	serialization.GetOutputArchive()->startNode();

	serialization.SerializeInt("bodyType", (int)bodyType_);
	serialization.SerializeFloat("friction", friction_);
	serialization.SerializeFloat("gravity", gravity_);
	serialization.SerializeFloat("mass", mass_);
	serialization.SerializeFloat("imass", imass_);
	serialization.SerializeFloat("drag", drag_);
	serialization.SerializeBool("hasGravity", locGrav_);
	serialization.SerializeBool("resolveCollisions", resolveCollisions_);

	serialization.GetOutputArchive()->finishNode();
}

void RigidBody::Deserialize(CloudEngine::Serialization& serialization)
{
	serialization.GetInputArchive()->startNode();
	const char* member_name = serialization.GetInputArchive()->getNodeName();

	int bodyType;
	serialization.DeserializeInt(bodyType);
	bodyType_ = (BodyEnum)bodyType;
	serialization.DeserializeFloat(friction_);
	serialization.DeserializeFloat(gravity_);
	serialization.DeserializeFloat(mass_);
	serialization.DeserializeFloat(imass_);
	serialization.DeserializeFloat(drag_);
	serialization.DeserializeBool(locGrav_);
	serialization.DeserializeBool(resolveCollisions_);

	serialization.GetInputArchive()->finishNode();
}

void CloudEngine::Physics::RigidBody::AddCollision(Collision& add)
{
	// Look for the collision in the list
	if (CloudEngine::Physics::RigidBody::FindCollision(add) == NULL)
	{
		// if we couldn't find it, add it
		collisions_.push_back(add);
	}
}

void CloudEngine::Physics::RigidBody::Start()
{
	// Add the RigidBody to the list
	PhysicsSystem::AddRigidBody(this);
}

void CloudEngine::Physics::RigidBody::Stop()
{
	// Remove the RigidBody from the list
	PhysicsSystem::RemoveRigidBody(this);
}

void CloudEngine::Physics::RigidBody::ResolveCollisions()
{
	// if we have no collisions, quit out
	if (collisions_.empty()) return;

	// loop through each collision
	for (Collision& c : collisions_)
	{
		// if either collider has no parent, abort
		if (!c.aCollider_.Parent() || !c.bCollider_.Parent()) continue;
		//RigidBody* rb1 = c.aCollider_.Parent()->GetComponent<RigidBody>();
		//RigidBody* rb2 = c.bCollider_.Parent()->GetComponent<RigidBody>();

		int aLayer = c.aCollider_.Parent()->GetLayerNumber();
		int bLayer = c.bCollider_.Parent()->GetLayerNumber();

		if ((aLayer == -1 && bLayer == 100) || (aLayer == 100 && bLayer == -1)) continue;

		// grab whether nor not each is a trigger
		bool aTrigger = c.aCollider_.GetIsTrigger();
		bool bTrigger = c.bCollider_.GetIsTrigger();

		//glm::vec3 aTran = *c.aCollider_.Parent()->GetComponent<Components::Transform>()->GetTranslation();
		//glm::vec3 bTran = *c.bCollider_.Parent()->GetComponent<Components::Transform>()->GetTranslation();

		// If either layer is -1, always collide. Otherwise if b is higher up or on the same level as a, they collide
		if ((bLayer == -1 || aLayer == -1 || bLayer >= aLayer) && 
		   ((aTrigger == false) && (bTrigger == false)))
		{
			// resolve the collision
			ResolveCollision(c);

			// call any other collision handler for specific behavior
			Components::WallCollisionHandler(c);
			Player::InstantDeathCollider(c);
			
		}
		else
		{
			// special behavior
			Player::PlayerCollisionHandler(c);
			Components::PickUpCollisionHandler(c);
			Components::CheckpointCollisionHandler(c);
			Components::TriggerBoxCollisionHandler(c);
			Components::WolfCollisionHandler(c);
			Components::CameraSwitcherCollisionHandler(c);
			Components::LevelEndCollisionHandler(c);
			
			// next collision
			continue;
		}
	}

	// clear the list of collisions
	collisions_.clear();
}

CloudEngine::Components::Component* RigidBody::Clone() const
{
	// Clone the RigidBody
	return new RigidBody(*this);
}

std::vector<Collision> RigidBody::GetCollisions()
{
	// return the list of collisions
	return collisions_;
}

Collision* RigidBody::FindCollision(Collision find)
{
	// loop through the collision list
	for (int i = 0; i < collisions_.size(); i++)
	{
		// if the collision is the same
		if (collisions_[i] == find)
		{
			// if both colliders are the same
		    //if (collisions_[i].aCollider_ == find.aCollider_ && collisions_[i].bCollider_ == find.bCollider_)
			if ((collisions_[i].aCollider_ == find.aCollider_  || collisions_[i].bCollider_ == find.aCollider_) && 
				(collisions_[i].bCollider_ == find.bCollider_ || collisions_[i].aCollider_ == find.bCollider_))
			{
				// return it
				return &collisions_[i];
			}
		}
	}
	// if we couldn't find it
	return NULL;
}

void RigidBody::Update(float dt)
{
	// grab the old velocity before updating
	//glm::vec2 oldVel = glm::abs(velocity_);
	if (disableVel_ == true) { return; }

	// if the RigidBody is static, set the mass to 0
	//if (bodyType_ == pStatic) mass_ = 0;

	if (hasGravity_ == true)
	{
		if (grounded_ == false && bodyType_ != pStatic)
		{
			velocity_.y -= (9.8f * gravityMult_ * dt);
		}
		else if (grounded_ == true)
		{
			if (velocity_.y < 0.0f)
			{
				velocity_.y = 0.0f;
			}
		}
	}

	// grab the transform
	Components::Transform* t = parent_->GetComponent<Components::Transform>();

	// grab the translation
	glm::vec3 position = *t->GetTranslation();

	// if the velocity is less than epsilon, set it to 0
	if (glm::abs(velocity_.x) < x_epsilon) velocity_.x = 0;
	if (glm::abs(velocity_.y) < y_epsilon) velocity_.y = 0;

	// add the drag
	velocity_.x = (1.0f - dt * drag_) * velocity_.x;

	// if we're not on the ground and aren't a static object
	//if(!grounded_ && bodyType_ != pStatic)
		// add gravity
		//velocity_.y -= (9.8f * 100.0f * dt);

	if (velocity_.x != 0)
	{
		// add friction
		velocity_.x *= (1.0f - friction_ * dt);
	}


	// clamp the velocity so it can't exceede the maximum
	velocity_.x = glm::clamp(velocity_.x, -maxVel_.x, maxVel_.x);
	velocity_.y = glm::clamp(velocity_.y, -maxVel_.y, maxVel_.y);

	// add the velocity to the position
	position += glm::vec3(velocity_ * dt, 0);

	// set the translation to the new position
	t->SetTranslation(position);
}

void RigidBody::AddForce(glm::vec2 force)
{
	// add the force to the velocity
	glm::vec2 temp = velocity_ + (force * Time::DeltaTime() / mass_);

	// absolute value of it
	glm::vec2 absTemp = glm::abs(temp);

	// if the velocity is greater than 0 but less than epsilon
	if (absTemp.x > 0 && absTemp.x < x_epsilon)
	{
		// offset epsilon + velocity to ensure at least some force is added
		(temp.x > 0) ? temp.x += x_epsilon : temp.x -= x_epsilon;
	}

	// if the velocity is greater than 0 but less than epsilon
	if (absTemp.y > 0 && absTemp.y < y_epsilon)
	{
		// offset epsilon + velocity to ensure at least some force is added
		(temp.y > 0) ? temp.y += y_epsilon : temp.y -= y_epsilon;
	}

	// set the new velocity
	velocity_ = temp;
	//velocity_ += (force * Time::DeltaTime() / mass_);
}

void CloudEngine::Physics::RigidBody::ClearCollisions()
{
	// clear the collisions list
	collisions_.clear();
}

void CloudEngine::Physics::RigidBody::ResolveCollision(Collision& collision)
{
	// Grab both RigidBodies in the collision
	RigidBody* rb1 = collision.aCollider_.Parent()->GetComponent<RigidBody>();
	RigidBody* rb2 = collision.bCollider_.Parent()->GetComponent<RigidBody>();

	glm::vec3 aT = *collision.aCollider_.Parent()->GetComponent<Components::Transform>()->GetTranslation();
	glm::vec3 bT = *collision.bCollider_.Parent()->GetComponent<Components::Transform>()->GetTranslation();

	int aLayer = collision.aCollider_.Parent()->GetLayerNumber();
	int bLayer = collision.bCollider_.Parent()->GetLayerNumber();

	// if the first collider has no RigidBody
	if (!rb1)
	{
		// assign a standard static RigidBody
		rb1 = new RigidBody(pStatic);
	}

	// if the second collider has no RigidBody
	if (!rb2)
	{
		// assign a standard static RigidBody
		rb2 = new RigidBody(pStatic);
	}

	// if both RigidBodies are static, don't do anything
	if (rb1->bodyType_ == pStatic && rb2->bodyType_ == pStatic) return;

	// grab the velocities
	glm::vec2 vi1 = rb1->velocity_;
	glm::vec2 vi2 = rb2->velocity_;

	// get the average velocity
	glm::vec2 rv = vi2 - vi1;

	// calculate the projected velocity
	float velocityProjected = glm::dot(rv, collision.normal_);

	// if objects are moving away from each other
	if (velocityProjected > 0) return;

	// inverse projected velocity
	float j = -velocityProjected;

	// divide out the inverse mass of both colliders
	j /= rb1->imass_ + rb2->imass_;

	// multiply by the normal to get the impulse
	glm::vec2 impulse = j * collision.normal_;

	if (rb1->GetBody() != pStatic && rb1->resolveCollisions_ == true)
	{
		// offset the velocity by the inverse mass * the impulse
		rb1->velocity_ -= rb1->imass_ * impulse;
	}

	if (rb2->GetBody() != pStatic && rb2->resolveCollisions_ == true)
	{
		// offset the velocity by the inverse mass * the impulse
		rb2->velocity_ += rb2->imass_ * impulse;
	}
}


#include "Debug.h"
void RigidBody::Render()
{
	// Grab any potential collider that might be on the object
	ColliderAABB* colliderAABB = parent_->GetComponent<ColliderAABB>();
	ColliderCircle* colliderC = parent_->GetComponent<ColliderCircle>();
	ColliderOOBB* colliderOOBB = parent_->GetComponent< ColliderOOBB>();

	// grab the object's transform
	Components::Transform* trans = parent_->GetComponent<Components::Transform>();

	// if the object has an AABB collider
	if (colliderAABB)
	{
		// draw a rectangle
	}
	// if the object has a Circle collider
	else if (colliderC)
	{
		// draw a circle
	}
	// if the object has an OOBB collider
	else if (colliderOOBB)
	{
		// transfered to a separate function
	}

	// grab the translation offset by the velocity
	glm::vec3 B = *trans->GetTranslation() + glm::vec3(velocity_, 0);

	// draw the velocity line
	Debug::DrawLine(*trans->GetTranslation(), B, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

	// loop through each collision
	for (Collision& collision : collisions_)
	{
		// grab the position of the intersect
		glm::vec3 pos1 = glm::vec3(collision.intersectPoint_, 0);

		// and the normal
		glm::vec3 pos2 = pos1 + glm::vec3(collision.normal_, 0) * 20.0f;

		// draw a circle at the intersect point
		Debug::DrawCircle(pos1, 5.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

		// draw a line in the direction of the normal
		Debug::DrawLine(pos1, pos2, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	}

	// clear the collisions list
	collisions_.clear();
}

//======================================================================//
//																		//
//							Setters & Getters							//
//																		//
//======================================================================//

void RigidBody::SetBody(BodyEnum type)
{
	// set the body type
	bodyType_ = type;
}

void RigidBody::SetMass(float mass)
{
	// set the mass and inverse mass
	mass_ = mass;
	imass_ = mass == 0 ? 0 : 1.0f / mass;
}

void RigidBody::SetVelocity(glm::vec2 velocity)
{
	// set the velocity
	velocity_ = velocity;
}

void RigidBody::SetMoving(bool set)
{
	// set whether we'r emoving
	isMoving_ = set;
}

void RigidBody::SetGravity(bool gravity)
{
	// set the gravity
	locGrav_ = gravity;
}

void RigidBody::SetGrounded(bool set)
{
	// set whether or not we're on the ground
	grounded_ = set;
}

BodyEnum RigidBody::GetBody() const
{
	// return the body type (Dynamic, Static, Kinematic)
	return bodyType_;
}

float RigidBody::GetMass() const
{
	// return the mass
	return mass_;
}

glm::vec2 RigidBody::GetVelocity() const
{
	// return the velocity
	return velocity_;
}

bool RigidBody::IsMoving()
{
	// return the moving
	return isMoving_;
}

bool RigidBody::IsGravity() const
{
	// true if we are experiencing gravity
	return locGrav_;
}

bool RigidBody::IsGrounded() const
{
	// true if we're on the ground
	return grounded_;
}

void RigidBody::SetCollisionResolve(bool toggle)
{
	resolveCollisions_ = toggle;
}

bool RigidBody::GetResolveCollision() const
{
	return resolveCollisions_;
}


//======================================================================//
//																		//
//						Deprication Graveyard							//
//																		//
//======================================================================//

// logic was initially used in OnCollisionEnter to calculate velocities after collision

	//glm::vec2 f1 = vi1 * m1 / dt;								// force the object applies in the collision
	//glm::vec2 f2 = vi2 * m2 / dt;								// same for other collider
	//float alpha1 = 0;
	//float alpha2 = 0;
	//alpha1 = glm::dot(vi1 == glm::vec2(0, 0) ? vi1 : glm::normalize(vi1), collision.normal_);	// what % of the momentum is dumped into the other object
	//alpha2 = glm::dot(vi2 == glm::vec2(0, 0) ? vi2 : glm::normalize(vi2), collision.normal_);	// ^ same for other object
	//glm::vec2 fa1 = -collision.normal_ * glm::length(f1) * alpha1;									// amount of force applied by first object
	//glm::vec2 fa2 = collision.normal_ * glm::length(f2) * alpha2;									// ammount of force applied by other object
	//glm::vec2 vf1;
	//glm::vec2 vf2;

	//if ((aBody->velocity_.x > epsilon && bBody->velocity_.x > epsilon) ||
	//	(aBody->velocity_.y > epsilon && bBody->velocity_.y > epsilon) ||
	//	(aBody->velocity_.x < -epsilon && bBody->velocity_.x < -epsilon) ||
	//	(aBody->velocity_.y < -epsilon && bBody->velocity_.y < -epsilon))		// This is INCREDIBLY scuffed. There is no operator overloads for glm::vec2 (</>) glm::vec2 so I have to do it manually
	//{
	//	vf1 = vi1 + (fa1 + fa2) * dt / m1;	// sum forces applied, multiply by change in time,
	//	vf2 = vi2 + (-fa2 - fa1) * dt / m2;	// factor out mass to get change in velocity, add to original velocity
	//}
	//else
	//{
	//	vf1 = vi1 + (fa1 - fa2) * dt / m1;	// sum forces applied, multiply by change in time,
	//	vf2 = vi2 + (fa2 - fa1) * dt / m2;	// factor out mass to get change in velocity, add to original velocity
	//}