/**
 * Implementation of the DynamicsWorld
 * For now just check collision between 2 Dynamic bodies
 * @author: Quan Bui
 * @version: 02/26/2020
 */

#include "P3DynamicsWorld.h"

#include <iostream>

 // Order of operations for each timestep: Collision -> apply forces -> solve constraints -> update positions
void P3DynamicsWorld::stepSimulation(double dt)
{
	// To define a plane, we need a normal and a point
	glm::vec3 surfaceNormal{ 0.0f, 1.0f, 0.0f };
	glm::vec3 surfacePoint{ 0.0f, 1.0f, 0.0f };

	// There will be another outer loop to iterate through how many iterations to reach convergence.
	// For 1 body and 1 position constraint.
	for (auto& linearTransform : mLinearTransformContainer)
	{
		// Collision detection - broad phase + near phase

		// Apply forces - gravity most likely
		linearTransform.velocity.y -= 9.81f * dt;
		linearTransform.momentum.y = linearTransform.mass * linearTransform.velocity.y;

		glm::vec3 accumulateImpulse{ 0.0f };
		
		// Check for constraint and solve it
		// MULTIPLE ITERATIONS
		// NO BOUNCE!
		int iterations = 4;
		glm::vec3 sampleVelocity = linearTransform.velocity;
		glm::vec3 samplePosition = linearTransform.position;
		while (iterations--)
		{
			// Update position of rigid body after apply forces
			samplePosition += sampleVelocity * float(dt);

			// Express the constraint in term of position. This is a position constraint.
			if (samplePosition.y - (-3.0f) < 0.0f)
			{
				// We have to modify the accumulate velocity to solve this position constraint
				// How much in the y direction that we have to push the object, this y direction can be generalize
				//  to just the surface/plane normal.
				// THE IMPULSE CAN PUSH, BUT NOT PULL.
				float signedDistance = samplePosition.y - (-3.0f);
				//accumulateImpulse += glm::vec3(0.0f, -signedDistance, 0.0f);

				// Has the constraint been satisfied
				//sampleVelocity += accumulateImpulse * linearTransform.inverseMass;
				//samplePosition += sampleVelocity * float(dt);

				samplePosition.y -= signedDistance;
				linearTransform.position = samplePosition;
				// Make current velocity zero
				accumulateImpulse = glm::length(sampleVelocity) * glm::normalize(glm::vec3(0.0f, -signedDistance, 0.0f));
			}
		}

		// Apply the final impulse
		linearTransform.velocity += accumulateImpulse * linearTransform.inverseMass;

		// Integrate change in velocity
		
		linearTransform.position += linearTransform.velocity * float(dt);
	}

	//std::cout << mLinearTransformContainer[0].position.y << std::endl;
}

void P3DynamicsWorld::addRigidBody()
{
	// Generate unique ID and add to ID container
	mBodyContainer.emplace_back(mUniqueID++);
	// Add to linear transform container
	mLinearTransformContainer.emplace_back();
	// Add to angular transform container
	mAngularTransformContainer.emplace_back();

	// TODO: Set up appropriate entity -> index and index -> entity maps. This is needed
	// in the case of removing bodies.
}

void P3DynamicsWorld::addRigidBody(float mass, glm::vec3 const& position, glm::vec3 const& velocity)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mLinearTransformContainer.emplace_back();
	LinearTransform& lastLinearTransform = mLinearTransformContainer.back();
	lastLinearTransform.mass = mass;
	lastLinearTransform.inverseMass = 1.0f / mass;
	lastLinearTransform.position = position;
	lastLinearTransform.velocity = velocity;
	lastLinearTransform.momentum = mass * velocity;

	mAngularTransformContainer.emplace_back();
}

void P3DynamicsWorld::addRigidBody(LinearTransform const& linearTransform, AngularTransform const& angularTransform)
{
	mBodyContainer.emplace_back(mUniqueID++);

	mLinearTransformContainer.emplace_back(linearTransform);	// Copy constructor will be called here.

	// Add to angular transform container
	mAngularTransformContainer.emplace_back(angularTransform);
}

