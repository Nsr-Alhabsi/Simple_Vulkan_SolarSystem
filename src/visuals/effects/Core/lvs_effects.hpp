#pragma once

// ============================================================
//  INCLUDES
// ============================================================

#include "../../../utils/easing_functions.hpp"
#include "../../../models/lvs_game_object.hpp"

#include <glm/glm.hpp>

namespace lvs {

// ============================================================
//  FORWARD DECLARATIONS
// ============================================================

class LvsEffectManager;

// ============================================================
//  CLASS DECLARATION
// ============================================================

/**
 * @brief User-facing handle for creating and controlling a single particle effect.
 *
 * LvsEffects wraps LvsEffectManager to provide a simple per-effect interface.
 * Call setEffect() to submit an effectProperties configuration; the returned
 * SoA slot index is stored in m_EffectIdx and can also be retrieved via
 * getEffectIdx(). pauseEffect(), continueEffect(), and deleteEffect() all
 * take an explicit index so a single LvsEffects instance can optionally manage
 * effects other than its own.
 */
class LvsEffects {
public:

    // ============================================================
    //  STRUCTS / TYPES
    // ============================================================

    /**
     * @brief Configuration block that fully describes a particle effect.
     *
     * Fill in the desired fields and pass the struct to setEffect(). Fields
     * that are not set retain their defaults. All per-field documentation is
     * preserved below exactly as authored.
     */
    struct effectProperties {

        // ------------------------------------------------------------
        //  General
        // ------------------------------------------------------------

        /// @brief Easing curve applied to the overall effect's main interpolation.
        /// @note Defaults to LINEAR. Applies globally unless overridden by a per-property ease field.
        LvsEasingFunctions::EaseType EASE{LvsEasingFunctions::LINEAR};

        // ------------------------------------------------------------
        //  Particle reference
        // ------------------------------------------------------------

        /// @brief Pointer to the game object used as the particle template for this effect.
        /// @note Must not be null before the effect is played. Ownership is not transferred.
        LvsGameObject* particle{nullptr};

        // ------------------------------------------------------------
        //  Spawn / Emission
        // ------------------------------------------------------------

        /// @brief World-space position where particles travel toward (or end up at) over their lifetime.
        /// @note Units match the world coordinate system (NDC-derived or scene units).
        glm::vec2 particle_ending_position{0.f, 0.f};

        /// @brief Radius of the circular emission area around the emitter origin.
        /// @note 0 emits from a single point. Units are scene/world units.
        float emission_radius{0.f};

        /// @brief Angular span of the emission cone, in degrees.
        /// @note 360 emits in all directions. Values less than 360 create a directed arc.
        float emission_arc{360.f};

        /// @brief Rotational offset applied to the emission arc, in degrees.
        /// @note 0 means the arc is centered on the positive-X axis. Positive values rotate counter-clockwise.
        float emission_arc_offset{0.f};

        /// @brief When true, particles spawn on the outer edge of emission_radius rather than inside it.
        bool emit_from_edge{false};

        /// @brief Number of particles spawned per second.
        /// @note 0 disables continuous emission. Use burst_mode for instantaneous spawning.
        float spawn_rate{0.f};

        // @brief Used only when particle_duration is -1 For all other cases the limit is derived purely from existing properties:
        int max_presistent_particles{200};

        /// @brief Total number of times the effect replays after the first run.
        /// @note 1 means the effect plays once (no repetition). -1 loops indefinitely.
        int repetition{1};

        /// @brief When true, the effect plays in reverse on alternating repetitions (ping-pong looping).
        bool reverse_on_finish{false};

        /// @brief Tells Whether the effect is active or not
        bool active{true};

        float spawn_accumulator{0.f};

        // ------------------------------------------------------------
        //  Motion / Physics
        // ------------------------------------------------------------

        /// @brief Speed of each particle at the start of its lifetime, in world units per second.
        /// @note Interpolated toward particle_velocity_end over the particle's duration.
        float particle_velocity_start{1.f};

        /// @brief Speed of each particle at the end of its lifetime, in world units per second.
        /// @note Interpolated from the particle's initial velocity toward this value over its duration.
        float particle_velocity_end{1.f};

