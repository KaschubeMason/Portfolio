/*****************************************************************//**
 * \file   Collider.cpp
 * \author Mason Kaschube (mason.kaschube)
 * \param  Return to the Skyway
 * \param  GAM200AF21
 *
 * \date   September 7th 2021
 *
 * \param Copyright © 2021 DigiPen (USA) Corporation.
 *********************************************************************/

 //---------------------------------------------------------------------
 // Include Files
 //--------------------------------------------------------------------
#include <glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "Collider.h"
#include "Transform.h"
#include "GameObject.h"
#include "RigidBody.h"
#include "Time.h"
#include "rttr/registration.h"
#include "CollisionManager.h"
#include "Mesh.h"
#include <ostream>
#include <limits>
#include "Debug.h"
 //--------------------------------------------------------------------
 // Forward References
 //--------------------------------------------------------------------

//--------------------------------------------------------------------
// Public Class Functions
//--------------------------------------------------------------------\

RTTR_REGISTRATION
{
	rttr::registration::class_<CloudEngine::Physics::ColliderAABB>("ColliderAABB")
		.constructor<>()
		.property("Is Trigger", &CloudEngine::Physics::ColliderAABB::isTrigger_)
		.property("Property", &CloudEngine::Physics::ColliderAABB::GetColliderPropertiesInt, &CloudEngine::Physics::ColliderAABB::SetColliderPropertiesInt)
		.property("Offset",	&CloudEngine::Physics::Collider::cOffset_)
		.property("Width", &CloudEngine::Physics::ColliderAABB::width_)
		.property("Height", &CloudEngine::Physics::ColliderAABB::height_)
		.property("Ground Forgiveness", &CloudEngine::Physics::ColliderAABB::epsilon_);

	rttr::registration::class_<CloudEngine::Physics::ColliderCircle>
		("ColliderCircle").property("Radius", &CloudEngine::Physics::ColliderCircle::radius_)
		.constructor<>();

	rttr::registration::class_<CloudEngine::Physics::ColliderOOBB>("ColliderOOBB")
		.constructor<>();


}

namespace CloudEngine
{
	namespace Physics
	{
		static float xBound_ = 1000.0f;
		static glm::vec2 currentNormal_;		// Current normal of the collision
		static glm::vec2 currentIntersect_;		// The intersect point of two colliders
		static float currentPenetration_;		// The penetration depth of one collider into the other

		Collider* GetCollider(Components::GameObject* parent)
		{
			// If the parent has an AABB collider, return it
			if (parent->GetComponent<Physics::ColliderAABB>()) return (Collider*)parent->GetComponent<Physics::ColliderAABB>();

			// If the parent has a Circle collider, return it
			if (parent->GetComponent<Physics::ColliderCircle>()) return (Collider*)parent->GetComponent<Physics::ColliderCircle>();

			// If the parent has an OOBB collider, return it
			if (parent->GetComponent<Physics::ColliderOOBB>()) return (Collider*)parent->GetComponent<Physics::ColliderOOBB>();

			return NULL;
		}

		bool Collision::operator==(Collision other) const
		{
			// If both coliders in the collision are the same
			if (aCollider_ == other.aCollider_ && bCollider_ == other.bCollider_)
			{
				return true;	// The two collision events are the same
			}
			return false;	// otherwise they're different
		}

		//==========================================//
		//					Base					//
		//==========================================//
		Collider::Collider()
		{
			cType_ = ctDefault;			// Default collider
			cProperties_ = cpDefault;	// Default properties
			isColliding_ = false;		// Not colliding
			isTrigger_ = false;			// Not a trigger
			cOffset_ = glm::vec2(0.0f, 0.0f);
			epsilon_ = 5.0f;

			for (int i = 0; i < cpMax; i++)
			{
				collisionMask_[i] = 0;	// No collision Masks
			}

		}

		void Collider::SetIsTrigger(bool set)
		{
			isTrigger_ = set;	// Set the trigger
		}

		bool Collider::GetIsTrigger()
		{
			return isTrigger_;	// Returns true if this collider is a trigger
		}

		Collider::~Collider()
		{

			//PhysicsSystem::RemoveCollider(this);
			// empty for now
		}

		bool Collider::operator==(const Collider& other) const
		{
			if (this->parent_ == other.parent_)		// if the two colliders have the same parent
			{
				if (this->cType_ == other.cType_)	// and the same collider type
				{
					return true;	// they're the same collider
				}
			}
			return false;	// otherwise they're different colliders
		}

		ColliderType Collider::GetColliderType()
		{
			// Return the type of collider (AABB, Circle, OOBB)
			return cType_;
		}

		bool Collider::GetIsColliding() const
		{
			// Returns true if this collider is colliding with at least one other collider
			return isColliding_;
		}

		ColliderProperties Collider::GetColliderProperties() const
		{
			// Returns the collider's properties (player collider, collectible collider, enemy collider, etc.) 
			return cProperties_;
		}

		int* Collider::GetCollisionMask()
		{
			// Returns a list of all the collision masks
			return collisionMask_;
		}

		void Collider::SetCollisionMask(int ignore)
		{
			// Adds an ignore to the collision mask
			collisionMask_[ignore] = 1;
		}

		void Collider::SetColliisonMasks(int* ignoreList)
		{
			// Loop through the list
			for (int i = 0; ignoreList[i]; i++)
			{
				if (ignoreList[i] > cpMax) { return; }	// if we've gone too far, quit out
				collisionMask_[ignoreList[i]] = ignoreList[i];	// otherwise combine the ignore list
			}
		}

		void Collider::Start()
		{
			// If the collider has a parent
			if (this->Parent() != NULL)
				// Add the collilder to the active collider list in the Collision Manager
				PhysicsSystem::AddCollider(this);
		}

		void Collider::Stop()
		{
			// Remove the collider from the Collision Manager
			PhysicsSystem::RemoveCollider(this);
		}

		void Collider::SetColliderProperties(ColliderProperties properties)
		{
			// Set the collider properties (Player collider, Enemy collider, Obstacle collider, etc.)
			cProperties_ = properties;
		}

		void Collider::SetIsColliding(bool set)
		{
			// Sets if we're colliding or not
			isColliding_ = set;
		}

