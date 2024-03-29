#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <string>
#include "camera.h"
#include "entities.h"

namespace game {
	class Renderable {
	public:
		Renderable(std::string name, bool blending = false);

		virtual void Draw(Camera* camera) = 0;

		virtual void Update(void) = 0;

		// Get name of node
		const std::string GetName(void) const;

		void setEntity(Entity* e);
		Entity* getEntity() const;

	protected:
		Entity* colisionBox_ = nullptr;
		std::string name_; // Name
		bool blending_ = false;

		// Set matrices that transform the node in a shader program
		virtual void SetupShader(GLuint program) = 0;
	};
}


#endif // !RENDERABLE_H
