/**
 * @file component_registry.h
 * @brief Single list of scene component types — drives storage, JSON, and future editor.
 */
#pragma once

#define NEBULA_COMPONENTS_WITH_INDEX(X) \
  X(TransformComponent, 0)              \
  X(MeshRendererComponent, 1)           \
  X(CameraComponent, 2)                 \
  X(ScriptComponent, 3)                 \
  X(TagComponent, 4)                    \
  X(RigidBodyComponent, 5)              \
  X(ColliderComponent, 6)               \
  X(followTargetComponent, 7)

#define NEBULA_COMPONENT_APPLY(Type, Index) Type
#define NEBULA_COMPONENTS(X) NEBULA_COMPONENTS_WITH_INDEX(NEBULA_COMPONENT_JOIN)

#define NEBULA_COMPONENT_JOIN(Type, Index) X(Type)
