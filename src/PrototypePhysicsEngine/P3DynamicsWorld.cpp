/**
 * Implementation of the DynamicsWorld
 * For now just check collision between 2 Dynamic bodies
 * @author: Quan Bui
 * @version: 02/26/2020
 */

#include "P3DynamicsWorld.h"

#include <iostream>

// Might have to create bounding primitives in these functions
bool P3DynamicsWorld::addDynamicBody(glm::vec3 const &position, BoundingVolume *boundPtr,
	float bounce, float mass)
{
	if (isFull()) return false;

	RigidBody *newDynamicBodyPtr{nullptr};

	// Some use of polymorphism here
	BoundingBox *boundBoxPtr = dynamic_cast<BoundingBox *>(boundPtr);
	if (boundBoxPtr) {
		newDynamicBodyPtr= new DynamicBody(position, boundBoxPtr, bounce, mass);
	}

	BoundingSphere *boundSpherePtr = dynamic_cast<BoundingSphere *>(boundPtr);
	if (boundSpherePtr) {
		newDynamicBodyPtr = new DynamicBody(position, boundSpherePtr, bounce, mass);
	}

	rigidBodyPtrContainer.emplace_back(newDynamicBodyPtr);

	return true;
}

bool P3DynamicsWorld::addStaticBody(glm::vec3 const &position, BoundingVolume *boundPtr, float bounce)
{
	if (isFull()) return false;

	RigidBody *newStaticBodyPtr{nullptr};

	BoundingBox *boundBoxPtr = dynamic_cast<BoundingBox *>(boundPtr);
	if (boundBoxPtr) {
		newStaticBodyPtr = new StaticBody(position, boundBoxPtr, bounce);
	}

	BoundingSphere *boundSpherePtr = dynamic_cast<BoundingSphere *>(boundPtr);
	if (boundSpherePtr) {
		newStaticBodyPtr = new StaticBody(position, boundSpherePtr, bounce);
	}

	rigidBodyPtrContainer.emplace_back(newStaticBodyPtr);

	return true;
}

bool P3DynamicsWorld::addKinematicBody(glm::vec3 const &position, BoundingVolume *boundPtr, float bounce)
{
	if (isFull()) return false;

	RigidBody *newKinematicBodyPtr{nullptr};

	BoundingBox *boundBoxPtr = dynamic_cast<BoundingBox *>(boundPtr);
	if (boundBoxPtr) {
		newKinematicBodyPtr = new KinematicBody(position, boundBoxPtr, bounce);
	}

	BoundingSphere *boundSpherePtr = dynamic_cast<BoundingSphere *>(boundPtr);
	if (boundSpherePtr) {
		newKinematicBodyPtr = new KinematicBody(position, boundSpherePtr, bounce);
	}

	rigidBodyPtrContainer.emplace_back(newKinematicBodyPtr);

	return true;
}


// TODO: better to use the position of the bounding volume to check for collision
bool P3DynamicsWorld::checkCollision(DynamicBody const &firstBody, DynamicBody const &secondBody)
{
	glm::vec3 firstBodyPosition = firstBody.getPosition();
	glm::vec3 secondBodyPosition = secondBody.getPosition();

	BoundingBox *firstBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(firstBody.getBoundPtr());
	BoundingBox *secondBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(secondBody.getBoundPtr());

	if (firstBodyBoundBoxPtr || secondBodyBoundBoxPtr) {
		// Compare right side of first body vs left side of second body
		//  then compare right side of second body vs left side of first body
		bool collisionX = firstBodyPosition.x + firstBodyBoundBoxPtr->getMaxBound().x >= secondBodyPosition.x + secondBodyBoundBoxPtr->getMinBound().x &&
			secondBodyPosition.x + secondBodyBoundBoxPtr->getMaxBound().x >= firstBodyPosition.x + firstBodyBoundBoxPtr->getMinBound().x;

		// Compare up side of first body vs down side of second body
		//  then compare up side of second body vs downside of first body
		bool collisionY = firstBodyPosition.y + firstBodyBoundBoxPtr->getMaxBound().y >= secondBodyPosition.y + secondBodyBoundBoxPtr->getMinBound().y &&
			secondBodyPosition.y + secondBodyBoundBoxPtr->getMaxBound().y >= firstBodyPosition.y + firstBodyBoundBoxPtr->getMinBound().y;

		// Pretty much the same thing for z direction
		bool collisionZ = firstBodyPosition.z + firstBodyBoundBoxPtr->getMaxBound().z >= secondBodyPosition.z + secondBodyBoundBoxPtr->getMinBound().z &&
			secondBodyPosition.z + secondBodyBoundBoxPtr->getMaxBound().z >= firstBodyPosition.z + firstBodyBoundBoxPtr->getMinBound().z;

		return collisionX && collisionY && collisionZ;
	}

	BoundingSphere *firstBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(firstBody.getBoundPtr());
	BoundingSphere *secondBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(secondBody.getBoundPtr());

	if (firstBodyBoundSpherePtr || secondBodyBoundSpherePtr) {
		return glm::distance(firstBodyPosition, secondBodyPosition) <= firstBodyBoundSpherePtr->getRadius() ||
			glm::distance(firstBodyPosition, secondBodyPosition) <= secondBodyBoundSpherePtr->getRadius();
	}

	return false;
}

