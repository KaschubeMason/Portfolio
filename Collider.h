/*****************************************************************//**
 * \file   Collider.h
 * \author Mason Kaschube (mason.kaschube)
 * \param  Return to the Skyway
 * \param  GAM200AF21
 *
 * \date   September 7th 2021
 *
 * \param Copyright © 2021 DigiPen (USA) Corporation.
 *********************************************************************/

#pragma once
 //---------------------------------------------------------------------
 // Include Files
 //--------------------------------------------------------------------
#include "Component.h"
#include "Object.h"
#include <glm.hpp>
#include <map>

namespace CloudEngine
{
	namespace Physics
	{

	    //--------------------------------------------------------------------
	    // Public Class Definition
	    //--------------------------------------------------------------------
		class Collider;
		class ColliderCircle;
		class ColliderAABB;
		class ColliderOOBB;

		// Basic information regarding collisions
		struct Collision
		{
			Collider& aCollider_;		// This collider
			Collider& bCollider_;		// Other collider
			glm::vec2 intersectPoint_;	// Where the 2 objects collide. In World Coords
			glm::vec2 normal_;			// the normal vector of the object
			bool handle_;				// Did we collide?
			float penetration_;			// How far in did the collision go?

			bool operator==(Collision other) const;

		};

		enum ColliderType{
			ctDefault = 0
			,ctAABB = 1
			,ctCircle = 2
			,ctOOBB = 3
		};

		/**
		 * Used to determine the behavior done on the collider.
		 * For example, enemies don't collide with each other but if
		 * a player collides with an enemy, the level restarts
		 */
		enum ColliderProperties {
			cpDefault = 0		// treat the collider no differently
			, cpPlayer = 1
			, cpPlatform = 2
			, cpEnemy = 3
			, cpCreature = 4
			, cpCollectable = 5
			, cpWall = 6
			, cpHazard = 7
			, cpMax = 8
			, cpCheckpoint = 9
			, cpTriggerBox = 10
		};

		Collider* GetCollider(Components::GameObject*);
		// Base
		class Collider : public Components::Component, public ObjectManagement::Object
		{
		public:
			/**
			 * Base constructor for the collider.
			*/
			Collider();

			/**
			* Default destructor.
			*/
			virtual ~Collider() = 0;

			/**
			 * Checks current collider against another collider.
			 *
			 * \param otherCollider the other collider to check against
			 */
			void CheckCollision(Collider& otherCollider);

			/**
			* Returns the collider's type. (AABB or Cirlce currently)
			*/
			ColliderType GetColliderType();

			/**
			 * Returns the value of the isColliding_ bool.
			 */
			bool GetIsColliding() const;

			/**
			 * Sets whether we're colliding or not.
			 */
			void SetIsColliding(bool);

			/**
			 * Sets the properties of the collider.
			 * Is this collider a Player, Enemy, Platform, etc.?
			 */
			void SetColliderProperties(ColliderProperties);

			/**
			 * Gets the properties of the collider.
			 * Whether it's a Player, Enemy, or Platform
			 */
			ColliderProperties GetColliderProperties() const;

			/**
			 * Sets what Collider Properties to ignore resolution for.
			 *
			 * \param ignore The specific Collider Property we want ignored
			 */
			void SetCollisionMask(int ignore);

			/**
			 * Sets multiple masks.
			 *
			 * \param ignoreList A list of collider properties we want ignored
			 */
			void SetColliisonMasks(int* ignoreList);

			/**
			 * Grabs the current list of masked Colliders.
			 */
			int* GetCollisionMask();

			/**
			 * Sets whether or not this collider is a trigger
			 *
			 *\param set the bool value we want to save
			 */
			void SetIsTrigger(bool set);

			void SetOffset(glm::vec2 set) { cOffset_ = set; }
			glm::vec2 GetOffset() { return cOffset_; }

			/**
			 * Returns true if the collider is a trigger, false if it's not
			 */
			bool GetIsTrigger();

			/**
			 * Override of the Component's base Start function.
			 * Adds a collider to the collision manager's collider list
			 */
			void Start() override;

			/**
			 * Override of the Component's base Stop function.
			 * Removes the collider from the collision manager's collider list
			 */
			void Stop() override;

			/**
			 * Base function to determine if we're colliding with another collider.
			 */
			virtual bool Colliding(Collider& otherCollider) { return otherCollider.Colliding(*this); } //= 0;

			/**
			 * Are we colliding with a Circle Collider.
			 */
			virtual bool Colliding(ColliderCircle& otherCollider) = 0;

