# Pac-Man Game - UML Class Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                    Game                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│ - running_: bool                                                            │
│ - game_initialized_: bool                                                   │
│ - paused_: bool                                                             │
│ - escape_key_cooldown_: double                                              │
│ - last_time_: double                                                        │
│ - current_game_mode_: GameMode                                              │
│ - previous_game_mode_: GameMode                                             │
│ - current_level_: int                                                       │
│ - maze_: unique_ptr<Maze>                                                   │
│ - sprite_sheet_: unique_ptr<SpriteSheet>                                    │
│ - pacman_: unique_ptr<Pacman>                                               │
│ - ghost1_: unique_ptr<Ghost>                                                │
│ - ghost2_: unique_ptr<Ghost>                                                │
│ - fruit_: unique_ptr<Fruit>                                                 │
│ - game_state_: unique_ptr<GameState>                                        │
│ - sound_manager_: unique_ptr<SoundManager>                                  │
│ - menu_: unique_ptr<Menu>                                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│ + Game()                                                                    │
│ + initialize(): bool                                                        │
│ + run(): void                                                               │
│ - update(delta_time: double): void                                          │
│ - render(): void                                                            │
│ - handle_events(): void                                                     │
│ - initialize_game_entities(): void                                          │
│ - update_game_mode(delta_time: double): void                                │
│ - determine_current_game_mode(): GameMode                                   │
│ - handle_ghost_collisions(): void                                           │
│ - check_win_condition(): bool                                               │
│ - calculate_pellet_percentage(): double                                     │
│ - advance_to_next_level(): void                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                    │ owns                │ owns                │ owns
                    ▼                     ▼                     ▼
        ┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
        │      Maze        │  │   SpriteSheet    │  │   SoundManager   │
        ├──────────────────┤  ├──────────────────┤  ├──────────────────┤
        │ - layout_: int[*]│  │ - sheet_: bitmap │  │ - sound_base_    │
        │ - level_: int    │  │ - frame_w: int   │  │   path_: string  │
        ├──────────────────┤  │ - frame_h: int   │  │ - start_sound_   │
        │ + is_empty():bool│  ├──────────────────┤  │   playing_: bool │
        │ + can_move_to()  │  │ + draw_sprite_   │  │ - chase_sound_   │
        │ + initialize_    │  │   at_pixel()     │  │   playing_: bool │
        │   tokens()       │  │ + frame_width()  │  ├──────────────────┤
        │ + get_cell_      │  │ + frame_height() │  │ + initialize()   │
        │   center_x/y()   │  └──────────────────┘  │ + update_back    │
        └──────────────────┘                        │   ground_audio() │
                                                    │ + play_dot_      │
                                                    │   collection()   │
                                                    │ + stop_all_      │
                                                    │   sounds()       │
                                                    └──────────────────┘
                    │ owns
                    ▼
        ┌──────────────────────────────────────────┐
        │              GameState                   │
        ├──────────────────────────────────────────┤
        │ - score_: int                            │
        │ - tokens_: vector<Token>                 │
        │ - power_pellets_: vector<PowerPellet>    │
        │ - token_just_collected_: bool            │
        ├──────────────────────────────────────────┤
        │ + get_score(): int                       │
        │ + add_score(points: int): void           │
        │ + add_token(row: int, col: int): void    │
        │ + check_token_collection(): void         │
        │ + all_tokens_collected(): bool           │
        │ + draw_tokens(): void                    │
        │ + draw_score(): void                     │
        └──────────────────────────────────────────┘
                    │ contains
                    ▼
            ┌───────────────┐        ┌───────────────────┐
            │     Token     │        │   PowerPellet     │
            ├───────────────┤        ├───────────────────┤
            │ - row_: int   │        │ - row_: int       │
            │ - col_: int   │        │ - col_: int       │
            │ - collected_  │        │ - collected_: bool│
            ├───────────────┤        ├───────────────────┤
            │ + collect()   │        │ + collect()       │
            │ + draw()      │        │ + draw()          │
            └───────────────┘        └───────────────────┘

        ┌──────────────────────────────────────────┐
        │              Menu                        │
        ├──────────────────────────────────────────┤
        │ - current_state_: MenuState              │
        │ - selected_option_: int                  │
        │ - should_start_game_: bool               │
        │ - should_quit_: bool                     │
        │ - difficulty_level_: DifficultyLevel     │
        │ - endless_mode_: bool                    │
        │ - selected_level_: int                   │
        │ - sprite_sheet_: SpriteSheet*            │
        │ - sound_manager_: SoundManager*          │
        ├──────────────────────────────────────────┤
        │ + Menu()                                 │
        │ + handle_input(): void                   │
        │ + render(): void                         │
        │ + should_start_game(): bool              │
        │ + should_quit_game(): bool               │
        │ + get_selected_level(): int              │
        │ + get_difficulty_speed_multiplier(): dbl │
        │ + is_endless_mode(): bool                │
        │ - render_main_menu(): void               │
        │ - render_settings_screen(): void         │
        │ - handle_main_menu_input(): void         │
        └──────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────────────┐
