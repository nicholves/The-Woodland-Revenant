#include "interactable_node.h"

namespace game {

InteractableNode::InteractableNode(const std::string name, const Resource *geometry, const Resource *material, const Resource* texture) : SceneNode(name, geometry, material, texture) {
    held_pos_ = glm::vec3(0, 0, -5);
    held_scale_ = glm::vec3(1, 1, 1);
    held_orientation_ = glm::angleAxis(0.0f, glm::vec3(0, 1, 0));

    world_scale_ = glm::vec3(1, 1, 1);
    world_orientation_ = glm::angleAxis(0.0f, glm::vec3(0, 1, 0));
}


InteractableNode::~InteractableNode(){
}


void InteractableNode::SetPositioning(glm::vec3 held_pos, glm::vec3 held_scale, glm::quat held_orientation, glm::vec3 world_scale, glm::quat world_orientation) {
    held_pos_ = held_pos;
    held_scale_ = held_scale;
    held_orientation_ = held_orientation;

    world_scale_ = world_scale;
    world_orientation_ = world_orientation;
}


glm::vec3 InteractableNode::GetHeldPos() {
    return held_pos_;
}


glm::vec3 InteractableNode::GetHeldScale() {
    return held_scale_;
}


glm::quat InteractableNode::GetHeldOrientation() {
    return held_orientation_;
}


glm::vec3 InteractableNode::GetWorldScale() {
    return world_scale_;
}


glm::quat InteractableNode::GetWorldOrientation() {
    return world_orientation_;
}


void InteractableNode::SetParticles(SceneNode* particles) {
    particles->SetParent(this);
    particles->SetPosition(glm::vec3(0, 0, 0));

    particles_ = particles;
}


SceneNode* InteractableNode::GetParticles() {
    return particles_;
}


void InteractableNode::Update(void){
}
            
} // namespace game
