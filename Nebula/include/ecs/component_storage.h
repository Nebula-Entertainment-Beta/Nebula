/**
 * @file component_storage.h
 * @brief Sparse-set storage: dense component array + sparse entity-id index.
 */
#pragma once
#include "ecs/entity.h"
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Nebula
{

  namespace detail
  {
    inline constexpr std::size_t kInvalidSparseIndex = static_cast<std::size_t>(-1);
  }

  template <typename T>
  class ComponentStorage
  {
  public:
    template <typename... Args>
    T &emplace(Entity entity, Args &&...args)
    {
      ensureSparseCapacity(entity.id);
      if (m_sparse[entity.id] != detail::kInvalidSparseIndex)
      {
        return m_dense[m_sparse[entity.id]];
      }

      const std::size_t denseIndex = m_dense.size();
      m_sparse[entity.id] = denseIndex;
      m_dense.emplace_back(std::forward<Args>(args)...);
      m_entities.push_back(entity);
      return m_dense.back();
    }

    T &get(Entity entity)
    {
      if (!contains(entity))
      {
        throw std::runtime_error("Component not found on entity");
      }
      return m_dense[m_sparse[entity.id]];
    }

    const T &get(Entity entity) const
    {
      if (!contains(entity))
      {
        throw std::runtime_error("Component not found on entity");
      }
      return m_dense[m_sparse[entity.id]];
    }

    bool contains(Entity entity) const
    {
      if (entity.id >= m_sparse.size())
      {
        return false;
      }
      const std::size_t index = m_sparse[entity.id];
      if (index == detail::kInvalidSparseIndex)
      {
        return false;
      }
      return m_entities[index] == entity;
    }

    void remove(Entity entity)
    {
      if (!contains(entity))
      {
        return;
      }

      const std::size_t index = m_sparse[entity.id];
      const std::size_t last = m_dense.size() - 1;
      if (index != last)
      {
        m_dense[index] = std::move(m_dense[last]);
        m_entities[index] = m_entities[last];
        m_sparse[m_entities[index].id] = index;
      }

      m_dense.pop_back();
      m_entities.pop_back();
      m_sparse[entity.id] = detail::kInvalidSparseIndex;
    }

    void clear()
    {
      m_dense.clear();
      m_entities.clear();
      if (!m_sparse.empty())
      {
        std::fill(m_sparse.begin(), m_sparse.end(), detail::kInvalidSparseIndex);
      }
    }

    const std::vector<T> &dense() const { return m_dense; }
    const std::vector<Entity> &entities() const { return m_entities; }

  private:
    void ensureSparseCapacity(EntityID id)
    {
      if (id < m_sparse.size())
      {
        return;
      }
      m_sparse.resize(static_cast<std::size_t>(id) + 1, detail::kInvalidSparseIndex);
    }

    std::vector<T> m_dense;
    std::vector<Entity> m_entities;
    std::vector<std::size_t> m_sparse;
  };

} // namespace Nebula
