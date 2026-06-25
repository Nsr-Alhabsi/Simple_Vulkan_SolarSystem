#pragma once

// ============================================================
//  INCLUDES
// ============================================================

#include "../../utils/easing_functions.hpp"
#include "../../models/lvs_game_object.hpp"

#include <glm/glm.hpp>

// std:
#include <memory>
#include <vector>

namespace lvs {

// ============================================================
//  STRUCTS / TYPES
// ============================================================

/**
 * @brief Structure-of-Arrays storage for all per-effect state.
 *
 * Every array is indexed by the effect's SoA slot index. Arrays are
 * allocated as fixed-size std::unique_ptr<T[]> blocks in
 * LvsEffectManager::init() and must not be resized after that point.
 * Fields mirror the effectProperties struct exactly; syncPropertiesWithSoA()
 * bridges between the two representations.
 *
 * free_slots and active_indices form the pool bookkeeping: free_slots holds
 * indices available for new effects, active_indices lists all currently
 * running effect slots iterated each frame by updateEffects().
 */
struct LvsSOAEffects {

    // --- General ---
    std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_EASE;

    // --- Particle reference ---
    std::unique_ptr<LvsGameObject*[]> effect_particle;

    // --- Spawn / Emission ---
    std::unique_ptr<glm::vec2[]> effect_particle_ending_position;
    std::unique_ptr<float[]>     effect_emission_radius;
    std::unique_ptr<float[]>     effect_emission_arc;
    std::unique_ptr<float[]>     effect_emission_arc_offset;
    std::unique_ptr<bool[]>      effect_emit_from_edge;
    std::unique_ptr<float[]>     effect_spawn_rate;
    std::unique_ptr<int[]>       effect_repetition;
    std::unique_ptr<bool[]>      effect_reverse_on_finish;
    std::unique_ptr<int[]>       effect_max_presistent_particles;
    std::unique_ptr<bool[]>      effect_active;
    std::unique_ptr<float[]>     effect_spawn_accumulator;

    // --- Motion / Physics ---
    std::unique_ptr<float[]>     effect_particle_velocity_start;
    std::unique_ptr<float[]>     effect_particle_velocity_end;
    std::unique_ptr<glm::vec2[]> effect_particle__acceleration;
    std::unique_ptr<float[]>     effect_gravity_strength;
    std::unique_ptr<float[]>     effect_particle_direction_end;
    std::unique_ptr<float[]>     effect_particle_angular_velocity;
    std::unique_ptr<float[]>     effect_particle_angular_velocity_end;
    std::unique_ptr<float[]>     effect_drag;
    std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_velocity_ease;
    std::unique_ptr<float(*[])(float)>              effect_velocity_custom_ease_function;

    // --- Lifetime ---
    std::unique_ptr<float[]> effect_particle_duration;
    std::unique_ptr<float[]> effect_particle_duration_variance;
    std::unique_ptr<float[]> effect_particle_delay_variance;
    std::unique_ptr<float[]> effect_fade_in_time;
    std::unique_ptr<float[]> effect_fade_out_time;
    std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_fade_ease;
    std::unique_ptr<bool[]>  effect_destroy_on_finish;

    // --- Appearance ---
    std::unique_ptr<glm::vec3[]> effect_particle_color_start;
    std::unique_ptr<glm::vec3[]> effect_particle_color_end;
    std::unique_ptr<glm::vec3[]> effect_particle_color2_start;
    std::unique_ptr<glm::vec3[]> effect_particle_color2_end;
    std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_color_ease;
    std::unique_ptr<bool[]>      effect_use_gradient;
    std::unique_ptr<glm::vec2[]> effect_gradient_direction_start;
    std::unique_ptr<glm::vec2[]> effect_gradient_direction_end;
    std::unique_ptr<float[]>     effect_particle_opacity_start;
    std::unique_ptr<float[]>     effect_particle_opacity_end;
    std::unique_ptr<glm::vec2[]> effect_particle_scale_end;
    std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_scale_ease;
    std::unique_ptr<float(*[])(float)>              effect_color_custom_ease_function;
    std::unique_ptr<float(*[])(float)>              effect_scale_custom_ease_function;

