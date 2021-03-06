#include "RenderSystem.h"

#include <iostream>
#include <vector>

#include "GLSL.h"
#include "PrototypePhysicsEngine/P3BroadPhaseCollisionDetection.h"
#include "PrototypePhysicsEngine/P3NarrowPhaseCommon.h"
#include "PrototypePhysicsEngine/P3NarrowPhaseCollisionDetection.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

void RenderSystem::init(int width, int height)
{
	initRenderPrograms(width, height);
	initMeshes();
	initDebug();
}

void RenderSystem::render(MatrixContainer const &modelMatrices, const CollisionPairGpuPackage *collisionPairs, int rigidBodyCount)
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Program const &renderProgram = mPrograms[0];

	// Bind render program
	renderProgram.bind();

	uint8_t objectIdx  = 0u;
	uint8_t meshKeyIdx = 0u;
	MeshKey shapeIdx   = QUAD;

	// Iterate through the input composite model matrices
	for (int i = 0; i < modelMatrices.size(); ++i)
	{
		glm::mat4 const &modelMatrix = modelMatrices[i];
		unsigned int redOrNo = 0u;
		unsigned int isRigid = 0u;

		isRigid = i < rigidBodyCount;

		// Check collision pair list if this mesh has collided

		GLuint progID = renderProgram.getPID();

		glUniform1ui(glGetUniformLocation(progID, "isRigid"), isRigid);
		glUniform1ui(glGetUniformLocation(progID, "redOrNo"), redOrNo);
		glUniformMatrix4fv(glGetUniformLocation(progID, "projection"),
			1, GL_FALSE, glm::value_ptr(mProjection));
		glUniformMatrix4fv(glGetUniformLocation(progID, "view"),
			1, GL_FALSE, glm::value_ptr(mView));
		glUniformMatrix4fv(glGetUniformLocation(progID, "model"),
			1, GL_FALSE, glm::value_ptr(modelMatrix));

		shapeIdx = mMeshKeys[meshKeyIdx++];

		mMeshes[shapeIdx].draw(renderProgram); // Reusing the loaded meshes

		++objectIdx;
	}

	renderProgram.unbind();
}

void RenderSystem::renderInstanced(MatrixContainer const &modelMatrices)
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Program const &renderProgram = mPrograms[NORMAL_INSTANCED];

	// Bind render program
	renderProgram.bind();

	glUniform1ui(glGetUniformLocation(renderProgram.getPID(), "redOrNo"), 0);
	glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "projection"),
		1, GL_FALSE, glm::value_ptr(mProjection));
	glUniformMatrix4fv(glGetUniformLocation(renderProgram.getPID(), "view"),
		1, GL_FALSE, glm::value_ptr(mView));

	mMeshes[CUBE].drawInstanced(renderProgram, modelMatrices.size() , modelMatrices.data());

	renderProgram.unbind();
}

void RenderSystem::renderDebug( std::vector<P3BoxCollider> const &boxColliders,
								const ManifoldGpuPackage *manifoldGpuPackage )
{
	glBindVertexArray(mDebugVao);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo));

	// Bind render program
	Program const &debugShaderProg = mPrograms[DEBUG];
	debugShaderProg.bind();

	GLuint progID = debugShaderProg.getPID();

	// Draw yellow dots - collider verts
	glUniform3f(glGetUniformLocation(progID, "vertColor"), 1.0f, 1.0f, 0.0f);
	glUniformMatrix4fv(glGetUniformLocation(progID, "projection"),
		1, GL_FALSE, glm::value_ptr(mProjection));
	glUniformMatrix4fv(glGetUniformLocation(progID, "view"),
		1, GL_FALSE, glm::value_ptr(mView));

	// Iterate through all the box colliders and batch all the vertices
	glm::vec4 batchedVertices[cBoxColliderVertCount * max_mesh_count]{};
	int colliderIdx = 0;

	for (P3BoxCollider const &boxCollider : boxColliders)
	{
		for (int vertIdx = 0; vertIdx < cBoxColliderVertCount; ++vertIdx)
		{
			batchedVertices[cBoxColliderVertCount * colliderIdx + vertIdx] = boxCollider.mVertices[vertIdx];
		}

		++colliderIdx;
	}

	// Batch draw call
	glBufferData(
		GL_ARRAY_BUFFER,
		cBoxColliderVertCount * boxColliders.size() * sizeof(glm::vec4),
		(const void *)batchedVertices,
		GL_DYNAMIC_DRAW
	);
	CHECKED_GL_CALL(glDrawArrays(GL_POINTS, 0, cBoxColliderVertCount * boxColliders.size()));

	// The size is not correct, cMaxColliderCount is just a placeholder number.
	glm::vec4 batchedContactPoints[cMaxContactPointCount * cMaxColliderCount]{};
	glm::vec4 batchedContactNormals[cMaxContactPointCount * cMaxColliderCount]{};
	int contactPointIdx = 0, contacNormalIdx = 0;

	for (int manifoldIdx = 0; manifoldIdx < manifoldGpuPackage->misc.x; ++manifoldIdx)
	{
		Manifold const &manifold = manifoldGpuPackage->manifolds[manifoldIdx];

		// Iterate through all the contact points of this manifold
		for (int k = 0; k < manifold.contactBoxIndicesAndContactCount.z; ++k)
		{
			assert(k < cMaxContactPointCount); // Prob move this to the for loop evaluation later for better fail-safe.

			batchedContactPoints[contactPointIdx++]  = manifold.contacts[k].position;
			batchedContactNormals[contacNormalIdx++] = manifold.contacts[k].position;
			batchedContactNormals[contacNormalIdx++] = manifold.contacts[k].position
													 + glm::vec4((1.1f * manifold.contactNormal.w) * glm::vec3(manifold.contactNormal), 1.0f);
		}
	}

	// Draw purple dots - contact points
	glUniform3f(glGetUniformLocation(progID, "vertColor"), 1.0f, 0.0f, 1.0f);

	glDisable(GL_DEPTH_TEST);

	glBufferData(
		GL_ARRAY_BUFFER,
		contactPointIdx * sizeof(glm::vec4),
		(const void *)batchedContactPoints,
		GL_DYNAMIC_DRAW
	);
	CHECKED_GL_CALL(glDrawArrays(GL_POINTS, 0, contactPointIdx));

	// Draw white lines - contact normals
	glUniform3f(glGetUniformLocation(progID, "vertColor"), 1.0f, 1.0f, 1.0f);

	glBufferData(
		GL_ARRAY_BUFFER,
		contacNormalIdx * sizeof(glm::vec4),
		(const void *)batchedContactNormals,
		GL_DYNAMIC_DRAW
	);
	CHECKED_GL_CALL(glDrawArrays(GL_LINES, 0, contacNormalIdx));

	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	debugShaderProg.unbind();
}

