#include "sound_manager.h"

using namespace SoundConfig;

/**
 * @brief Constructor - initializes sound manager with default state
 */
SoundManager::SoundManager()
    : ghost_chase_sound_playing_(false), current_ghost_chase_sound_(nullptr), ghost_blue_sound_playing_(false), start_sound_playing_(false), use_dot1_sound_(true), sound_base_path_(BASE_SOUND_PATH)
{
}

/**
 * @brief Load all sound effects required for the game
 * @return true if all sounds loaded successfully, false otherwise
 */
bool SoundManager::initialize()
{
    try
    {
        // Load ghost chase sounds (based on pellet percentage)
        load_sound_effect(GHOST_CHASE_SOUND_NAME, (sound_base_path_ + GHOST_CHASE_SOUND_FILE).c_str());
        load_sound_effect(GHOST_CHASE_SOUND2_NAME, (sound_base_path_ + GHOST_CHASE_SOUND2_FILE).c_str());
        load_sound_effect(GHOST_CHASE_SOUND3_NAME, (sound_base_path_ + GHOST_CHASE_SOUND3_FILE).c_str());
        load_sound_effect(GHOST_CHASE_SOUND4_NAME, (sound_base_path_ + GHOST_CHASE_SOUND4_FILE).c_str());
        load_sound_effect(GHOST_CHASE_SOUND5_NAME, (sound_base_path_ + GHOST_CHASE_SOUND5_FILE).c_str());

        // Load pellet collection sounds
        load_sound_effect(DOT1_SOUND_NAME, (sound_base_path_ + DOT1_SOUND_FILE).c_str());
        load_sound_effect(DOT2_SOUND_NAME, (sound_base_path_ + DOT2_SOUND_FILE).c_str());

        // Load power mode and ghost interaction sounds
        load_sound_effect(GHOST_BLUE_SOUND_NAME, (sound_base_path_ + GHOST_BLUE_SOUND_FILE).c_str());
        load_sound_effect(GHOST_EAT_SOUND_NAME, (sound_base_path_ + GHOST_EAT_SOUND_FILE).c_str());
        load_sound_effect(GHOST_RETREAT_SOUND_NAME, (sound_base_path_ + GHOST_RETREAT_SOUND_FILE).c_str());

        // Load game state sounds
        load_sound_effect(START_SOUND_NAME, (sound_base_path_ + START_SOUND_FILE).c_str());
        load_sound_effect(DIE_SOUND_NAME, (sound_base_path_ + DIE_SOUND_FILE).c_str());
        load_sound_effect(CUTSCENE_SOUND_NAME, (sound_base_path_ + CUTSCENE_SOUND_FILE).c_str());
        load_sound_effect(FRUIT_SOUND_NAME, (sound_base_path_ + FRUIT_SOUND_FILE).c_str());

        return true;
    }
    catch (...)
    {
        return false;
    }
}

/**
 * @brief Update background audio based on current game mode and pellet percentage
 * @param game_mode Current game state
 * @param pellet_percentage Percentage of pellets remaining (0-100)
 */
void SoundManager::update_background_audio(GameMode game_mode, double pellet_percentage)
{
    switch (game_mode)
    {
    case GameMode::STARTING:
        // Game starting sequence - play start.wav once
        if (!start_sound_playing_)
        {
            play_sound_effect(START_SOUND_NAME);
            start_sound_playing_ = true;
        }
        break;

    case GameMode::NORMAL:
    {
        // Ghosts are chasing Pac-Man - play appropriate chase sound based on pellet percentage
        const char *required_chase_sound = get_chase_sound_for_percentage(pellet_percentage);

        // If wrong sound is playing or no sound is playing, switch to correct one
        if (!ghost_chase_sound_playing_ || current_ghost_chase_sound_ != required_chase_sound)
        {
            stop_current_chase_sound();
            play_sound_effect(required_chase_sound, -1); // Loop infinitely
            ghost_chase_sound_playing_ = true;
            current_ghost_chase_sound_ = required_chase_sound;
        }
    }
    break;

    case GameMode::POWER_MODE:
        // Pac-Man is chasing scared ghosts - play ghostblue.wav, stop chase music
        stop_current_chase_sound();

        if (!ghost_blue_sound_playing_)
        {
            play_sound_effect(GHOST_BLUE_SOUND_NAME, -1); // Loop infinitely
            ghost_blue_sound_playing_ = true;
        }
        break;

    case GameMode::GAME_OVER:
    case GameMode::VICTORY:
        // Game ended - stop all background sounds
        stop_all_background_sounds();
        break;
    }

    // Handle sound state cleanup based on mode changes
    if (game_mode != GameMode::POWER_MODE && ghost_blue_sound_playing_)
    {
        stop_sound_effect(GHOST_BLUE_SOUND_NAME);
        ghost_blue_sound_playing_ = false;
    }

    if (game_mode != GameMode::STARTING && start_sound_playing_)
    {
        stop_sound_effect(START_SOUND_NAME);
        start_sound_playing_ = false;
    }
}

/**
 * @brief Play dot collection sound (alternates between dot1 and dot2)
 */
void SoundManager::play_dot_collection_sound()
{
    if (use_dot1_sound_)
    {
        play_sound_effect(DOT1_SOUND_NAME);
    }
    else
    {
        play_sound_effect(DOT2_SOUND_NAME);
    }

    // Toggle for next time
    use_dot1_sound_ = !use_dot1_sound_;
}