│                              Entity (Abstract)                               │
├──────────────────────────────────────────────────────────────────────────────┤
│ # x_: double                                                                 │
│ # y_: double                                                                 │
│ # dir_: direction_t                                                          │
│ # desired_dir_: direction_t                                                  │
│ # palette_: string                                                           │
│ # speed_multiplier_: double                                                  │
├──────────────────────────────────────────────────────────────────────────────┤
│ + Entity(x: double, y: double, palette: string)                             │
│ + get_x(): double                                                            │
│ + get_y(): double                                                            │
│ + get_direction(): direction_t                                               │
│ + set_position(x: double, y: double): void                                   │
│ + set_speed_multiplier(mult: double): void                                   │
│ + update(maze: Maze, delta_time: double): void                               │
│ + draw(): void  «abstract»                                                   │
│ + get_current_speed(): double  «virtual»                                     │
│ - move_in_direction(maze: Maze, delta_time: double): void                    │
│ - attempt_direction_change(maze: Maze, ...): void                            │
└──────────────────────────────────────────────────────────────────────────────┘
                        △                          △
                        │                          │
            ┌───────────┴──────────┐      ┌────────┴────────┐
            │                      │      │                 │
┌───────────────────────┐  ┌──────────────────────────────────────┐
│       Pacman          │  │            Ghost                     │
├───────────────────────┤  ├──────────────────────────────────────┤
│ - sheet_: SpriteSheet*│  │ - sheet_: SpriteSheet*               │
│ - anim_state_: enum   │  │ - anim_state_: AnimationState        │
│ - anim_timer_: double │  │ - anim_timer_: double                │
│ - is_in_power_mode_   │  │ - current_state_: GhostState         │
│   : bool              │  │ - scared_timer_: double              │
├───────────────────────┤  │ - ai_type_: GhostAIType              │
│ + capture_input()     │  │ - target_x_, target_y_: double       │
│ + update(maze, gs,    │  │ - show_score_popup_: bool            │
│   delta): void        │  │ - popup_timer_: double               │
│ + draw(): void        │  ├──────────────────────────────────────┤
│ + set_power_mode()    │  │ + update(maze, px, py, dir, dt): void│
│ + play_dying_         │  │ + draw(): void                       │
│   animation()         │  │ + set_scared_mode(): void            │
│ - update_animation()  │  │ + set_caught_mode(): void            │
│ - handle_tunnel_      │  │ + is_scared(): bool                  │
│   wrapping()          │  │ + trigger_score_popup(x, y): void    │
└───────────────────────┘  │ - choose_direction_towards_target()  │
                           │ - choose_direction_random_patrol()   │
                           │ - choose_direction_ambush()          │
                           │ - choose_direction_away_from_target()│
                           │ - update_animation(): void           │
                           └──────────────────────────────────────┘

            ┌──────────────────────────────────────┐
            │              Fruit                   │
            ├──────────────────────────────────────┤
            │ - sheet_: SpriteSheet*               │
            │ - x_, y_: double                     │
            │ - fruit_type_: int                   │
            │ - is_active_: bool                   │
            │ - spawn_timer_: double               │
            │ - visible_timer_: double             │
            │ - show_score_popup_: bool            │
            │ - popup_timer_: double               │
            ├──────────────────────────────────────┤
            │ + update(delta: double, maze: Maze)  │
            │ + draw(): void                       │
            │ + check_collision(px, py): bool      │
            │ + is_active(): bool                  │
            │ - spawn_fruit(maze: Maze): void      │
            └──────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                         Enumerations                            │
├─────────────────────────────────────────────────────────────────┤
│ «enum» direction_t                                              │
│   DIR_NONE, DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN              │
│                                                                 │
│ «enum» GameMode                                                 │
│   STARTING, NORMAL, POWER_MODE, GAME_OVER, VICTORY             │
│                                                                 │
│ «enum» MenuState                                                │
│   MAIN_MENU, LEVEL_SELECT, DIFFICULTY, HIGH_SCORES,            │
│   SETTINGS, NAME_ENTRY, IN_GAME                                │
│                                                                 │
│ «enum» GhostState                                               │
│   CHASING, SCARED, CAUGHT, COOLDOWN                            │
│                                                                 │
│ «enum» GhostAIType                                              │
│   RANDOM_PATROL, AMBUSHER                                       │
│                                                                 │
│ «enum» DifficultyLevel                                          │
│   EASY, MEDIUM, HARD, CRAZY                                    │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                      Relationship Legend                        │
├─────────────────────────────────────────────────────────────────┤
│ ───▶       Association (uses/depends on)                       │
│ ◆───▶      Composition (owns/contains, lifecycle dependent)    │
│ △          Inheritance (is-a relationship)                      │
│ - - - ▶    Dependency (temporary usage)                         │
└─────────────────────────────────────────────────────────────────┘

Key Relationships:
=================
1. Game ◆─→ Maze, SpriteSheet, SoundManager, Menu, GameState (Composition)
2. Game ◆─→ Pacman, Ghost (x2), Fruit (Composition)
3. Entity △─→ Pacman, Ghost (Inheritance)
4. GameState ◆─→ Token, PowerPellet (Composition)
5. Menu ──→ SpriteSheet, SoundManager (Association - uses pointers)
6. Pacman, Ghost, Fruit ──→ SpriteSheet (Association - uses pointer)
7. All entities ──→ Maze (Dependency - passed to update methods)
8. Pacman ──→ GameState (Dependency - passed to update method)

Design Patterns Used:
====================
1. **Composition**: Game owns all subsystems via unique_ptr
2. **Inheritance**: Entity base class with Pacman/Ghost derived classes
3. **Strategy Pattern**: Ghost AI types (RANDOM_PATROL, AMBUSHER)
4. **State Pattern**: GameMode, GhostState, MenuState enums
5. **Facade Pattern**: Game class coordinates all subsystems
6. **Singleton-like**: SoundManager, Menu (single instances)
```