        /// @brief Constant acceleration applied to each particle each frame, in world units per second squared.
        /// @note (0, 0) means no acceleration. Acts additively on top of gravity_strength.
        glm::vec2 particle__acceleration{0.f, 0.f};

        /// @brief Downward gravitational pull applied to all particles, in world units per second squared.
        /// @note 0 disables gravity. Positive values pull in the negative-Y direction.
        float gravity_strength{0.f};

        /// @brief Travel direction of each particle at the end of its lifetime, in degrees.
        /// @note Interpolated from the spawn direction. 0 is the positive-X axis; angles increase counter-clockwise.
        float particle_direction_end{0.f};

        /// @brief Rotation speed of each particle around its own center, in degrees per second.
        /// @note Applied at spawn. Use particle_angular_velocity_end to interpolate toward a different rate.
        float particle_angular_velocity{0.f};

        /// @brief Rotation speed of each particle at the end of its lifetime, in degrees per second.
        /// @note Interpolated from particle_angular_velocity over the particle's duration.
        float particle_angular_velocity_end{0.f};

        /// @brief Linear drag coefficient that decelerates particles over time.
        /// @note 0 means no drag. Higher values cause faster deceleration. Applied as a per-frame velocity multiplier.
        float drag{0.f};

        /// @brief Easing curve used when interpolating particle velocity over its lifetime.
        LvsEasingFunctions::EaseType velocity_ease{LvsEasingFunctions::LINEAR};

        /// @brief Optional custom easing function for velocity interpolation.
        /// @note Takes a normalized time t in [0, 1] and returns a mapped value. Overrides velocity_ease when non-null.
        float (*velocity_custom_ease_function)(float){nullptr};

        // ------------------------------------------------------------
        //  Lifetime
        // ------------------------------------------------------------

        /// @brief Per-particle lifetime in seconds.
        /// @note -1 causes the particle to live till the effect dies. 0 would make particles die immediately.
        float particle_duration{-1.f}; // per-particle lifetime; -1 = duration of particle till effect dies

        /// @brief Maximum random deviation added to or subtracted from particle_duration, in seconds.
        /// @note 0 means all particles have identical lifetimes.
        float particle_duration_variance{0.f};

        /// @brief Maximum random delay before a particle begins its life after spawning, in seconds.
        /// @note 0 means particles start immediately. Helps stagger simultaneous spawns.
        float particle_delay_variance{0.f};

        /// @brief Duration over which a particle fades from transparent to fully opaque at the start of its life, in seconds.
        /// @note 0 disables fade-in. Must be less than or equal to particle_duration.
        float fade_in_time{0.f};

        /// @brief Duration over which a particle fades from fully opaque to transparent at the end of its life, in seconds.
        /// @note 0 disables fade-out. Must be less than or equal to particle_duration.
        float fade_out_time{0.f};

        /// @brief Easing curve applied to both fade-in and fade-out opacity transitions.
        LvsEasingFunctions::EaseType fade_ease{LvsEasingFunctions::LINEAR};

        /// @brief When true, the effect object is automatically destroyed after all repetitions complete.
        bool destroy_on_finish{true};

        // ------------------------------------------------------------
        //  Appearance
        // ------------------------------------------------------------

        /// @brief RGB color of each particle at the start of its lifetime.
        /// @note Components are in [0, 1]. Used as the base (or first gradient) color.
        glm::vec3 particle_color_start{1, 1, 1};

        /// @brief RGB color of each particle at the end of its lifetime.
        /// @note Interpolated from particle_color_start using color_ease.
        glm::vec3 particle_color_end{1, 1, 1};

        /// @brief Secondary RGB color at the start of the particle's lifetime, used as the gradient's second color.
        /// @note Only relevant when use_gradient is true.
        glm::vec3 particle_color2_start{1, 1, 1};

        /// @brief Secondary RGB color at the end of the particle's lifetime, interpolated toward from particle_color2_start.
        /// @note Only relevant when use_gradient is true.
        glm::vec3 particle_color2_end{1, 1, 1};