		void Collider::CheckCollision(Collider& otherCollider)
		{
			// if neither collider has a parent, there is a problem
			if (!this->parent_ || !otherCollider.parent_) { return; }
			bool isColliding = false;
			if (!this->parent_->GetComponent<Physics::RigidBody>() || this->parent_->GetComponent<Physics::RigidBody>()->GetBody() == pStatic)
			{
				isColliding = otherCollider.Colliding(*this);
			}
			else
			{
				isColliding = Colliding(otherCollider);	// determine if we're colliding
			}


			if (isColliding)	// if we're colliding
			{
				// Grab the current collider's RigidBody
				RigidBody* rb = parent_->GetComponent<RigidBody>();

				// Make a collision event out of it
				Collision col = { *this, otherCollider, currentIntersect_, currentNormal_ };

				// Make a collision event with the other colllider
				Collision ncol = { otherCollider, *this, currentIntersect_, currentNormal_ };

				// if our collider doesn't have a RigidBody
				if (!rb)
				{
					// grab the other collider's RigidBody
					rb = otherCollider.parent_->GetComponent<RigidBody>();

					// if neither object has a RigidBody
					if (!rb)
					{
						// Nothing happens when they collide
						return;
					}

					// if !trigger
					// Add it to the list of collisions
					rb->AddCollision(ncol);

					// quit out
					return;
				}

				if (parent_->GetComponent<RigidBody>()->GetBody() == pStatic)
				{
					// If the first collider has a static body
					rb->AddCollision(ncol);
				}
				else
				{
					// if !trigger
					// Else if the first body has a dynamic body
					rb->AddCollision(col);
				}
			}
		}
		//==========================================//
		//				   Circle					//
		//==========================================//

		ColliderCircle::ColliderCircle()
		{
			radius_ = 20.0f;	// Default radius
			cType_ = ctCircle;	// Collider circle
		}

		ColliderCircle::ColliderCircle(const ColliderCircle& other)
			: radius_(other.radius_)
		{
			cType_ = ctCircle;	// Collider circle
		}

		ColliderCircle::~ColliderCircle()
		{
			Collider::~Collider();	// Call the default destructor
		}

		bool ColliderCircle::Colliding(ColliderCircle& otherCollider)
		{
			// Grab the parent's transform
			Components::Transform* thisTransform = Parent()->GetComponent<Components::Transform>();

			// Grab the other collider's parent's transform
			Components::Transform* otherTransform = otherCollider.Parent()->GetComponent<Components::Transform>();

			// Grab both radii
			float r1 = radius_;
			float r2 = otherCollider.radius_;

			// Grab both translations
			glm::vec2 p1 = *thisTransform->GetTranslation();
			glm::vec2 p2 = *otherTransform->GetTranslation();

			// If the length between the two objects is longer than the length of both radii
			if (glm::length((p2 - p1)) > r1 + r2)
			{
				return false;	// not possible to collide
			}

			// Set the current intersect to be the point along the line between the two colliders where they intersect
			currentIntersect_ = p1 + (r1 / (r1 + r2)) * (p2 - p1);

			// Set the current normal to be the vector between the first point and the intersect point
			currentNormal_ = glm::normalize(p1 - currentIntersect_);

			// Set the new transform so the two colliders don't continue to collide
			thisTransform->SetTranslation(glm::vec3(p2 + glm::normalize(p1 - p2) * (r1 + r2 + 0.1f), thisTransform->GetTranslation()->z));
			return true;
		}

