#include "register_Script.h"

#include <cstddef>
#include "scriptFields.h"
#include "script_Registry.h"
#include "player_Script.h"
#include "ground_Script.h"
#include "enemy_Script.h"
#include "mainCamera_Script.h"
#include "combatDirector.h"
#include "waveSpawner.h"
#include "traversalDirector.h"
#include "bouncePad_Script.h"
#include "windVolume_Script.h"

namespace Nimbus
{
    void registerAllGameplayScripts(Nebula::ScriptRegistry &registry,
                                    Nebula::ScriptFieldRegistry &fieldRegistry)
    {
        static const Nebula::ScriptFieldDescriptor playerFields[] = {
            {.name = "moveSpeed", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 3.f},
        };
        fieldRegistry.registerFields("Player", playerFields, std::size(playerFields));
        registry.registerScript("Player", []
                                { return std::make_unique<PlayerScript>(); });

        registry.registerScript("Ground", []
                                { return std::make_unique<groundScript>(); });

        static const Nebula::ScriptFieldDescriptor cameraFields[] = {
            {.name = "lookSensitivity", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.0035f},
            {.name = "zoomSpeed", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.6f},
            {.name = "pitchMin", .type = Nebula::ScriptFieldType::Float, .defaultFloat = -1.2f},
            {.name = "pitchMax", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.65f},

        };
        fieldRegistry.registerFields("MainCamera", cameraFields, std::size(cameraFields));
        registry.registerScript("MainCamera", []
                                { return std::make_unique<MainCameraScript>(); });

        static const Nebula::ScriptFieldDescriptor traversalDirectorFields[] = {
            {.name = "gravity", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 20.f},
            {.name = "jumpSpeed", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 8.f},
            {.name = "coyoteTime", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.12f},
            {.name = "jumpBufferTime", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.12f},
            {.name = "airControlMult", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.65f},
            {.name = "fastFallMult", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 2.5f},
            {.name = "terminalVelocity", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 50.f},
            {.name = "killY", .type = Nebula::ScriptFieldType::Float, .defaultFloat = -20.f},
        };

        fieldRegistry.registerFields("TraversalDirector", traversalDirectorFields, std::size(traversalDirectorFields));
        registry.registerScript("TraversalDirector", []
                                { return std::make_unique<TraversalDirector>(); });

        static const Nebula::ScriptFieldDescriptor combatDirectorFields[] = {
            {.name = "lightDamage", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 10.f},
            {.name = "heavyDamage", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 25.f},
            {.name = "lightWindup", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.08f},
            {.name = "lightActive", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.12f},
            {.name = "lightRecovery", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.2f},
            {.name = "heavyWindup", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.18f},
            {.name = "heavyActive", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.15f},
            {.name = "heavyRecovery", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.35f},
            {.name = "hitRadius", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 1.2f},
            {.name = "playerIFrameDuration", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.4f},
            {.name = "enemyMaxHealth", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 120.f},
            {.name = "knockbackForce", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 2.f},
            {.name = "staggerDuration", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.35f},
            {.name = "enemyAttackInterval", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 1.5f},
            {.name = "enemyMoveSpeed", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 2.f},
            {.name = "aggroRange", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 12.f},
            {.name = "enemyAttackRange", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 1.5f},
            {.name = "enemiesPerWave", .type = Nebula::ScriptFieldType::Int, .defaultInt = 5},
            {.name = "spawnRadius", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 6.f},
            {.name = "timeBetweenWaves", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 3.f},
        };
        fieldRegistry.registerFields("CombatDirector", combatDirectorFields, std::size(combatDirectorFields));
        registry.registerScript("CombatDirector", []
                                { return std::make_unique<combatDirector>(); });

        static const Nebula::ScriptFieldDescriptor enemyFields[] = {
            {.name = "health", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 100.f},
            {.name = "attackDamage", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 20.f},

        };
        fieldRegistry.registerFields("Enemy", enemyFields, std::size(enemyFields));
        registry.registerScript("Enemy", []
                                { return std::make_unique<enemyScript>(); });

        static const Nebula::ScriptFieldDescriptor waveSpawnerFields[] = {
            {.name = "waveEnemies", .type = Nebula::ScriptFieldType::EntityVector, .defaultEntityVector = {}},
        };
        fieldRegistry.registerFields("WaveSpawner", waveSpawnerFields, std::size(waveSpawnerFields));
        registry.registerScript("WaveSpawner", []
                                { return std::make_unique<WaveSpawner>(); });

        static const Nebula::ScriptFieldDescriptor bouncePadFields[] = {
            {.name = "impulse", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 14.f},
        };
        fieldRegistry.registerFields("BouncePad", bouncePadFields, std::size(bouncePadFields));
        registry.registerScript("BouncePad", []
                                { return std::make_unique<BouncePadScript>(); });

        static const Nebula::ScriptFieldDescriptor windVolumeFields[] = {
            {.name = "liftSpeed", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 6.f},
        };
        fieldRegistry.registerFields("WindVolume", windVolumeFields, std::size(windVolumeFields));
        registry.registerScript("WindVolume", []
                                { return std::make_unique<WindVolumeScript>(); });
    }
}