bool P3DynamicsWorld::checkCollision(DynamicBody const &firstBody, StaticBody const &secondBody)
{
	glm::vec3 firstBodyPosition = firstBody.getPosition();
	glm::vec3 secondBodyPosition = secondBody.getPosition();

	BoundingBox *firstBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(firstBody.getBoundPtr());
	BoundingBox *secondBodyBoundBoxPtr = dynamic_cast<BoundingBox *>(secondBody.getBoundPtr());

	if (firstBodyBoundBoxPtr || secondBodyBoundBoxPtr) {
		// Compare right side of first body vs left side of second body
		//  then compare right side of second body vs left side of first body
		bool collisionX = firstBodyPosition.x + firstBodyBoundBoxPtr->getMaxBound().x >= secondBodyPosition.x + secondBodyBoundBoxPtr->getMinBound().x &&
			secondBodyPosition.x + secondBodyBoundBoxPtr->getMaxBound().x >= firstBodyPosition.x + firstBodyBoundBoxPtr->getMinBound().x;

		// Compare up side of first body vs down side of second body
		//  then compare up side of second body vs downside of first body
		bool collisionY = firstBodyPosition.y + firstBodyBoundBoxPtr->getMaxBound().y >= secondBodyPosition.y + secondBodyBoundBoxPtr->getMinBound().y &&
			secondBodyPosition.y + secondBodyBoundBoxPtr->getMaxBound().y >= firstBodyPosition.y + firstBodyBoundBoxPtr->getMinBound().y;

		// Pretty much the same thing for z direction
		bool collisionZ = firstBodyPosition.z + firstBodyBoundBoxPtr->getMaxBound().z >= secondBodyPosition.z + secondBodyBoundBoxPtr->getMinBound().z &&
			secondBodyPosition.z + secondBodyBoundBoxPtr->getMaxBound().z >= firstBodyPosition.z + firstBodyBoundBoxPtr->getMinBound().z;

		if (collisionX && collisionY && collisionZ) {
			std::cout << "COLLIDED!!\n";
		}

		return collisionX && collisionY && collisionZ;
	}

	BoundingSphere *firstBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(firstBody.getBoundPtr());
	BoundingSphere *secondBodyBoundSpherePtr = dynamic_cast<BoundingSphere *>(secondBody.getBoundPtr());

	if (firstBodyBoundSpherePtr || secondBodyBoundSpherePtr) {
		return glm::distance(firstBodyPosition, secondBodyPosition) <= firstBodyBoundSpherePtr->getRadius() ||
			glm::distance(firstBodyPosition, secondBodyPosition) <= secondBodyBoundSpherePtr->getRadius();
	}

	return false;
}

// Assuming every collision is elastic for simplicity's sake.
// In 3D collision, it matters a lot where the collision happen. This is going to be inaccurate!!
void P3DynamicsWorld::resolveCollision(DynamicBody &firstBody, DynamicBody &secondBody)
{
	// We care about the current velocities and masses of both bodies when this function is invoked.
	//  then simply set the velocities of each body accordingly to conservation of linear momentum and kinetic energy.
	glm::vec3 firstBodyInitialVelocity = firstBody.getLinearVelocity();
	glm::vec3 secondBodyInitialVelocity = secondBody.getLinearVelocity();
	float firstBodyMass = firstBody.getMass();
	float secondBodyMass = secondBody.getMass();

	// Assume the bodies act like point masses. NOTE: this is obviously NOT accurate!!
	float totalMass = firstBodyMass + secondBodyMass;

	glm::vec3 firstBodyFinalVelocity = ((firstBodyMass - secondBodyMass) * firstBodyInitialVelocity + 2.f * secondBodyMass * secondBodyInitialVelocity) / totalMass;
	glm::vec3 secondBodyFinalVelocity =  (2.f * firstBodyMass * firstBodyInitialVelocity + (secondBodyMass - firstBodyMass) * secondBodyInitialVelocity) / totalMass;

	firstBody.setLinearVelocity(firstBodyFinalVelocity);
	secondBody.setLinearVelocity(secondBodyFinalVelocity);
}

// TODO: the StaticBody here is a plane for simplicity's sake.
void P3DynamicsWorld::resolveCollision(DynamicBody &dynamicBody, StaticBody &staticBody)
{
	// This is hardcoded for demo. Not general for most cases at all.
	//  The physics object would loose a small percentage of velocity to simulate energy loss on impact
	glm::vec3 dynamicBodyFinalVelocity{ dynamicBody.getLinearVelocity().x,
		-0.75f * dynamicBody.getLinearVelocity().y, dynamicBody.getLinearVelocity().z };

	dynamicBody.setLinearVelocity(dynamicBodyFinalVelocity);
}