        /// @brief Easing curve applied when interpolating particle color (and secondary color) over its lifetime.
        LvsEasingFunctions::EaseType color_ease{LvsEasingFunctions::LINEAR};

        /// @brief When true, the particle is rendered with a two-color gradient instead of a flat color.
        /// @note Requires the shader to support gradient rendering.
        bool use_gradient{false};

        /// @brief Normalized direction vector defining the gradient axis at the start of the particle's lifetime.
        /// @note (1, 0) is a horizontal left-to-right gradient. Interpolated toward gradient_direction_end.
        glm::vec2 gradient_direction_start{1.f, 0.f};

        /// @brief Normalized direction vector defining the gradient axis at the end of the particle's lifetime.
        /// @note Interpolated from gradient_direction_start over the particle's duration.
        glm::vec2 gradient_direction_end{1.f, 0.f};

        /// @brief Opacity of each particle at the start of its lifetime.
        /// @note Range [0, 1]. 0 is fully transparent, 1 is fully opaque.
        float particle_opacity_start{1.f};

        /// @brief Opacity of each particle at the end of its lifetime.
        /// @note Range [0, 1]. Interpolated from particle_opacity_start. TODO: Requires alpha support in the shader.
        float particle_opacity_end{1.f}; //TODO: Make sure to add alpha color support in the shader

        /// @brief Scale of each particle at the end of its lifetime, as a (width, height) multiplier.
        /// @note Interpolated from the particle's initial scale. (1, 1) means no change at end.
        glm::vec2 particle_scale_end{1, 1};

        /// @brief Easing curve applied when interpolating particle scale over its lifetime.
        LvsEasingFunctions::EaseType scale_ease{LvsEasingFunctions::LINEAR};

        /// @brief Optional custom easing function for color interpolation.
        /// @note Takes normalized time t in [0, 1] and returns a mapped value. Overrides color_ease when non-null.
        float (*color_custom_ease_function)(float){nullptr};

        /// @brief Optional custom easing function for scale interpolation.
        /// @note Takes normalized time t in [0, 1] and returns a mapped value. Overrides scale_ease when non-null.
        float (*scale_custom_ease_function)(float){nullptr};

        // ------------------------------------------------------------
        //  Looping / Repetition
        // ------------------------------------------------------------

        /// @brief Delay in seconds between the end of one loop iteration and the start of the next.
        /// @note 0 means iterations restart immediately. Only meaningful when repetition > 1 or looping indefinitely.
        float loop_delay{0.f};

        /// @brief When true, all particles in a burst are spawned simultaneously rather than over time.
        bool burst_mode{false};

        /// @brief Number of particles spawned in a single burst.
        /// @note Only used when burst_mode is true. Must be >= 1.
        int burst_count{1};

        /// @brief Time in seconds between successive bursts when looping in burst mode.
        /// @note 0 means bursts fire back-to-back without delay.
        float burst_interval{0.f};

        float burst_timer{0.f};

        /// @brief Tracks how many repetitions have been completed so far at runtime.
        /// @note Managed internally by the effect system. Do not set manually before playback.
        int current_repetition{0};

        /// @brief Reamining time till delay is finished in seconds
        float loop_delay_reamining{0.f};

        // ------------------------------------------------------------
        //  Randomness / Variance
        // ------------------------------------------------------------

        /// @brief Seed for the random number generator used to apply variance fields.
        /// @note 0 typically produces a default or time-based seed depending on implementation.
        uint32_t random_seed{0};

        /// @brief Maximum random deviation added to each particle's initial speed, in world units per second.
        /// @note 0 means all particles spawn at the same speed.
        float velocity_variance{0.f};

        /// @brief Maximum random angular deviation applied to each particle's spawn direction, in degrees.
        /// @note 0 means all particles fire in the exact same direction.
        float direction_variance{0.f};

        /// @brief Maximum random deviation applied to each particle's angular velocity, in degrees per second.
        /// @note 0 means all particles spin at the same rate.
        float angular_velocity_variance{0.f};

        /// @brief Maximum random per-axis deviation applied to each particle's scale at spawn.
        /// @note (0, 0) means no scale randomness. Applied symmetrically (+/-) around the base scale.
        glm::vec2 scale_variance{0.f, 0.f};