/**
 * @brief Play ghost eat sound (when Pac-Man catches a scared ghost)
 */
void SoundManager::play_ghost_eat_sound()
{
    play_sound_effect(GHOST_EAT_SOUND_NAME);
}

/**
 * @brief Play ghost retreat sound (when ghost is caught and retreating)
 */
void SoundManager::play_ghost_retreat_sound()
{
    play_sound_effect(GHOST_RETREAT_SOUND_NAME);
}

/**
 * @brief Play cutscene sound (when level is completed)
 */
void SoundManager::play_cutscene_sound()
{
    play_sound_effect(CUTSCENE_SOUND_NAME);
}

/**
 * @brief Stop all background sounds (chase, power mode, start sounds)
 */
void SoundManager::stop_all_background_sounds()
{
    stop_current_chase_sound();

    if (ghost_blue_sound_playing_)
    {
        stop_sound_effect(GHOST_BLUE_SOUND_NAME);
        ghost_blue_sound_playing_ = false;
    }

    if (start_sound_playing_)
    {
        stop_sound_effect(START_SOUND_NAME);
        start_sound_playing_ = false;
    }
}

/**
 * @brief Stop all sounds completely (including sound effects)
 */
void SoundManager::stop_all_sounds()
{
    stop_all_background_sounds();
    // Note: Individual sound effects (dot collection, ghost eat, etc.)
    // are short duration and don't need explicit stopping
}

/**
 * @brief Get the appropriate ghost chase sound name based on pellet percentage
 * @param pellet_percentage Percentage of pellets remaining (0-100)
 * @return Sound name constant for the appropriate chase sound
 */
const char *SoundManager::get_chase_sound_for_percentage(double pellet_percentage) const
{
    if (pellet_percentage > 75.0)
        return GHOST_CHASE_SOUND_NAME; // ghost1.wav
    else if (pellet_percentage > 50.0)
        return GHOST_CHASE_SOUND2_NAME; // ghost2.wav
    else if (pellet_percentage > 25.0)
        return GHOST_CHASE_SOUND3_NAME; // ghost3.wav
    else if (pellet_percentage > 10.0)
        return GHOST_CHASE_SOUND4_NAME; // ghost4.wav
    else
        return GHOST_CHASE_SOUND5_NAME; // ghost5.wav
}

/**
 * @brief Stop the currently playing ghost chase sound
 */
void SoundManager::stop_current_chase_sound()
{
    if (ghost_chase_sound_playing_ && current_ghost_chase_sound_ != nullptr)
    {
        stop_sound_effect(current_ghost_chase_sound_);
        ghost_chase_sound_playing_ = false;
        current_ghost_chase_sound_ = nullptr;
    }
}

/**
 * @brief Set the base path for sound files
 * @param base_path The base directory path for sounds (e.g., "Resources/Sounds/Normal/")
 */
void SoundManager::set_sound_base_path(const std::string &base_path)
{
    sound_base_path_ = base_path;
    // Ensure the path ends with a slash
    if (!sound_base_path_.empty() && sound_base_path_.back() != '/')
    {
        sound_base_path_ += '/';
    }
}

/**
 * @brief Unload all currently loaded sounds to allow reloading with different files
 */
void SoundManager::unload_all_sounds()
{
    // Stop all sounds first
    stop_all_sounds();

    // Free/unload all sound effects by getting the sound_effect pointer first
    if (has_sound_effect(GHOST_CHASE_SOUND_NAME))
        free_sound_effect(sound_effect_named(GHOST_CHASE_SOUND_NAME));
    if (has_sound_effect(GHOST_CHASE_SOUND2_NAME))
        free_sound_effect(sound_effect_named(GHOST_CHASE_SOUND2_NAME));
    if (has_sound_effect(GHOST_CHASE_SOUND3_NAME))
        free_sound_effect(sound_effect_named(GHOST_CHASE_SOUND3_NAME));
    if (has_sound_effect(GHOST_CHASE_SOUND4_NAME))
        free_sound_effect(sound_effect_named(GHOST_CHASE_SOUND4_NAME));
    if (has_sound_effect(GHOST_CHASE_SOUND5_NAME))
        free_sound_effect(sound_effect_named(GHOST_CHASE_SOUND5_NAME));
    if (has_sound_effect(DOT1_SOUND_NAME))
        free_sound_effect(sound_effect_named(DOT1_SOUND_NAME));
    if (has_sound_effect(DOT2_SOUND_NAME))
        free_sound_effect(sound_effect_named(DOT2_SOUND_NAME));
    if (has_sound_effect(GHOST_BLUE_SOUND_NAME))
        free_sound_effect(sound_effect_named(GHOST_BLUE_SOUND_NAME));
    if (has_sound_effect(GHOST_EAT_SOUND_NAME))
        free_sound_effect(sound_effect_named(GHOST_EAT_SOUND_NAME));
    if (has_sound_effect(GHOST_RETREAT_SOUND_NAME))
        free_sound_effect(sound_effect_named(GHOST_RETREAT_SOUND_NAME));
    if (has_sound_effect(START_SOUND_NAME))
        free_sound_effect(sound_effect_named(START_SOUND_NAME));
    if (has_sound_effect(DIE_SOUND_NAME))
        free_sound_effect(sound_effect_named(DIE_SOUND_NAME));
    if (has_sound_effect(CUTSCENE_SOUND_NAME))
        free_sound_effect(sound_effect_named(CUTSCENE_SOUND_NAME));
}
