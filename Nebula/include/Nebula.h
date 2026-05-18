/**
 * @file Nebula.h
 * @brief Default single include for games built on Nebula.
 *
 * Covers: app loop, ECS, scene load/save, scripts, events, input, public math types.
 * For custom low-level rendering, include specific headers (e.g. renderer.h) or a future NebulaRender.h.
 */
#pragma once

// --- Core loop (pulls window, input, world, scheduler, scripts, renderer init, …) ---
#include "application.h"

// --- ECS & scene I/O (not all re-exported by application.h) ---
#include "component.h"
#include "tag_component.h"
#include "scene_query.h"
#include "sceneSerializer.h"

// --- Math / transforms (public API; no glm in include/) ---
#include "math_types.h"
#include "transform3D.h"
#include "camera3D.h"

// --- Events (payload types; eventBus.h comes via application.h) ---
#include "eventTypes.h"

// NebulaRender.h — only for tools / custom render code
#include "renderer.h"
#include "mesh.h"
#include "material.h"