// TODO
void P3DynamicsWorld::fillWorldWithBodies()
{
	glm::vec3 position(0.0f);
	float mass = 0.0f;

	for (size_t i = 0; i < mMaxCapacity; ++i)
	{
		// Append unique ID
		mBodyContainer.emplace_back(mUniqueID);
		++mUniqueID;
	}
}

void P3DynamicsWorld::bowlingGameDemo()
{
	float startingX = -2.0f;
	for (float i = 0.0f; i < 5.0f; ++i)
	{
		addRigidBody(1, glm::vec3(startingX + i, -3.0f, -15.0f), glm::vec3(0.0f));
	}
}

void P3DynamicsWorld::stackingSpheresDemo()
{
	// Stack 2 unit spheres on top of each other. Same mass.
	addRigidBody(1, glm::vec3(0.0f, 4.0f, -20.0f), glm::vec3(0.0f));
	addRigidBody(1, glm::vec3(0.0f, 5.0f, -20.0f), glm::vec3(0.0f));
}

void P3DynamicsWorld::stackingBoxesDemo()
{
	// Stack 2 unit cubes on top of each other. Same mass.
	addRigidBody(1, glm::vec3(0.0f, 0.0f, -15.0f), glm::vec3(0.0f));
	addRigidBody(1, glm::vec3(0.0f, 5.0f, -15.0f), glm::vec3(0.0f));
}