			/**
			 * Are we colliding with an AABB collider.
			 */
			virtual bool Colliding(ColliderAABB& otherCollider) = 0;

			/**
			 * Are we colliding with an OOBB collider (not yet implemented).
			 */
			virtual bool Colliding(ColliderOOBB& otherCollider) = 0;

			/**
			 * Operator overload of the equate operator. Determines if two colliders are the same
			 */
			bool operator==(const Collider& other) const;

			void SetColliderPropertiesInt(int v) { cProperties_ = (ColliderProperties)v; }
			int GetColliderPropertiesInt() { return cProperties_; }

		protected:
			ColliderType cType_;						// Type of collider (useful for when we cast to know the original type)
			ColliderProperties cProperties_;			// Used to determine how to resolve collision (is it an enemy collider, player collider, platform collider, etc.)
			bool isColliding_;							// self explanatory. Are we colliding with anything?
			glm::vec2 cOffset_ = glm::vec2(0.0f, 0.0f);
			float epsilon_ = 5.0f;
			//std::map<Collider*, Collision> contacts_;	// All the other colliders we're in contact with
			int collisionMask_[cpMax];
			bool isTrigger_;
			RTTR_ENABLE(Component)
			RTTR_REGISTRATION_FRIEND
		};

		class ColliderCircle : public Collider
		{
		public:
			/**
			 * Base constructor for Circle Collider
			 */
			ColliderCircle();

			/**
			 * Copy constructor for Circle Collider
			 * 
			 * \param other the collider to copy
			 */
			ColliderCircle(const ColliderCircle& other);

			/**
			 * Destructor for Colldier Circle
			 */
			~ColliderCircle() override;

			/**
			 * Circle vs unkown collider.
			 */
			bool Colliding(Collider& otherCollider) override;

			/**
			 * Circle vs Circle collision.
			 *
			 * \return true if colliding, false if not
			 */
			bool Colliding(ColliderCircle& otherCollider) override;

			/**
			 * Circle vs AABB collision.
			 *
			 * \return true if colliding, false if not
			 */
			bool Colliding(ColliderAABB& otherCollider) override;

			/**
			 * Circle vs OOBB collision (not yet implemented).
			 *
			 * \return true if colliding, false if not
			 */
			bool Colliding(ColliderOOBB& otherCollider) override;

			/**
			 * Returns the radius of the circle.
			 */
			float GetRadius();

			void Render() override;

			/**
			 * Set the radius of the circle.
			 */
			void SetRadius(float);

			void Serialize(Serialization& serialization);
			void Deserialize(Serialization& serialization);

			/**
			 * Override of the clone function (not currently in use).
			 */
			Components::Component* Clone(void) const override;

		private:
			float radius_;				// Circle's radius
			RTTR_ENABLE(Collider)
			RTTR_REGISTRATION_FRIEND
		};

		class ColliderAABB : public Collider
		{
		public:

			/**
			 * AABB default constructor.
			 */
			ColliderAABB();

			/**
			 * AABB copy constructor.
			 */
			ColliderAABB(const ColliderAABB& other);

			/**
			 * AABB destructor.
			 */
			~ColliderAABB() override;

			void Serialize(Serialization& serialization);
			void Deserialize(Serialization& serialization);

			/**
			 * AABB vs unknown collider.
			 *
			 * \return true if colliding, false if not
			 */
			bool Colliding(Collider& otherCollider) override;

			/**
			 * AABB vs Circle collider.
			 *
			 * \return true if colliding, false if not
			 */
			bool Colliding(ColliderCircle& otherCollider) override;

			/**
			 * AABB vs AABB collider.
			 *
			 * \return true if colliding, false if not
			 */
			bool Colliding(ColliderAABB& otherCollider) override;

			/**
			 * AABB vs OOBB collider (not yet implemented).
			 *
			 * \return true if colliding, false if not
			 */
			bool Colliding(ColliderOOBB& otherCollider) override;

			/**
			 * Override of the component's clone function
			 */
			Components::Component* Clone(void) const override;

			/**
			 * Returns the width of the AABB.
			 */
			float GetWidth() const;

			void Render() override;

			/**
			 * Returns the height of the AABB.
			 */
			float GetHeight() const;

			/**
			 * Sets the AABB's width.
			 */
			void SetWidth(float);

			/**
			 * Sets the AABB's height.
			 */
			void SetHeight(float);

			/**
			 * Gets the verticies of the AABB collider (Used for OOBB vs AABB).
			 * 
			 * \return a vector of the AABB's verticies
			 */
			std::vector<glm::vec2>& GetVertices();