		void ColliderCircle::Render()
		{
			Components::Transform* trans = parent_->GetComponent<Components::Transform>();
			Debug::DrawCircle(*trans->GetTranslation(), GetRadius(), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		void ColliderCircle::Serialize(Serialization& serialization)
		{
			serialization.GetOutputArchive()->setNextName("collider_circle");
			serialization.GetOutputArchive()->startNode();

			serialization.SerializeInt("cType", (int)cType_);
			serialization.SerializeInt("cProperties", (int)cProperties_);
			serialization.SerializeBool("isColliding", isColliding_);
			serialization.SerializeBool("isTrigger", isTrigger_);
			serialization.SerializeFloat("radius", radius_);

			serialization.GetOutputArchive()->finishNode();
		}

		void ColliderCircle::Deserialize(Serialization& serialization)
		{
			serialization.GetInputArchive()->startNode();
			const char* member_name = serialization.GetInputArchive()->getNodeName();

			int cType;
			serialization.DeserializeInt(cType);
			cType_ = (ColliderType)cType;
			int cProp;
			serialization.DeserializeInt(cProp);
			cProperties_ = (ColliderProperties)cProp;
			serialization.DeserializeBool(isColliding_);
			serialization.DeserializeBool(isTrigger_);
			serialization.DeserializeFloat(radius_);

			serialization.GetInputArchive()->finishNode();
		}

		bool ColliderCircle::Colliding(ColliderAABB& otherCollider)
		{
			// Grab the transform of the circle
			Components::Transform* thisTransform = Parent()->GetComponent<Components::Transform>();

			// Grab the translation of the circle's transform
			glm::vec2 aPos = *thisTransform->GetTranslation();

			// Grab the AABB's translation
			glm::vec2 bPos = *otherCollider.Parent()->GetComponent<Components::Transform>()->GetTranslation();

			// calculate the normal
			glm::vec2 n = bPos - aPos;

			// and the closest point
			glm::vec2 cp = n;

			// calculate the AABB's half extents
			glm::vec2 he;
			he.x = otherCollider.GetWidth() / 2.0f;
			he.y = otherCollider.GetHeight() / 2.0f;

			// Calculate the closest points
			cp.x = glm::clamp(cp.x, -he.x, he.x);
			cp.y = glm::clamp(cp.y, -he.y, he.y);

			bool intersect = false;

			// if the closest point is the same as the normal
			if (cp == n)
			{
				// we're intersecting
				intersect = true;

				// if the normal of the x is larger than the y
				if (glm::abs(n.x) > glm::abs(n.y))
				{
					// and the closet point on the x is positive
					if (cp.x > 0)
					{
						cp.x = he.x;	// closest point is positive half extent
					}
					else cp.x = -he.x;	// or negative half extent
				}
				else
				{
					// if the closest point is positive on the y
					if (cp.y > 0)
					{
						cp.y = he.y;	// closest point is positive half extent
					}
					else cp.y = -he.y;	// or negative half extent
				}
			}

			// Calculate the new normal
			glm::vec2 normal = n - cp;

			// Grab the length of the normal
			float d1 = (float)normal.length();

			// Square it
			float d2 = d1 * d1;

			// Grab our radius
			float r1 = GetRadius();

			// Square it
			float r2 = r1 * r1;

			// if the distance squared is greater than the radius squared
			if (d2 > r2 && intersect == false)
			{
				return false;	// we're not colliding
			}

			// otherwise
			if (intersect == true)
			{
				// Set the current intersect
				currentIntersect_ = cp;

				// Set the current normal
				currentNormal_ = glm::normalize(currentIntersect_ - aPos) * -1.0f;

				// Offset the circle
				thisTransform->SetTranslation(glm::vec3(aPos + currentNormal_, 0));
				return true;
			}
			return false;
		}

		bool ColliderCircle::Colliding(ColliderOOBB& otherCollider)
		{
			// Call the OOBB's colliding logic
			return otherCollider.Colliding(*this);
		}

		bool ColliderCircle::Colliding(Collider& collision)
		{
			// Call the default colliding logic
			return collision.Colliding(*this);
		}

		Components::Component* ColliderCircle::Clone(void) const
		{
			// Clones the collider circle
			return new ColliderCircle(*this);
		}

		float ColliderCircle::GetRadius()
		{
			// return the radius value
			return radius_;
		}

		void ColliderCircle::SetRadius(float newR)
		{
			// set the new radius
			radius_ = newR;
		}

		//==========================================//
		//					AABB					//
		//==========================================//
		ColliderAABB::ColliderAABB()
		{
			width_ = 40.0f;		// Default width (x-value)
			height_ = 40.0f;	// Default height (y-value)
			cType_ = ctAABB;	// AABB collider
			vertices_.resize(4);	// Verticies size (used for OOBB collision)

			// Set the verticies to be the four corners
			vertices_[0] = glm::vec2(-0.5f, 0.5f);
			vertices_[1] = glm::vec2(0.5f, 0.5f);
			vertices_[2] = glm::vec2(0.5f, -0.5f);
			vertices_[3] = glm::vec2(-0.5f, -0.5f);
		}

		ColliderAABB::ColliderAABB(const ColliderAABB& other)
			: width_(other.width_)
			, height_(other.height_)
		{
			cType_ = ctAABB;		// AABB collider
			cProperties_ = other.cProperties_;
			vertices_.resize(4);	// Verticies size (used for OOBB collision)

			// Set the verticies to be the four corners
			vertices_[0] = glm::vec2(-0.5f, 0.5f);
			vertices_[1] = glm::vec2(0.5f, 0.5f);
			vertices_[2] = glm::vec2(0.5f, -0.5f);
			vertices_[3] = glm::vec2(-0.5f, -0.5f);
		}

		ColliderAABB::~ColliderAABB()
		{
			// Call the defualt destructor
			Collider::~Collider();
		}

		void ColliderAABB::Serialize(Serialization& serialization)
		{
			serialization.GetOutputArchive()->setNextName("collider_aabb");
			serialization.GetOutputArchive()->startNode();

			serialization.SerializeInt("cType", (int)cType_);
			serialization.SerializeInt("cProperties", (int)cProperties_);
			serialization.SerializeBool("isColliding", isColliding_);
			serialization.SerializeBool("isTrigger", isTrigger_);
			serialization.SerializeFloat("width", width_);
			serialization.SerializeFloat("height", height_);

			float x = cOffset_.x;
			float y = cOffset_.y;
			serialization.SerializeFloat("offset_x", x);
			serialization.SerializeFloat("offset_y", y);

			serialization.GetOutputArchive()->finishNode();
		}

		void ColliderAABB::Deserialize(Serialization& serialization)
		{
			serialization.GetInputArchive()->startNode();

			int cType;
			serialization.DeserializeInt(cType);
			cType_ = (ColliderType)cType;
			int cProp;
			serialization.DeserializeInt(cProp);
			cProperties_ = (ColliderProperties)cProp;
			serialization.DeserializeBool(isColliding_);
			serialization.DeserializeBool(isTrigger_);
			serialization.DeserializeFloat(width_);
			serialization.DeserializeFloat(height_);

			serialization.DeserializeFloat(cOffset_.x);
			serialization.DeserializeFloat(cOffset_.y);

			serialization.GetInputArchive()->finishNode();
		}

		void ColliderAABB::Render()
		{
			Components::Transform* trans = parent_->GetComponent<Components::Transform>();
			Debug::DrawRect(*trans->GetTranslation() + glm::vec3(cOffset_, 0.0f)
				, glm::vec3(GetWidth() / 2, GetHeight() / 2, 0), 0, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		bool ColliderAABB::Colliding(Collider& otherCollider)
		{
			// pass it to the appropriate function
			return otherCollider.Colliding(*this);
		}

		bool ColliderAABB::Colliding(ColliderCircle& otherCollider)
		{
			// Abbreviations:
			// n = vector between the 2 midpoints
			// he = half extents
			// cp = Closest point

			// Grab the AABB's transform
			Components::Transform* thisTransform = Parent()->GetComponent<Components::Transform>();

			// Grab its translation
			glm::vec2 aPos = *Parent()->GetComponent<Components::Transform>()->GetTranslation();

			// Grab the Circle's translation
			glm::vec2 bPos = *otherCollider.Parent()->GetComponent<Components::Transform>()->GetTranslation();


			// vector from midpoint of each collider
			glm::vec2 n = bPos - aPos;

			// closest point on AABB to the circle
			glm::vec2 cp = n;

			// Calculate the half extents of the AABB
			glm::vec2 he;
			he.x = width_ / 2;
			he.y = height_ / 2;

			// Calculate the closest point
			cp.x = glm::clamp(cp.x, -he.x, he.x);
			cp.y = glm::clamp(cp.y, -he.y, he.y);

			bool intersect = false;

			// if the closest point is the normal
			if (cp == n)
			{
				intersect = true;

				if (glm::abs(n.x) > glm::abs(n.y))	// closest axis to the point
				{
					if (cp.x > 0)		// if it's above our x-axis
					{
						cp.x = he.x;	// closest point is the positive half extent
					}
					else cp.x = -he.x;	// or negative half extent
				}
				else
				{
					if (cp.y > 0)		// if it's above (right) our y-axis
					{
						cp.y = he.y;	// closest point is the positive half extent
					}
					else cp.y = -he.y;	// or negative
				}
			}

			// new normal
			glm::vec2 normal = n - cp;

			// normal length
			float d1 = (float)normal.length();

			// normal length squared
			float d2 = d1 * d1;

			// radius
			float r1 = otherCollider.GetRadius();

			// radius squared
			float r2 = r1 * r1;

			// if the 2 objects are lined up but not intersecting
			if (d2 > r2 && intersect == false)
			{
				// they're not colliding
				return false;
			}

			if (intersect == true)
			{
				// set our intersection point
				currentIntersect_ = cp;

				// set our normal
				currentNormal_ = glm::normalize(currentIntersect_ - aPos) * -1.0f;

				// set the new transform
				thisTransform->SetTranslation(glm::vec3(aPos + currentNormal_, 0));
				// note: not doing this results in collision being detected for multiple frames, which is big no no
				return true;
			}
			return false;
		}

		bool ColliderAABB::Colliding(ColliderAABB& otherCollider)
		{
			// Grab both collider's RigidBodies
			Physics::RigidBody* thisRB = parent_->GetComponent<Physics::RigidBody>();
			Physics::RigidBody* otherRB = otherCollider.parent_->GetComponent<Physics::RigidBody>();

			// Grab the first collider's transform and both collider's translations
			Components::Transform* thisTransform = Parent()->GetComponent<Components::Transform>();
			glm::vec3 aPos = *Parent()->GetComponent<Components::Transform>()->GetTranslation() + glm::vec3(cOffset_, 0.0f);
			glm::vec3 bPos = *otherCollider.Parent()->GetComponent<Components::Transform>()->GetTranslation() + glm::vec3(otherCollider.cOffset_, 0.0f);

			//float aEnd = (this->width_ * 0.5f) - xBound_;
			//float bEnd = (otherCollider.width_ * 0.5f) - xBound_;

			if (this->cProperties_ == cpPlayer && otherCollider.cProperties_ == cpPlatform && thisRB && otherRB)
			{
				float awid = width_ * 0.5f;
				float ahyt = height_ * 0.5f;
				float bwid = otherCollider.width_ * 0.5f;
				float bhyt = otherCollider.height_ * 0.5f;

				if (aPos.y - ahyt < (bPos.y + bhyt + epsilon_) &&
					aPos.y - ahyt >(bPos.y + bhyt - epsilon_))
				{
					if (aPos.x - awid < bPos.x + bwid &&
						aPos.x + awid > bPos.x - bwid)
					{
						thisRB->SetGrounded(true);
					}
				}
			}
			else if (this->cProperties_ == cpPlatform && otherCollider.cProperties_ == cpPlayer && thisRB && otherRB)
			{
				float awid = width_ * 0.5f;
				float ahyt = height_ * 0.5f;
				float bwid = otherCollider.width_ * 0.5f;
				float bhyt = otherCollider.height_ * 0.5f;

				if (bPos.y - bhyt < (aPos.y + ahyt + epsilon_) &&
					bPos.y - bhyt >(aPos.y + ahyt - epsilon_))
				{
					if (bPos.x - bwid < aPos.x + awid &&
						bPos.x + bwid > aPos.x - awid)
					{
						otherRB->SetGrounded(true);
					}
				}
			}

			// difference between the two collider's position
			glm::vec2 diff = aPos - bPos;

			// difference between the two collider's position
			glm::vec2 absDiff = glm::abs(diff);
			glm::vec2 signDiff = glm::sign(diff);

			if (absDiff.x < otherCollider.width_ / 2 + width_ / 2 &&	// if the left/right bounds of both objects do not exceede the difference
				absDiff.y < otherCollider.height_ / 2 + height_ / 2)	// if the top/bottom bounds do not exceede the difference
			{
				// caclulate the intersectsS
				currentIntersect_ = (aPos + bPos) / 2.0f;

				// magnitude of both colliders
				float m1 = glm::abs(diff.y / diff.x);
				float m2 = height_ / width_;

				// if the magnitude of the difference is greater than the 
				if (m1 > m2)	// x-axis
				{
					// Caclulate the penetration
					currentPenetration_ = glm::abs((aPos.y + (height_ * 0.5f) * -signDiff.y) - (bPos.y + (otherCollider.height_ * 0.5f) * signDiff.y));
					currentNormal_.x = 0;
					if (diff.y < 0)
					{
						// above
						currentNormal_.y = -1;

						// if the other RigidBody is real
						if (otherRB && otherRB->GetResolveCollision() == true)
						{
							if (this->parent_->GetLayerNumber() == -1 || this->parent_->GetLayerNumber() == 31)
							{
								if (!thisRB || thisRB->GetBody() == pStatic || this->GetColliderProperties() == ColliderProperties::cpPlatform)
								{
									//if (bPos.x + (otherCollider.width_ * 0.5f) - 5.0f >= aPos.x - (this->width_ * 0.5f)) { otherRB->SetVelocity(glm::vec2(0.0f, 100.0f)); }
									//else if (bPos.x - (otherCollider.width_ * 0.5f) + 5.0f < aPos.x - (this->width_ * 0.5f)) { otherRB->SetVelocity(glm::vec2(0.0f, 1000.0f)); }
									otherRB->SetGrounded(true);
									if (currentPenetration_ > epsilon_)
									{
										bPos -= glm::vec3(otherCollider.cOffset_, 0.0f);
										otherCollider.parent_->GetComponent<Components::Transform>()->SetTranslation(glm::vec3(bPos.x , bPos.y + currentPenetration_, bPos.z));
									}
								}
							}
						}
					}
					else
					{
						// below
						currentNormal_.y = 1;

						// if our collider has a RigidBody
						if (thisRB && thisRB->GetResolveCollision() == true)
						{
							if (otherCollider.parent_->GetLayerNumber() == -1 || otherCollider.parent_->GetLayerNumber() == 31)
							{
								if (!otherRB || otherRB->GetBody() == pStatic || otherCollider.GetColliderProperties() == ColliderProperties::cpPlatform)
								{
									//if (aPos.x + (this->width_ * 0.5f) - 5.0f < bPos.x - (otherCollider.width_ * 0.5f)) { thisRB->SetVelocity(glm::vec2(0.0f, 100.0f)); }
									//else if (aPos.x - (this->width_ * 0.5f) + 5.0f < bPos.x - (otherCollider.width_ * 0.5f)) { thisRB->SetVelocity(glm::vec2(0.0f, 1000.0f)); }
									thisRB->SetGrounded(true);
									if (currentPenetration_ > epsilon_)
									{
										aPos -= glm::vec3(this->cOffset_, 0.0f);
										this->parent_->GetComponent<Components::Transform>()->SetTranslation(glm::vec3(aPos.x, aPos.y + currentPenetration_, aPos.z));
									}
								}
							}
						}
					}

					// if the other collider has a larger width
					if (width_ < otherCollider.width_)
					{
						// intersect based off the first collider
						currentIntersect_.x = aPos.x + std::min(1.0f / m1 * height_ / 2.0f, width_ / 2.0f) * -signDiff.x;
						currentIntersect_.y = aPos.y + height_ / 2.0f * -signDiff.y;
					}
					else
					{
						// otherwise based off the second collider
						currentIntersect_.x = bPos.x - std::min(1.0f / m1 * otherCollider.height_ / 2.0f, otherCollider.width_ / 2.0f) * -signDiff.x;
						currentIntersect_.y = bPos.y - otherCollider.height_ / 2.0f * -signDiff.y;
					}
				}
				else	// y-axis
				{
					// calculate penetration
					currentPenetration_ = glm::abs((aPos.x + width_ * -signDiff.x) - (bPos.x + otherCollider.width_ * signDiff.x));
					currentNormal_.y = 0;
					if (diff.x < 0)
					{
						// right
						currentNormal_.x = -1;
					}
					else
					{
						// left
						currentNormal_.x = 1;
					}

					// if the first object's height is less than the other object's
					if (height_ < otherCollider.height_)
					{
						// intersect based off the first object
						currentIntersect_.x = aPos.x + width_ / 2.0f * -signDiff.x;
						currentIntersect_.y = aPos.y + std::min(m1 * width_ / 2.0f, height_ / 2.0f) * -signDiff.y;
					}
					else
					{
						// otherwise the second object
						currentIntersect_.x = bPos.x - otherCollider.width_ / 2.0f * -signDiff.x;
						currentIntersect_.y = bPos.y - std::min(m1 * otherCollider.width_ / 2.0f, otherCollider.height_ / 2.0f) * -signDiff.y;
					}
				}
				return true;
			}
			// not colliding at this point
			this->isColliding_ = false;
			otherCollider.isColliding_ = false;

			return false;
		}

		bool ColliderAABB::Colliding(ColliderOOBB& otherCollider)
		{
			// Pass to the OOBB's function
			return otherCollider.Colliding(*this);
		}

		Components::Component* ColliderAABB::Clone(void) const
		{
			// Clone the AABB
			return new ColliderAABB(*this);
		}

		float ColliderAABB::GetWidth() const
		{
			// return the AABB's width
			return width_;
		}

		float ColliderAABB::GetHeight() const
		{
			// return the AABB's height
			return height_;
		}

		void ColliderAABB::SetWidth(float newL)
		{
			// Set the new width
			width_ = newL;

			/*vertices_[0] = glm::vec2(-width_ * 0.5f, height_ * 0.5f);
			vertices_[1] = glm::vec2(width_ * 0.5f, height_ * 0.5f);
			vertices_[2] = glm::vec2(width_ * 0.5f, -height_ * 0.5f);
			vertices_[3] = glm::vec2(-width_ * 0.5f, -height_ * 0.5f);*/
		}

		void ColliderAABB::SetHeight(float newW)
		{
			// Set the new height
			height_ = newW;

			/*vertices_[0] = glm::vec2(-width_ * 0.5f, height_ * 0.5f);
			vertices_[1] = glm::vec2(width_ * 0.5f, height_ * 0.5f);
			vertices_[2] = glm::vec2(width_ * 0.5f, -height_ * 0.5f);
			vertices_[3] = glm::vec2(-width_ * 0.5f, -height_ * 0.5f);*/
		}

		void ColliderAABB::SetVerticies(std::vector<glm::vec2>& set)
		{
			// Set the AABB's verticies
			vertices_ = set;
		}

		std::vector<glm::vec2>& ColliderAABB::GetVertices()
		{
			// Return the AABB's verticies
			return vertices_;
		}

		/*void ColliderAABB::InitVerticies()
		{
			vertices_[0] = glm::vec2(-width_ * 0.5f, height_ * 0.5f);
			vertices_[1] = glm::vec2(width_ * 0.5f, height_ * 0.5f);
			vertices_[2] = glm::vec2(width_ * 0.5f, -height_ * 0.5f);
			vertices_[3] = glm::vec2(-width_ * 0.5f, -height_ * 0.5f);
		}*/

		//==========================================//
		//					OOBB					//
		//==========================================//

		//==============================================//
		//		Helper/local function declarations		//
		//==============================================//

		/**
		 * Separating Axis Theorem function.
		 *
		 * \param normal normal of the face on object A
		 * \param vertList all the verticies in object B
		 * \return the "best" face to check (which ever one collides the most with the other object)
		 */
		glm::vec2 SATHelp(const glm::vec2 normal, const std::vector<glm::vec2>& vertList);

		/**
		 * Gets the vector (line) between two points.
		 */
		glm::vec2 GetVec(glm::vec2 a, glm::vec2 b);

		/**
		 * tests whether or not the given value is between the two limits.
		 *
		 * \param val the value we want to check
		 * \param lower the lower limit
		 * \param upper the upper limit
		 * \return true if it's between the limits, false if it's outside
		 */
		bool Between(float val, float lower, float upper);

		/**
		 * Helper function to determine if 2 faces are intersecting.
		 *
		 * \param aMin minimum extent along axis A
		 * \param aMax maximum extent along axis A
		 * \param bMin minimum extent along axis B
		 * \param bMax maximum extent along axis B
		 * \return whether or not the two axes intersect
		 */
		bool Intersecting(float aMin, float aMax, float bMin, float bMax);

		/**
		 * Determines if a line intersects with a cirlce (OOBB vs Circle).
		 */
		bool LineIntersectCircle(glm::vec2 pointA, glm::vec2 pointB, glm::vec2 translation, float radius);

		//=================================================================//

		ColliderOOBB::ColliderOOBB() : verticies_(), normals_()
		{
			// OOBB collider
			cType_ = ctOOBB;
		}

		ColliderOOBB::~ColliderOOBB()
		{
			// Call the default collider's destructor
			Collider::~Collider();
		}

		bool ColliderOOBB::Colliding(Collider& otherCollider)
		{
			// Pass to the default collider's logic
			return otherCollider.Colliding(*this);
		}

		void ColliderOOBB::Serialize(Serialization& serialization)
		{
			serialization.GetOutputArchive()->setNextName("collider_oobb");
			serialization.GetOutputArchive()->startNode();

			serialization.SerializeInt("cType", (int)cType_);
			serialization.SerializeInt("cProperties", (int)cProperties_);
			serialization.SerializeBool("isColliding", isColliding_);
			serialization.SerializeBool("isTrigger", isTrigger_);

			serialization.SerializeInt("vertex_count", verticies_.size());

			serialization.GetOutputArchive()->setNextName("verticies");
			serialization.GetOutputArchive()->startNode();

			for (int i = 0; i < verticies_.size(); i++)
			{
				std::string buff = "vertex " + std::to_string(i);
				serialization.GetOutputArchive()->setNextName(buff.c_str());
				serialization.GetOutputArchive()->startNode();

				serialization.SerializeFloat("x", verticies_[i].x);
				serialization.SerializeFloat("y", verticies_[i].y);

				serialization.GetOutputArchive()->finishNode();
			}
			serialization.GetOutputArchive()->finishNode();

			serialization.SerializeInt("normal_count", normals_.size());

			serialization.GetOutputArchive()->setNextName("normals");
			serialization.GetOutputArchive()->startNode();

			for (int i = 0; i < normals_.size(); i++)
			{
				std::string buff = "normal " + std::to_string(i);
				serialization.GetOutputArchive()->setNextName(buff.c_str());
				serialization.GetOutputArchive()->startNode();

				serialization.SerializeFloat("x", normals_[i].x);
				serialization.SerializeFloat("y", normals_[i].y);

				serialization.GetOutputArchive()->finishNode();
			}
			serialization.GetOutputArchive()->finishNode();

			serialization.GetOutputArchive()->finishNode();
		}

		void ColliderOOBB::Deserialize(Serialization& serialization)
		{
			serialization.GetInputArchive()->startNode();
			const char* member_name = serialization.GetInputArchive()->getNodeName();

			int cType;
			serialization.DeserializeInt(cType);
			cType_ = (ColliderType)cType;
			int cProp;
			serialization.DeserializeInt(cProp);
			cProperties_ = (ColliderProperties)cProp;
			serialization.DeserializeBool(isColliding_);
			serialization.DeserializeBool(isTrigger_);

			int vertex_count;
			serialization.DeserializeInt(vertex_count);
			serialization.GetInputArchive()->startNode();

			for (int i = 0; i < vertex_count; i++)
			{
				serialization.GetInputArchive()->startNode();

				glm::vec2 v;
				serialization.DeserializeFloat(v.x);
				serialization.DeserializeFloat(v.y);

				verticies_.push_back(v);

				serialization.GetInputArchive()->finishNode();
			}
			serialization.GetInputArchive()->finishNode();

			int normal_count;
			serialization.DeserializeInt(normal_count);
			serialization.GetInputArchive()->startNode();

			for (int i = 0; i < normal_count; i++)
			{
				serialization.GetInputArchive()->startNode();

				glm::vec2 v;
				serialization.DeserializeFloat(v.x);
				serialization.DeserializeFloat(v.y);

				normals_.push_back(v);

				serialization.GetInputArchive()->finishNode();
			}
			serialization.GetInputArchive()->finishNode();

			serialization.GetInputArchive()->finishNode();
		}

		bool ColliderOOBB::Colliding(ColliderCircle& otherCollider)
		{
			/*int size = verticies_.size();
			for (int i = 0; i < size; i++)
			{

			}*/

			// loop through all the verticies
			// check each one with the LineIntersectCircle
			// if they collide at any point, gather all the information
			// current intersect = closest point on the line
			// current normal = intersect point * -1

			// Still need to implement
			return false;
		}

		bool LineIntersectCircle(glm::vec2 pointA, glm::vec2 pointB, glm::vec2 translation, float radius)
		{
			// Determine where the circle is in relation to the line
			// find the closest point from the circle to the line
			// Find the line between the closest point and the circle
			// trace through that line by the radius
			// if the closest point is less than or equal to the radius, we are colliding
			// otherwise we're not
			return false;
		}

		/**
		 * Normals for AABB colliders.
		 */
		static std::vector<glm::vec2> AABBNormals = {
			{ 0, 1 },
			{ 1, 0 },
			{ 0, -1 },
			{ -1, 0 }
		};

		/**
		 * OOBB collidng with AABB (not yet implemented).
		 *
		 * \param otherCollider AABB collider to check against
		 * \return true if colliding, false if not (not yet implemented)
		 */
		bool ColliderOOBB::Colliding(ColliderAABB& otherCollider)
		{
			// Set up AABB like OOBB. Get the faces based off the length and width
			// Check each endpoint of the AABB if it collides with OOBB
			// Same with OOBB to the AABB
			// if any endpoint intersects, gather the appropriate information
			// current intersect = closest point from intersection point onto the face (either AABB or OOBB face depending on the point that intersects)
			// current normal = current intersect * -1

			return false;
			/*int aSize = normals_.size();
			int aVert = verticies_.size();
			int bSize = AABBNormals.size();
			int bVert = otherCollider.GetVertices().size();
			glm::mat4 thisMatrix = *parent_->GetComponent<Components::Transform>()->GetMatrix();
			glm::mat4 otherMatrix = *otherCollider.Parent()->GetComponent<Components::Transform>()->GetMatrix();

			for (int i = 0; i < aSize; i++)
			{
				float aMin, aMax, bMin, bMax;
				aMin = std::numeric_limits<float>::max();
				aMax = -std::numeric_limits<float>::max();
				for (int j = 0; j < aVert; j++)
				{
					float dotVal = glm::dot(verticies_[j], normals_[j]);
					if (dotVal < aMin) aMin = dotVal;
					if (dotVal > aMax) aMax = dotVal;
				}
				//SATHelp(normals_[i], verticies_, thisMatrix, aMin, aMax);
				//SATHelp(normals_[i], otherCollider.GetVertices(), otherMatrix, bMin, bMax);
				if (!Intersecting(aMin, aMax, bMin, bMax))
				{
					return false;
				}
			}

			for (int j = 0; j < bSize; j++)
			{
				float aMin, aMax, bMin, bMax;
				//SATHelp(AABBNormals[j], otherCollider.GetVertices(), otherMatrix, aMin, aMax);
				//SATHelp(AABBNormals[j], verticies_, thisMatrix, bMin, bMax);
				if (!Intersecting(aMin, aMax, bMin, bMax))
				{
					return false;
				}
			}

			parent_->GetComponent<RigidBody>()->SetGrounded(true);
			return true;*/
		}

		bool Intersecting(float aMin, float aMax, float bMin, float bMax)
		{
			// true if the bMin is between the aMin or aMax and same fro aMin and bMin/bMax
			return Between(bMin, aMin, aMax) || Between(aMin, bMin, bMax);
		}

		bool Between(float val, float lower, float upper)
		{
			// true if the value is between the lower and upper bounds
			return lower <= val && val <= upper;
		}

		/**
		 * determint the bias (error) in calculation between two values.
		 *
		 * \param a first value to test
		 * \param b other value to test
		 * \return
		 */
		inline bool BiasError(float a, float b)
		{
			constexpr float k_biasRelative = 0.95f;	// Relative bias: potential error over a more local scale
			constexpr float k_biasAbsolute = 0.01f;	// Absolute bias: potential error over a larger scale
			return a >= ((b * k_biasRelative) + (a * k_biasAbsolute));	// used to counteract the effects of potential floating point errors (saftey measure)
		}

		float ColliderOOBB::FindAxisLeastPenetration(int* faceIndex, ColliderOOBB& A, ColliderOOBB& B)
		{
			int aSize = A.normals_.size();	// a collider's face normals
			constexpr float floatMin_ = -std::numeric_limits<float>::max();	// the minimum value of a float
			float bestDistance = floatMin_;	// the largest distance (penetration distance from A into B)
			glm::mat4 thisMatrix = *A.Parent()->GetComponent<Components::Transform>()->GetMatrix();		// A's transform matrix
			glm::mat4 otherMatrix = *B.Parent()->GetComponent<Components::Transform>()->GetMatrix();	// B's transform matrix
			glm::mat4 inverse = glm::inverse(otherMatrix);	// the inverse of the B's matrix (useful for transitioning A into B's object space)
			glm::mat4 matrix = inverse * thisMatrix;		// Translates A's cooridnates to be in terms of B's object space (with the center of B being (0,0))
			int bestIndex = 0;	// the best face index to use

			for (int i = 0; i < aSize; i++)
			{
				//float aMin, aMax, bMin, bMax;		// depricated for now 

				// translates all of A's face normals into terms of B's object space
				glm::vec2 n = glm::normalize(glm::vec2(matrix * glm::vec4(A.normals_[i], 0, 0)));

				// Calculates the furthest point in a given direction within a shape from B's verticies, running along the negative of A's normal
				// this is used to determine the best face used (the face that we know collides with the other coillider)
				glm::vec2 s = SATHelp(-n, B.verticies_);

				// Transforms A's vertex face into B's model space
				glm::vec2 v = matrix * glm::vec4(A.verticies_[i], 0, 1);

				// Determine the penetration distance from A into B in realtion to B's model space
				float d = glm::dot(n, s - v);

				// Determine if we have a new point that goes further into B
				if (d > bestDistance)
				{
					bestDistance = d;	// if so, save that point to check with later ones
					bestIndex = i;		// mark the index of the point in the list
				}
			}

			(*faceIndex) = bestIndex;	// sets the index of our best point (determined from the for loop)

			return bestDistance;	// send out the distance the closest point intersects with B

			///////////////////////////////////////////////
			//		+-------+
			//		|		|
			//		|	+---+-------+
			//		+===+===0	    |
			//			|			|
			//			|			|
			//			+-----------+
			// 0 would be the closest point
			// ====== would be the best face
			///////////////////////////////////////////////
		}

		void ColliderOOBB::FindIncidentFace(glm::vec2* v, ColliderOOBB& reference, ColliderOOBB& incident, int referenceFace)
		{
			constexpr float floatMax_ = std::numeric_limits<float>::max();	// Maximum value of a float
			glm::mat4 refMatrix = *reference.Parent()->GetComponent<Components::Transform>()->GetMatrix();	// reference's matrix
			glm::mat4 incMatrix = *incident.Parent()->GetComponent<Components::Transform>()->GetMatrix();	// incident's matrix
			glm::mat4 refToIncMatrix = glm::inverse(incMatrix) * refMatrix;	// convertint to the incident's model space
			glm::vec2 refNormal = reference.normals_[referenceFace];	// grab the correct reference normal based off what face on the reference collider we're using

			// put reference normal into the incident's space
			refNormal = refToIncMatrix * glm::vec4(refNormal, 0, 0);

			int incidentFace = 0;		// incident face's index in the vector
			float minDot = floatMax_;	// most anti-parallel face
			int incidentSize = incident.verticies_.size();	// size of our incident collider

			for (int i = 0; i < incidentSize; i++)
			{
				// dot the incident normal against the reference normal
				// (the smaller the result, the closer to anti-parallel it is)
				float dotProd = glm::dot(refNormal, incident.normals_[i]);

				// if our new face is more anti-parallel to the reference face's normal
				if (dotProd < minDot)
				{
					minDot = dotProd;	// save it to check against the next ones
					incidentFace = i;	// save its position
				}
			}

			// Once we have the most anti-parallel face, we assign its verticies
			v[0] = incMatrix * glm::vec4(incident.verticies_[incidentFace], 0, 1);
			incidentFace = (incidentFace + 1) % incident.verticies_.size();
			v[1] = incMatrix * glm::vec4(incident.verticies_[incidentFace], 0, 1);
		}

		/**
		 * Clips the incident face with adjacent faces.
		 *
		 * \param normal The normal of the reference face
		 * \param c The side we're checking
		 * \param face The incident's face
		 * \return the amount of points within the clipping plane
		 */
		int Clip(glm::vec2 normal, float c, glm::vec2* face)
		{
			// points in the clipping plane
			int sp = 0;

			// the two points of the incident face's vertex
			glm::vec2 out[3] = { face[0], face[1] };

			// Distances from each point to the plane
			float d1 = glm::dot(normal, face[0]) - c;
			float d2 = glm::dot(normal, face[1]) - c;

			// Check if they're within the clipping plane
			if (d1 <= 0.0f) out[sp++] = face[0];
			if (d2 <= 0.0f) out[sp++] = face[1];

			// if the points are on different sides of the offset
			if (d1 * d2 < 0.0f && sp < 3)
			{
				float alpha = d1 / (d1 - d2);	// Location along the clipped face
				out[sp] = face[0] + alpha * (face[1] - face[0]);	// save the points
				// face[1] - face[0] is the vertex/edge we're clipping
				// the face[0] is the start point, adding the alpha scaled by the edge gives us the point
				// along the edge that we're clipping. This adjusts the points to be outside the clipped plane if they
				// were previously inside
				++sp;	// increment clipped points
			}

			// Save our new values
			face[0] = out[0];
			face[1] = out[1];

			return sp;	// send out how many points were within the clipped plane
		}

		bool ColliderOOBB::Colliding(ColliderOOBB& otherCollider)
		{
			int faceA, faceB;	// the faces on each object we want to use

			float penetrationA = FindAxisLeastPenetration(&faceA, *this, otherCollider);	// calculate the penetration of collider a into collider b

			if (penetrationA >= 0.0f)	// if they're not colliding at all
			{
				return false;
			}

			float penetrationB = FindAxisLeastPenetration(&faceB, otherCollider, *this);	// calculate the penetration of collider b into collider a

			if (penetrationB >= 0.0f)	// if they're not penetrating at all
			{
				return false;
			}

			int referenceIndex;
			bool flip;

			ColliderOOBB* reference;
			ColliderOOBB* incident;

			if (BiasError(penetrationA, penetrationB))	// calculate out any error in calculation
			{
				reference = this;
				incident = &otherCollider;
				referenceIndex = faceA;	// the face of the reference collider
				flip = false;			// is A our 'this' collider or the other collider? Useful for clipping
			}
			else
			{
				reference = &otherCollider;
				incident = this;
				referenceIndex = faceB;		// face of reference collider
				flip = true;				// using the other collider as our reference 
			}

			// the matricies of the two colliders
			glm::mat4 refMatrix = *reference->parent_->GetComponent<Components::Transform>()->GetMatrix();
			glm::mat4 incMatrix = *incident->parent_->GetComponent<Components::Transform>()->GetMatrix();

			// Find the face on the incident collider
			glm::vec2 incidentFace[2];
			FindIncidentFace(incidentFace, *reference, *incident, referenceIndex);

			// Grabs the two reference face verticies in world space 
			glm::vec2 referenceFace[2] = {
				refMatrix * glm::vec4(reference->verticies_[referenceIndex], 0, 1),
				refMatrix * glm::vec4(reference->verticies_[(referenceIndex + 1) % reference->verticies_.size()], 0, 1)
			};

			glm::vec2 referenceSlope = glm::normalize(referenceFace[1] - referenceFace[0]);	// reference face's slope
			glm::vec2 referenceNormal = glm::vec2(-referenceSlope.y, referenceSlope.x);		// the normal of that slope

			float refC = glm::dot(referenceNormal, referenceFace[0]);		// distance from the origin
			float negSide = -glm::dot(referenceSlope, referenceFace[0]);	// left plane equation
			float posSide = glm::dot(referenceSlope, referenceFace[1]);		// right plane equation

			// clip with the left plane
			if (Clip(-referenceSlope, negSide, incidentFace) < 2)
				return false;	// accounting for errors

			// clip with the right plane
			if (Clip(referenceSlope, posSide, incidentFace) < 2)
				return false;	// accounting for errors

			// set the collisions normal based on whether or not we needed to flip the two colliders
			currentNormal_ = flip ? referenceNormal : -referenceNormal;

			int cp = 0;	// clipped points
			float separation = glm::dot(referenceNormal, incidentFace[0]) - refC;	// separation normal using the first point
			glm::vec2 contacts[2];

			if (separation <= 0.0f)	// if we lost this point from clipping
			{
				contacts[cp] = incidentFace[0];		// save the point
				currentPenetration_ = -separation;	// set the penetration
				++cp;	// increment the clipped points counter
			}
			else
			{
				currentPenetration_ = 0;	// otherwise we're good
			}

			separation = glm::dot(referenceNormal, incidentFace[1]) - refC;	// separation normal using the second point

			if (separation <= 0.0f)	// if we lost this point from clipping
			{
				contacts[cp] = incidentFace[1];		// save it
				currentPenetration_ += -separation;	// set the penetration
				++cp;	// increment the clipped points

				currentPenetration_ /= (float)cp;	// the average penetration
			}

			if (cp == 1)	// if we lost one point from clipping
			{
				currentIntersect_ = contacts[0];	// set the intersect at that point
			}
			else // otherwise if we lost 2 points from clipping
			{
				currentIntersect_ = (incidentFace[0] + incidentFace[1]) / 2.0f;	// the intersect is the midpoint of those two points
			}

			if (cp > 0)
			{
				if (glm::dot(currentNormal_, glm::vec2(0, -1)) > 0.5f)	// if the a collider is above the b collider
				{
					RigidBody* rb = otherCollider.parent_->GetComponent<RigidBody>();	// and it has a RigidBody
					if (rb)
					{
						rb->SetGrounded(true);	// it's on the ground
					}
				}
				else if (glm::dot(currentNormal_, glm::vec2(0, 1)) > 0.5f)	// if the b collider is above the a collider
				{
					RigidBody* rb = parent_->GetComponent<RigidBody>();	// and has a RigidBody
					if (rb)
					{
						rb->SetGrounded(true);	// it's grounded
					}
				}
				return true;	// we're colliding
			}

			return false;
		}

		glm::vec2 SATHelp(const glm::vec2 normal, const std::vector<glm::vec2>& vertList)	// Helper function
		{
			constexpr float floatMin_ = -std::numeric_limits<float>::max();		// minimum value of a float
			//constexpr float floatMax_ = std::numeric_limits<float>::max();		// maximum value of a float (depricated)
			float bestPenetration = floatMin_;	// the best face to use for calculation 
			float dotProd;	// dot product results
			glm::vec2 best = glm::vec2(0, 0);
			for (const glm::vec2& vertex : vertList)	// loop through all the verticies in object B
			{
				dotProd = glm::dot(vertex, normal);		// dot product them with the normal of object A

				if (dotProd > bestPenetration)			// if the current face is better than the one we had before
				{
					bestPenetration = dotProd;			// save the dot product to compare to later ones
					best = vertex;						// save the vertex
				}
			}
			return best;	// return the best face to use
		}

		void ColliderOOBB::AddPoint(glm::vec2 point)
		{
			// Add the point to the end of the vertex list
			verticies_.push_back(point);

			// Re-calculate the normals
			CalculateNormals();
		}

		void ColliderOOBB::AddPoints(std::vector<glm::vec2> points)
		{
			// grab the size of the list of new points
			int size = points.size();

			// add each one to the vertex list
			for (int i = 0; i < size; i++)
			{
				verticies_.push_back(points[i]);
			}

			// Recalculate the new normals
			CalculateNormals();
		}

		void ColliderOOBB::RemovePoint(int index)
		{
			// grab the size of the vertex list
			int size = verticies_.size();

			// if our index isn't past the size
			if (index < size)
			{
				// erase the point
				verticies_.erase(verticies_.begin() + index);

				// recalculate the normals
				CalculateNormals();
			}
		}

		void ColliderOOBB::RemovePoint(glm::vec2 point)
		{
			// grab the size of the vertex list
			int size = verticies_.size();

			// loop through it
			for (int i = 0; i < size; i++)
			{
				// if the index is the same as the point
				if (verticies_[i] == point)
				{
					// erase it
					verticies_.erase(verticies_.begin() + i);

					// recalculate the normals
					CalculateNormals();

					// break out
					return;
				}
			}
		}

		void ColliderOOBB::CalculateNormals()
		{
			// clear the current normals list
			normals_.clear();

			// grab the size of the vertex list
			int size = verticies_.size();

			// loop through it
			for (int i = 0; i < size; i++)
			{
				// vector of the two points
				glm::vec2 v;

				// if the next point is past the end
				if (i + 1 == size)
				{
					// use the last point and the first point
					v = GetVec(verticies_[i], verticies_[0]);
				}
				else
				{
					// use the current point and the next point
					v = GetVec(verticies_[i], verticies_[i + 1]);
				}

				// temp for swap
				float temp = v.x;
				// swap the x and y values
				v.x = -v.y;
				v.y = temp;

				// add the normal to the end of the list
				normals_.push_back(glm::normalize(v));
			}
		}

		glm::vec2 GetVec(glm::vec2 a, glm::vec2 b)	// Helper
		{
			// if the two points are identical, there is no vector between them
			if (a == b) return glm::vec2(0.0f, 0.0f);

			// grab the vector between the two points by subtracting the first point from the second
			glm::vec2 output = glm::vec2(b.x - a.x, b.y - a.y);

			// return it
			return output;
		}

		ColliderOOBB* ColliderOOBB::OOBBRect(float x_len, float y_len)
		{ //																			1	 _________   2
			ColliderOOBB::AddPoint(glm::vec2(-1.0f, 1.0f));	// top left						|         |
			ColliderOOBB::AddPoint(glm::vec2(1.0f, 1.0f));	// top right					|         |
			ColliderOOBB::AddPoint(glm::vec2(1.0f, -1.0f));	// bottom right					|	      |
			ColliderOOBB::AddPoint(glm::vec2(-1.0f, -1.0f));	// bottom left			 4  |_________|  3
			return this;
		}

		const std::vector<glm::vec2>& ColliderOOBB::GetVerticies() const
		{
			// return the list of OOBB verticies
			return verticies_;
		}

		void ColliderOOBB::SetVerticies(std::vector<glm::vec2> vertexList)
		{
			// Set the list of OOBB verticies
			verticies_ = vertexList;
		}

		void ColliderOOBB::Render()
		{
			// draw a wireframe around the OOBB
			Debug::DrawAsWireframe(verticies_, *parent_->GetComponent<Components::Transform>()->GetMatrix(), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		Components::Component* ColliderOOBB::Clone(void) const
		{
			// Clones the current OOBB collider
			return new ColliderOOBB(*this);
		}
	}
}