    // --- Looping / Repetition ---
    std::unique_ptr<float[]> effect_loop_delay;
    std::unique_ptr<bool[]>  effect_burst_mode;
    std::unique_ptr<int[]>   effect_burst_count;
    std::unique_ptr<float[]> effect_burst_interval;
    std::unique_ptr<float[]> effect_burst_timer;
    std::unique_ptr<int[]>   effect_current_repetition;
    std::unique_ptr<float[]> effect_loop_delay_remaining;

    // --- Randomness / Variance ---
    std::unique_ptr<uint32_t[]>  effect_random_seed;
    std::unique_ptr<float[]>     effect_velocity_variance;
    std::unique_ptr<float[]>     effect_direction_variance;
    std::unique_ptr<float[]>     effect_angular_velocity_variance;
    std::unique_ptr<glm::vec2[]> effect_scale_variance;
    std::unique_ptr<glm::vec3[]> effect_color_start_variance;
    std::unique_ptr<glm::vec3[]> effect_color_end_variance;
    std::unique_ptr<float[]>     effect_opacity_variance;
    std::unique_ptr<glm::vec2[]> effect_starting_position_variance;

    // --- Callbacks ---
    std::unique_ptr<void*[]>           effect_callback_data;
    std::unique_ptr<void(*[])(void*)>  effect_on_effect_finish;
    std::unique_ptr<void(*[])(void*)>  effect_on_particle_spawn;
    std::unique_ptr<void(*[])(void*)>  effect_on_particle_death;
    std::unique_ptr<float(*[])(float)> effect_EFFECT_CUSTOM_EASE_FUNCTION;

    // --- Particle control ---
    /** @brief Maximum number of simultaneously alive particles allowed for each effect slot. */
    std::unique_ptr<int[]> effect_max_simultaneous_particles;

    /** @brief Absolute base index into the flat particle pool for each effect slot. */
    std::unique_ptr<int[]> effect_particle_pool_effect;

    // --- Pool bookkeeping ---
    /** @brief Indices of effect slots currently running; iterated every frame by updateEffects(). */
    std::vector<int> active_indices;

    /** @brief Indices of effect slots available for new initializeEffect() calls. */
    std::vector<int> free_slots;
};

// ------------------------------------------------------------

/**
 * @brief Structure-of-Arrays storage for all per-particle state.
 *
 * The flat particle pool is a single contiguous allocation of size
 * max_particles_total. Each effect owns a contiguous sub-range carved out
 * by LvsEffectManager's bump allocator; the base offset for effect i is
 * stored in LvsSOAEffects::effect_particle_pool_effect[i].
 *
 * Spawn-time snapshot arrays (p_*_start) are written once by
 * initalizeParticle() and used as stable baselines for per-frame
 * interpolation in the update functions.
 */
struct LvsSOAParticles {

    // --- Bookkeeping ---
    /** @brief Whether each particle slot is currently occupied by a live particle. */
    std::unique_ptr<bool[]> p_alive;

    /** @brief SoA slot index of the effect that owns each particle. */
    std::unique_ptr<int[]>  p_effect_idx;

    /** @brief Local slot index within the owning effect's particle sub-range. */
    std::unique_ptr<int[]>  p_local_slot;

    // --- Motion state ---
    std::unique_ptr<glm::vec2[]> p_position;
    std::unique_ptr<glm::vec2[]> p_spawn_position;
    std::unique_ptr<float[]>     p_velocity;
    std::unique_ptr<float[]>     p_direction;
    std::unique_ptr<float[]>     p_angular_vel;

    // --- Visual state ---
    std::unique_ptr<glm::vec2[]> p_scale;
    std::unique_ptr<glm::vec3[]> p_color;
    std::unique_ptr<float[]>     p_opacity;

    // --- Timing ---
    std::unique_ptr<float[]> p_age;
    std::unique_ptr<float[]> p_lifetime;
    std::unique_ptr<float[]> p_delay_remaining;

    // --- Spawn-time snapshots (stable baselines for per-frame interpolation) ---
    std::unique_ptr<float[]>     p_velocity_start;
    std::unique_ptr<float[]>     p_angular_vel_start;
    std::unique_ptr<glm::vec2[]> p_scale_start;
    std::unique_ptr<glm::vec3[]> p_color_start;
    std::unique_ptr<float[]>     p_opacity_start;
};

} // namespace lvs
