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

}