//// Might have to create bounding primitives in these functions
//bool P3DynamicsWorld::addDynamicBody(glm::vec3 const &position, BoundingVolume *boundPtr,
//	float bounce, float mass)
//{
//	if (isFull()) return false;
//
//	RigidBody *newDynamicBodyPtr{nullptr};
//
//	// Some use of polymorphism here
//	BoundingBox *boundBoxPtr = dynamic_cast<BoundingBox *>(boundPtr);
//	if (boundBoxPtr)
//	{
//		newDynamicBodyPtr= new DynamicBody(position, boundBoxPtr, bounce, mass);
//	}
//
//	BoundingSphere *boundSpherePtr = dynamic_cast<BoundingSphere *>(boundPtr);
//	if (boundSpherePtr)
//	{
//		newDynamicBodyPtr = new DynamicBody(position, boundSpherePtr, bounce, mass);
//	}
//
//	mRigidBodyPtrContainer.emplace_back(newDynamicBodyPtr);
//
//	return true;
//}
//
//bool P3DynamicsWorld::addStaticBody(glm::vec3 const &position, BoundingVolume *boundPtr, float bounce)
//{
//	if (isFull()) return false;
//
//	RigidBody *newStaticBodyPtr{nullptr};
//
//	BoundingBox *boundBoxPtr = dynamic_cast<BoundingBox *>(boundPtr);
//	if (boundBoxPtr)
//	{
//		newStaticBodyPtr = new StaticBody(position, boundBoxPtr, bounce);
//	}
//
//	BoundingSphere *boundSpherePtr = dynamic_cast<BoundingSphere *>(boundPtr);
//	if (boundSpherePtr)
//	{
//		newStaticBodyPtr = new StaticBody(position, boundSpherePtr, bounce);
//	}
//
//	mRigidBodyPtrContainer.emplace_back(newStaticBodyPtr);
//
//	return true;
//}
//
//bool P3DynamicsWorld::addKinematicBody(glm::vec3 const &position, BoundingVolume *boundPtr, float bounce)
//{
//	if (isFull()) return false;
//
//	RigidBody *newKinematicBodyPtr{nullptr};
//
//	BoundingBox *boundBoxPtr = dynamic_cast<BoundingBox *>(boundPtr);
//	if (boundBoxPtr)
//	{
//		newKinematicBodyPtr = new KinematicBody(position, boundBoxPtr, bounce);
//	}
//
//	BoundingSphere *boundSpherePtr = dynamic_cast<BoundingSphere *>(boundPtr);
//	if (boundSpherePtr)
//	{
//		newKinematicBodyPtr = new KinematicBody(position, boundSpherePtr, bounce);
//	}
//
//	mRigidBodyPtrContainer.emplace_back(newKinematicBodyPtr);
//
//	return true;
//}
//
//
//// TODO: better to use the position of the bounding volume to check for collision
//bool P3DynamicsWorld::checkCollision(DynamicBody const &firstBody, DynamicBody const &secondBody)
//{
//	glm::vec3 firstBodyPosition = firstBody.getPosition();
//	glm::vec3 secondBodyPosition = secondBody.getPosition();
//
//	BoundingBox *firstBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(firstBody.getBoundPtr());
//	BoundingBox *secondBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(secondBody.getBoundPtr());
//
//	if (firstBodyBoundBoxPtr || secondBodyBoundBoxPtr)
//	{
//		// Compare right side of first body vs left side of second body
//		//  then compare right side of second body vs left side of first body
//		bool collisionX = firstBodyPosition.x + firstBodyBoundBoxPtr->getMaxBound().x >= secondBodyPosition.x + secondBodyBoundBoxPtr->getMinBound().x &&
//			secondBodyPosition.x + secondBodyBoundBoxPtr->getMaxBound().x >= firstBodyPosition.x + firstBodyBoundBoxPtr->getMinBound().x;
//
//		// Compare up side of first body vs down side of second body
//		//  then compare up side of second body vs downside of first body
//		bool collisionY = firstBodyPosition.y + firstBodyBoundBoxPtr->getMaxBound().y >= secondBodyPosition.y + secondBodyBoundBoxPtr->getMinBound().y &&
//			secondBodyPosition.y + secondBodyBoundBoxPtr->getMaxBound().y >= firstBodyPosition.y + firstBodyBoundBoxPtr->getMinBound().y;
//
//		// Pretty much the same thing for z direction
//		bool collisionZ = firstBodyPosition.z + firstBodyBoundBoxPtr->getMaxBound().z >= secondBodyPosition.z + secondBodyBoundBoxPtr->getMinBound().z &&
//			secondBodyPosition.z + secondBodyBoundBoxPtr->getMaxBound().z >= firstBodyPosition.z + firstBodyBoundBoxPtr->getMinBound().z;
//
//		return collisionX && collisionY && collisionZ;
//	}
//
//	BoundingSphere *firstBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(firstBody.getBoundPtr());
//	BoundingSphere *secondBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(secondBody.getBoundPtr());
//
//	if (firstBodyBoundSpherePtr || secondBodyBoundSpherePtr)
//	{
//		return glm::distance(firstBodyPosition, secondBodyPosition) <= firstBodyBoundSpherePtr->getRadius()
//			|| glm::distance(firstBodyPosition, secondBodyPosition) <= secondBodyBoundSpherePtr->getRadius();
//	}
//
//	return false;
//}
//
//bool P3DynamicsWorld::checkCollision(DynamicBody const &firstBody, StaticBody const &secondBody)
//{
//	glm::vec3 firstBodyPosition = firstBody.getPosition();
//	glm::vec3 secondBodyPosition = secondBody.getPosition();
//
//	BoundingBox *firstBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(firstBody.getBoundPtr());
//	BoundingBox *secondBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(secondBody.getBoundPtr());
//
//	if (firstBodyBoundBoxPtr || secondBodyBoundBoxPtr)
//	  {
//		// Compare right side of first body vs left side of second body
//		//  then compare right side of second body vs left side of first body
//		bool collisionX = firstBodyPosition.x + firstBodyBoundBoxPtr->getMaxBound().x >= secondBodyPosition.x + secondBodyBoundBoxPtr->getMinBound().x &&
//			secondBodyPosition.x + secondBodyBoundBoxPtr->getMaxBound().x >= firstBodyPosition.x + firstBodyBoundBoxPtr->getMinBound().x;
//
//		// Compare up side of first body vs down side of second body
//		//  then compare up side of second body vs downside of first body
//		bool collisionY = firstBodyPosition.y + firstBodyBoundBoxPtr->getMaxBound().y >= secondBodyPosition.y + secondBodyBoundBoxPtr->getMinBound().y &&
//			secondBodyPosition.y + secondBodyBoundBoxPtr->getMaxBound().y >= firstBodyPosition.y + firstBodyBoundBoxPtr->getMinBound().y;
//
//		// Pretty much the same thing for z direction
//		bool collisionZ = firstBodyPosition.z + firstBodyBoundBoxPtr->getMaxBound().z >= secondBodyPosition.z + secondBodyBoundBoxPtr->getMinBound().z &&
//			secondBodyPosition.z + secondBodyBoundBoxPtr->getMaxBound().z >= firstBodyPosition.z + firstBodyBoundBoxPtr->getMinBound().z;
//
//		if (collisionX && collisionY && collisionZ)
//		{
//			std::cout << "COLLIDED!!\n";
//		}
//
//		return collisionX && collisionY && collisionZ;
//	}
//
//	BoundingSphere *firstBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(firstBody.getBoundPtr());
//	BoundingSphere *secondBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(secondBody.getBoundPtr());
//
//	if (firstBodyBoundSpherePtr || secondBodyBoundSpherePtr)
//	{
//		return glm::distance(firstBodyPosition, secondBodyPosition) <= firstBodyBoundSpherePtr->getRadius()
//			|| glm::distance(firstBodyPosition, secondBodyPosition) <= secondBodyBoundSpherePtr->getRadius();
//	}
//
//	return false;
//}
//
//// Assuming every collision is elastic for simplicity's sake.
//// In 3D collision, it matters a lot where the collision happen. This is going to be inaccurate!!
//void P3DynamicsWorld::resolveCollision(DynamicBody &firstBody, DynamicBody &secondBody)
//{
//	// We care about the current velocities and masses of both bodies when this function is invoked.
//	//  then simply set the velocities of each body accordingly to conservation of linear momentum and kinetic energy.
//	glm::vec3 firstBodyInitialVelocity = firstBody.getLinearVelocity();
//	glm::vec3 secondBodyInitialVelocity = secondBody.getLinearVelocity();
//	float firstBodyMass = firstBody.getMass();
//	float secondBodyMass = secondBody.getMass();
//
//	// Assume the bodies act like point masses. NOTE: this is obviously NOT accurate!!
//	float totalMass = firstBodyMass + secondBodyMass;
//
//	glm::vec3 firstBodyFinalVelocity = ((firstBodyMass - secondBodyMass) * firstBodyInitialVelocity + 2.f * secondBodyMass * secondBodyInitialVelocity) / totalMass;
//	glm::vec3 secondBodyFinalVelocity =  (2.f * firstBodyMass * firstBodyInitialVelocity + (secondBodyMass - firstBodyMass) * secondBodyInitialVelocity) / totalMass;
//
//	firstBody.setLinearVelocity(firstBodyFinalVelocity);
//	secondBody.setLinearVelocity(secondBodyFinalVelocity);
//}
//
//// TODO: the StaticBody here is a plane for simplicity's sake.
//void P3DynamicsWorld::resolveCollision(DynamicBody &dynamicBody, StaticBody &staticBody)
//{
//	// This is hardcoded for demo. Not general for most cases at all.
//	//  The physics object would loose a small percentage of velocity to simulate energy loss on impact
//	glm::vec3 dynamicBodyFinalVelocity{ dynamicBody.getLinearVelocity().x,
//		-0.75f * dynamicBody.getLinearVelocity().y, dynamicBody.getLinearVelocity().z };
//
//	dynamicBody.setLinearVelocity(dynamicBodyFinalVelocity);
//}