void RenderSystem::registerMeshForBody(MeshKey const &shape, unsigned int quantity)
{
	assert(mNextMeshKeyIdx + 1u < mesh_key_count && "Mesh keys at limit.");

	for (unsigned int i = 0u; i < quantity; ++i)
		mMeshKeys[mNextMeshKeyIdx++] = shape;
}

void RenderSystem::reset()
{
	mNextMeshKeyIdx = 0u;
}

void RenderSystem::initRenderPrograms(int width, int height)
{
	GLSL::checkVersion();

	glViewport(0, 0, width, height);

	// Set background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Program &renderProgram = mPrograms[NORMAL];
	renderProgram.setVerbose(true);
	renderProgram.setShaderNames(
		"../resources/shaders/vs.vert",
		"../resources/shaders/fs.frag");
	renderProgram.init();
	renderProgram.addAttribute("vertPos");
	renderProgram.addAttribute("vertNor");
	renderProgram.addAttribute("vertTex");

	Program &instanceProgram = mPrograms[NORMAL_INSTANCED];
	instanceProgram.setVerbose(true);
	instanceProgram.setShaderNames(
		"../resources/shaders/vs_instanced.vert",
		"../resources/shaders/fs.frag");
	instanceProgram.init();
	instanceProgram.addAttribute("vertPos");
	instanceProgram.addAttribute("vertNor");
	instanceProgram.addAttribute("vertTex");
}

void RenderSystem::initMeshes()
{
	std::vector<tinyobj::shape_t> TOshapes;
	std::vector<tinyobj::material_t> objMaterials;
	std::string errStr;

	bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/quad.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &quadMesh = mMeshes[mNextShapeIdx++];
		quadMesh.createShape(TOshapes[0u]);
		quadMesh.measure();
		quadMesh.init();
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/cube.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &cubeMesh = mMeshes[mNextShapeIdx++];
		cubeMesh.createShape(TOshapes[0u]);
		cubeMesh.measure();
		cubeMesh.resize();
		cubeMesh.init();
		cubeMesh.initInstancedBuffers();
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/sphere.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &sphereMesh = mMeshes[mNextShapeIdx++];
		sphereMesh.createShape(TOshapes[0u]);
		sphereMesh.measure();
		sphereMesh.resize();
		sphereMesh.init();
	}

	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, "../resources/models/bowling_pin.obj");
	if (!rc)
	{
		std::cerr << errStr;
	}
	else
	{
		Shape &bowlPinMesh = mMeshes[mNextShapeIdx++];
		bowlPinMesh.createShape(TOshapes[0u]);
		bowlPinMesh.measure();
		bowlPinMesh.resize();
		bowlPinMesh.init();
	}
}

void RenderSystem::initDebug()
{
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glGenVertexArrays(1u, &mDebugVao);
	glBindVertexArray(mDebugVao);

	glGenBuffers(1u, &mDebugVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mDebugVbo);

	glEnableVertexAttribArray(0u);
	glVertexAttribPointer(0u, 4u, GL_FLOAT, GL_FALSE, 0u, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindVertexArray(0u);

	Program &debugRenderProgram = mPrograms[DEBUG];
	debugRenderProgram.setVerbose(true);
	debugRenderProgram.setShaderNames(
		"../resources/shaders/debug.vert",
		"../resources/shaders/debug.frag");
	debugRenderProgram.init();
	debugRenderProgram.addAttribute("vertPos");
}
