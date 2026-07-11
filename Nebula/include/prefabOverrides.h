#pragma once

#include <nlohmann/json.hpp>

namespace Nebula::PrefabOverrides
{
  /** Deep-merge @p overrides into @p base (mutates @p base). */
  void mergeJsonDeep(nlohmann::json &base, const nlohmann::json &overrides);

  /** Merge component-level overrides onto a resolved entity JSON blob. */
  nlohmann::json mergeEntityJson(const nlohmann::json &baseEntity, const nlohmann::json &overrides);

  /** Produce override JSON containing only fields that differ from @p baseEntity. */
  nlohmann::json computeEntityDiff(const nlohmann::json &baseEntity, const nlohmann::json &currentEntity);

  bool overridesContainPath(const nlohmann::json &overrides, const char *componentKey, const char *fieldKey);

} // namespace Nebula::PrefabOverrides
