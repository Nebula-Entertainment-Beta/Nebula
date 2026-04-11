/**
 * @file camera2D.h
 * @brief Reserved for a **2D orthographic camera** (sprite games, UI, tile maps).
 *
 * A 2D camera typically builds a view matrix (pan/scale) and an orthographic projection so that
 * world units (e.g. pixels or meters) map to the screen without perspective. Shaders multiply
 * vertex positions by **projection × view** (and optionally a per-object model matrix).
 *
 * @note Implementation is not filled in yet — add it when a game needs scrolling or zoom.
 */
#pragma once