        /// @brief Maximum random per-channel deviation applied to particle_color_start.
        /// @note (0, 0, 0) disables color randomness on the start color. Applied per RGB channel independently.
        glm::vec3 color_start_variance{0.f, 0.f, 0.f};

        /// @brief Maximum random per-channel deviation applied to particle_color_end.
        /// @note (0, 0, 0) disables color randomness on the end color. Applied per RGB channel independently.
        glm::vec3 color_end_variance{0.f, 0.f, 0.f};

        /// @brief Maximum random deviation applied to each particle's starting opacity.
        /// @note 0 disables opacity randomness. Range should keep final opacity within [0, 1].
        float opacity_variance{0.f};

        /// @brief Maximum random per-axis offset applied to each particle's spawn position, in world units.
        /// @note (0, 0) means all particles start at the exact emitter position (before emission_radius is applied).
        glm::vec2 starting_position_variance{0.f, 0.f};

        // ------------------------------------------------------------
        //  Callbacks
        // ------------------------------------------------------------

        /// @brief Arbitrary user data pointer passed into all callback functions.
        /// @note The effect system never dereferences this pointer; it is forwarded as-is to callbacks.
        void* callback_data{nullptr};

        /// @brief Called once when the effect finishes all its repetitions.
        /// @note Receives callback_data. Useful for chaining effects or signaling game logic.
        void (*on_effect_finish)(void*){nullptr};

        /// @brief Called each time a new particle is spawned.
        /// @note Receives callback_data. Useful for sound, lighting, or spawn-side effects.
        void (*on_particle_spawn)(void*){nullptr};

        /// @brief Called each time a particle expires or is destroyed.
        /// @note Receives callback_data. Useful for on-death effects like sparks or sounds.
        void (*on_particle_death)(void*){nullptr};

        /// @brief Custom easing function applied to the entire effect's main EASE interpolation.
        /// @note Takes normalized time t in [0, 1] and returns a mapped value. Overrides EASE when non-null.
        float (*EFFECT_CUSTOM_EASE_FUNCTION)(float){nullptr};
    };

    // ------------------------------------------------------------
    //  CONSTRUCTORS / DESTRUCTOR
    // ------------------------------------------------------------

    /**
     * @brief Constructs the handle, binding it to an LvsEffectManager instance.
     * @param manager The manager that owns the SoA arrays and drives all effect logic.
     */
    LvsEffects(LvsEffectManager& manager) : m_Manager{manager} {}

    // ------------------------------------------------------------
    //  PUBLIC API
    // ------------------------------------------------------------

    /**
     * @brief Submits an effectProperties configuration to the manager and starts the effect.
     *
     * Stores the returned SoA slot index in m_EffectIdx for later retrieval via getEffectIdx().
     *
     * @param effect Fully configured effectProperties to submit.
     * @return SoA slot index assigned to this effect.
     */
    int  setEffect(effectProperties effect);

    /**
     * @brief Deletes an effect by SoA slot index and frees its particle sub-range.
     *
     * If idx matches the internally stored m_EffectIdx, resets it to -1.
     *
     * @param idx SoA slot index of the effect to delete.
     */
    void deleteEffect(int idx);

    /**
     * @brief Suspends an effect so it stops spawning and updating.
     * @param idx SoA slot index of the effect to pause.
     */
    void pauseEffect(int idx);

    /**
     * @brief Resumes a previously paused effect.
     * @param idx SoA slot index of the effect to resume.
     */
    void continueEffect(int idx);

    // ------------------------------------------------------------
    //  ACCESSORS
    // ------------------------------------------------------------

    /**
     * @brief Returns the SoA slot index of the effect submitted by the most recent setEffect() call.
     * @return Slot index, or -1 if no effect has been set or it was deleted.
     */
    int getEffectIdx() const { return m_EffectIdx; }

private:

    // ------------------------------------------------------------
    //  PRIVATE MEMBERS
    // ------------------------------------------------------------

    LvsEffectManager& m_Manager;
    int m_EffectIdx{-1};
};

} // namespace lvs
