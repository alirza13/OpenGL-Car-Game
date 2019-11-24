#include "Mesh.h"


Mesh::Mesh()
{
	
}

Mesh::Mesh(
	const std::vector<glm::vec3>& vertices,
	const std::vector<glm::vec3>& normals,
	const std::vector<glm::vec2>& uvs)
	//: vertices(vertices), uvs(uvs), normals(normals)
{
	this->vertex_size = vertices.size();

	//	Create a Vertex Buffer Object
	glGenBuffers(1, &this->vertex_buffer);

	//	Buffer vertices and enable the attribute pointer
	glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(glm::vec3),
		vertices.data(),
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(ATTR_LOCATION::POSITION);
	glVertexAttribPointer(ATTR_LOCATION::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	
	//	Create a Normal Buffer Object
	glGenBuffers(1, &this->normal_buffer);

	//	Buffer vertices and enable the attribute pointer
	glBindBuffer(GL_ARRAY_BUFFER, this->normal_buffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		normals.size() * sizeof(glm::vec3),
		normals.data(),
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(ATTR_LOCATION::NORMAL);
	glVertexAttribPointer(ATTR_LOCATION::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);


	//	Create a UV Buffer Object
	glGenBuffers(1, &this->uv_buffer);

	//	Buffer uvs and enable the attribute pointer
	glBindBuffer(GL_ARRAY_BUFFER, this->uv_buffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		uvs.size() * sizeof(glm::vec2),
		uvs.data(),
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(ATTR_LOCATION::UV);
	glVertexAttribPointer(ATTR_LOCATION::UV, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);


	// Unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setupBuffers()
{
}


Mesh::~Mesh()
{
}
