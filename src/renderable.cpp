#include "renderable.h"

namespace game {
	Renderable::Renderable(std::string name, bool blending) {
		name_ = name;
		blending_ = blending;
	}

	// Get name of node
	const std::string Renderable::GetName(void) const {
		return name_;
	}

	void Renderable::setEntity(Entity* e) {
		colisionBox_ = e;
	}

	Entity* Renderable::getEntity() const {
		return colisionBox_;
	}

}