			/**
			 * Sets the verticies of the AABB collider (OOBB vs AABB).
			 * 
			 * \param set the vector of arrays we want to save in
			 */
			void SetVerticies(std::vector<glm::vec2>& set);

			/**
			 * Initializes verticies (not in use).
			 */
			//void InitVerticies();

		private:
			float width_;	// x-axis
			float height_;	// y-axis	
			std::vector<glm::vec2> vertices_;	// the verticies (4 points) of the AABB collider

			RTTR_ENABLE(Collider)
			RTTR_REGISTRATION_FRIEND
		};

		class ColliderOOBB : public Collider
		{
		public:

			/**
			 * OOBB default constructor.
			 */
			ColliderOOBB();

			/**
			 * OOBB defualt destructor.
			 * 
			 */
			~ColliderOOBB() override;

			void Serialize(Serialization& serialization);
			void Deserialize(Serialization& serialization);

			/**
			 * OOBB vs unknown collider.
			 * 
			 * \param otherCollider the collider to check against
			 * \return did we collide or not
			 */
			bool Colliding(Collider& otherCollider) override;

			/**
			 * OOBB vs Circle (not yet implemented).
			 * 
			 * \param otherCollider Circle to check against
			 * \return did we collide or not
			 */
			bool Colliding(ColliderCircle& otherCollider) override;

			/**
			 * OOBB vs AABB (not yet implemented).
			 * 
			 * \param otherCollider AABB collider to check
			 * \return did we collide or not
			 */
			bool Colliding(ColliderAABB& otherCollider) override;

			/**
			 * OOBB vs OOBB.
			 * 
			 * \param otherCollider OOBB collider to check against
			 * \return did we collide or not
			 */
			bool Colliding(ColliderOOBB& otherCollider) override;

			/**
			 * Finds the face on the object with the least amount of penetration into the surface.
			 *
			 * \param faceIndex the index with the least penetration
			 * \param A main collider to check
			 * \param B secondary collider to check
			 * \return the furthest point from A that penetrates into B
			 */
			static float FindAxisLeastPenetration(int* faceIndex, ColliderOOBB& A, ColliderOOBB& B);

			/**
			 * Finds the Incident Face on the other object.
			 * Incident Face is the most anti-parallel face on a shape
			 *
			 * \param v verticies of incident face
			 * \param reference We compare the incident collider to this (think of it as the "anchor" collider)
			 * \param incident We check this against the reference collider (the incident being the one initiating contact)
			 * \param referenceFace Index of the normal found from the Bias calculations
			 */
			static void FindIncidentFace(glm::vec2* vertex, ColliderOOBB& reference, ColliderOOBB& incident, int referenceFace);

			/**
			 * Override of the Component's Clone function.
			 * 
			 * \return a cloned component
			 */
			Components::Component* Clone(void) const override;

			/**
			 * Draws wireframe around the collider.
			 */
			void Render() override;

			/**
			 * Adds a new point to the OOBB's verticies.
			 */
			void AddPoint(glm::vec2 point);	// When adding points, add clockwise to the previous point

			/**
			 * Adds multiple points to the OOBB's verticies.
			 * 
			 * \param points list of points to add
			 */
			void AddPoints(std::vector<glm::vec2> points);

			/**
			 * Removes a point from the OOBB's verticies.
			 * 
			 * \param index the index in the list we want gone
			 */
			void RemovePoint(int index);

			/**
			 * Removes a point from the OOBB's verticies.
			 * 
			 * \param point the point we want to remove
			 */
			void RemovePoint(glm::vec2 point);

			/**
			 * Calculates the normals of each face on the OOBB.
			 */
			void CalculateNormals();

			/**
			 * Creates a basic OOBB rectangle.
			 * 
			 * \param x_len width along the x-axis
			 * \param y_len height along the y-axis
			 * \return pointer to a new OOBB collider
			 */
			ColliderOOBB* OOBBRect(float x_len, float y_len);

			/**
			 * Grabs the list of all the OOBB's vertices.
			 * 
			 * \return reference to the OOBB's vertecies list
			 */
			const std::vector<glm::vec2>& GetVerticies() const;

			/**
			 * Sets the vertex list of the OOBB.
			 * 
			 * \param vertexList the vertex list we want assigned
			 */
			void SetVerticies(std::vector<glm::vec2> vertexList);

		private:
			std::vector<glm::vec2> verticies_;	// list of all the OOBB's verticies (points)
			std::vector<glm::vec2> normals_;	// list of all the face normals of the OOBB
			RTTR_ENABLE(Collider)
			RTTR_REGISTRATION_FRIEND
		};
	}
}