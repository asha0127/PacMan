#pragma once

#include "splashkit.h"
#include <string>

/**
 * @file sound_manager.h
 * @brief Sound management system for the Pac-Man game
 *
 * This file contains the SoundManager class which handles all audio operations
 * including loading, playing, and stopping sound effects based on game state.
 */

/**
 * Sound configuration constants
 * Contains all sound file names used in the game
 */
namespace SoundConfig
{
    // Base sound directory path - can be varied to change sound themes
    constexpr const char *BASE_SOUND_PATH = "Resources/Sounds/Normal/";

    // Ghost chase sounds (based on pellet percentage remaining)
    constexpr const char *GHOST_CHASE_SOUND_NAME = "ghost_chase";
    constexpr const char *GHOST_CHASE_SOUND_FILE = "ghost1.wav";
    constexpr const char *GHOST_CHASE_SOUND2_NAME = "ghost_chase2";
    constexpr const char *GHOST_CHASE_SOUND2_FILE = "ghost2.wav";
    constexpr const char *GHOST_CHASE_SOUND3_NAME = "ghost_chase3";
    constexpr const char *GHOST_CHASE_SOUND3_FILE = "ghost3.wav";
    constexpr const char *GHOST_CHASE_SOUND4_NAME = "ghost_chase4";
    constexpr const char *GHOST_CHASE_SOUND4_FILE = "ghost4.wav";
    constexpr const char *GHOST_CHASE_SOUND5_NAME = "ghost_chase5";
    constexpr const char *GHOST_CHASE_SOUND5_FILE = "ghost5.wav";

    // Pellet collection sounds
    constexpr const char *DOT1_SOUND_NAME = "dot1";
    constexpr const char *DOT1_SOUND_FILE = "dot1.wav";
    constexpr const char *DOT2_SOUND_NAME = "dot2";
    constexpr const char *DOT2_SOUND_FILE = "dot2.wav";

    // Power mode and ghost interaction sounds
    constexpr const char *GHOST_BLUE_SOUND_NAME = "ghost_blue";
    constexpr const char *GHOST_BLUE_SOUND_FILE = "ghostblue.wav";
    constexpr const char *GHOST_EAT_SOUND_NAME = "ghost_eat";
    constexpr const char *GHOST_EAT_SOUND_FILE = "ghosteat.wav";
    constexpr const char *GHOST_RETREAT_SOUND_NAME = "ghost_retreat";
    constexpr const char *GHOST_RETREAT_SOUND_FILE = "ghostretreat.wav";

    // Game state sounds
    constexpr const char *START_SOUND_NAME = "start";
    constexpr const char *START_SOUND_FILE = "start.wav";
    constexpr const char *DIE_SOUND_NAME = "die";
    constexpr const char *DIE_SOUND_FILE = "die.wav";
    constexpr const char *CUTSCENE_SOUND_NAME = "cutscene";
    constexpr const char *CUTSCENE_SOUND_FILE = "cutscene.wav";
    constexpr const char *FRUIT_SOUND_NAME = "fruit";
    constexpr const char *FRUIT_SOUND_FILE = "fruit.wav";
}

/**
 * Game mode enumeration for sound management
 * Determines which background sounds should be playing
 */
enum class GameMode
{
    STARTING,   ///< Game starting sequence (plays start.wav)
    NORMAL,     ///< Ghosts chasing Pac-Man (plays ghost chase sounds)
    POWER_MODE, ///< Pac-Man chasing scared ghosts (plays ghostblue.wav)
    GAME_OVER,  ///< Game has ended (stops all sounds, plays die.wav)
    VICTORY     ///< All pellets collected (stops all sounds)
};

/**
 * @class SoundManager
 * @brief Manages all audio operations for the Pac-Man game
 *
 * The SoundManager class provides centralized control over all game audio,
 * including background music based on game state, sound effects for game events,
 * and proper cleanup of audio resources.
 */
class SoundManager
{
public:
    /**
     * @brief Constructor - initializes sound manager with default state
     */
    SoundManager();

    /**
     * @brief Destructor - ensures proper cleanup of audio resources
     */
    ~SoundManager() = default;

    /**
     * @brief Load all sound effects required for the game
     * @return true if all sounds loaded successfully, false otherwise
     */
    bool initialize();

    /**
     * @brief Update background audio based on current game mode and pellet percentage
     * @param game_mode Current game state
     * @param pellet_percentage Percentage of pellets remaining (0-100)
     */
    void update_background_audio(GameMode game_mode, double pellet_percentage);

    /**
     * @brief Play dot collection sound (alternates between dot1 and dot2)
     */
    void play_dot_collection_sound();

    /**
     * @brief Play ghost eat sound (when Pac-Man catches a scared ghost)
     */
    void play_ghost_eat_sound();

    /**
     * @brief Play ghost retreat sound (when ghost is caught and retreating)
     */
    void play_ghost_retreat_sound();

    /**
     * @brief Play cutscene sound (when level is completed)
     */
    void play_cutscene_sound();

    /**
     * @brief Stop all background sounds (chase, power mode, start sounds)
     */
    void stop_all_background_sounds();

    /**
     * @brief Stop all sounds completely (including sound effects)
     */
    void stop_all_sounds();

    /**
     * @brief Set the base path for sound files
     * @param base_path The base directory path for sounds (e.g., "Resources/Sounds/Normal/")
     */
    void set_sound_base_path(const std::string &base_path);

    /**
     * @brief Unload all currently loaded sounds to allow reloading with different files
     */
    void unload_all_sounds();

private:
    // Sound state tracking
    bool ghost_chase_sound_playing_;        ///< Whether a ghost chase sound is currently playing
    const char *current_ghost_chase_sound_; ///< Which ghost chase sound is currently playing
    bool ghost_blue_sound_playing_;         ///< Whether ghostblue.wav is currently playing
    bool start_sound_playing_;              ///< Whether start.wav is currently playing
    bool use_dot1_sound_;                   ///< Alternates between dot1 and dot2 sounds
    std::string sound_base_path_;           ///< Base path for sound files

    /**
     * @brief Get the appropriate ghost chase sound name based on pellet percentage
     * @param pellet_percentage Percentage of pellets remaining (0-100)
     * @return Sound name constant for the appropriate chase sound
     */
    const char *get_chase_sound_for_percentage(double pellet_percentage) const;

    /**
     * @brief Stop the currently playing ghost chase sound
     */
    void stop_current_chase_sound();
};