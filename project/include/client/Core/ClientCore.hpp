/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientCore.hpp
*/

#ifndef CLIENT_CORE_HPP
    #define CLIENT_CORE_HPP

    #include <algorithm>
    #include <array>
    #include <cctype>
    #include <cmath>
    #include <cstdint>
    #include <fstream>
    #include <iostream>
    #include <map>
    #include <optional>
    #include <set>
    #include <string>
    #include <unordered_set>
    #include <vector>

    #include "network.hpp"
    #include "Display.hpp"
    #include "World.hpp"
    #include "Level.hpp"
    #include "sig.hpp"
    #include "Encryption/SecurityConfig.hpp"

namespace r_type::client {

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr std::size_t MAX_IP_LENGTH = 15;
constexpr const char *LOCAL_IP = "127.0.0.1";
constexpr const char *LOCALHOST = "localhost";
constexpr std::size_t MAX_TRANSFER_ENTITIES = 4096;
constexpr float CONNECT_LIMIT = 3.f;
constexpr float ROOM_REFRESH_DELAY = 2.f;
constexpr float INPUT_RESEND_DELAY = 0.1f;
constexpr int RELEASE_RESENDS = 3;
constexpr float MESSAGE_DURATION = 4.f;
constexpr float GAME_OVER_DELAY = 1.f;
constexpr float MENU_SLIDE = 3.f;
constexpr float MENU_FADE = 1.f;
constexpr float BACKGROUND_SCROLL = 15.f;
constexpr std::size_t LEVEL_CARDS = 4;
constexpr const char *SETTINGS_FILE = "r-type_settings.json";
constexpr const char *BACKGROUNDS_DIR = "backgrounds/";
constexpr const char *WINDOW_TITLE = "R-Type World Tour";
constexpr const char *TITLE_FONT = "title";
constexpr const char *TEXT_FONT = "text";
constexpr const char *TITLE_FONT_PATH = "assets/fonts/RussoOne-Regular.ttf";
constexpr const char *TEXT_FONT_PATH = "assets/fonts/Exo2-Regular.ttf";

enum class Action : std::uint8_t {
    Up = 0x00,
    Down = 0x01,
    Left = 0x02,
    Right = 0x03,
    Fire = 0x04
};

struct Binding {
    Key key;
    Action action;
};

constexpr std::array<Binding, 9> BINDINGS = {{
    {Key::Up, Action::Up},
    {Key::Down, Action::Down},
    {Key::Left, Action::Left},
    {Key::Right, Action::Right},
    {Key::W, Action::Up},
    {Key::S, Action::Down},
    {Key::A, Action::Left},
    {Key::D, Action::Right},
    {Key::Space, Action::Fire},
}};

struct Settings {
    bool autoFire = false;
    bool reduceMotion = false;
    bool darkBackgrounds = false;
};

struct OptionInfo {
    const char *name;
    const char *help;
    const char *key;
    bool Settings::*value;
};

constexpr std::array<OptionInfo, 3> OPTIONS = {{
    {"Auto-fire", "Your ship fires on its own: no need to hold SPACE", "auto_fire", &Settings::autoFire},
    {"Reduce motion", "Stops the scrolling background, the stars and the menu slideshow", "reduce_motion",
        &Settings::reduceMotion},
    {"Darker backgrounds", "Darkens the photos so the ships and the text stand out", "dark_backgrounds",
        &Settings::darkBackgrounds},
}};

namespace palette {
constexpr Rgba BACKGROUND{8, 8, 20, 255};
constexpr Rgba FOREGROUND{235, 235, 245, 255};
constexpr Rgba DIM{205, 205, 225, 255};
constexpr Rgba KEY{255, 203, 0, 255};
constexpr Rgba ACCENT{102, 191, 255, 255};
constexpr Rgba PANEL{24, 24, 44, 255};
constexpr Rgba PANEL_HOVER{40, 40, 72, 255};
constexpr Rgba ALERT{230, 80, 80, 255};
constexpr Rgba ENEMY{190, 33, 55, 255};
constexpr Rgba PROJECTILE{253, 249, 0, 255};
constexpr Rgba OTHER{130, 130, 130, 255};
constexpr Rgba BACKGROUND_SHADE{0, 0, 0, 110};
constexpr Rgba DARK_SHADE{0, 0, 0, 200};
constexpr Rgba BOSS_HEALTH{230, 60, 60, 255};
constexpr Rgba GAME_OVER_SHADE{0, 0, 0, 150};
constexpr Rgba MENU_SHADE{8, 8, 20, 140};
constexpr Rgba INPUT{8, 8, 20, 220};
constexpr Rgba BUTTON_HOVER{255, 255, 255, 60};
constexpr Rgba PLAYERS[4] = {{102, 191, 255, 255}, {255, 109, 194, 255}, {0, 200, 80, 255}, {255, 203, 0, 255}};
}

struct Character {
    const char *name;
    const char *country;
    const char *image;
    const char *background;
};

constexpr std::array<Character, 4> CHARACTERS = {{
    {"Ana", "Argentine", "assets/ships/ana.png", "assets/backgrounds/argentine.png"},
    {"Soha", "Tunisie", "assets/ships/soha.png", "assets/backgrounds/tunisie.png"},
    {"Hilal", "Turquie", "assets/ships/hilal.png", "assets/backgrounds/turquie.png"},
    {"Eric", "Corse", "assets/ships/eric.png", "assets/backgrounds/corse.png"},
}};

struct LevelInfo {
    std::string name;
    std::string country;
    std::string background;
    std::string boss;
};

constexpr const char *ASSETS_DIR = "assets/";
constexpr const char *IMAGE_EXTENSION = ".png";

constexpr std::array<const char *, 14> ENEMY_SPRITES = {
    "enemies/ballon", "enemies/supporter", "enemies/defenseur", "enemies/scorpion", "enemies/harissa",
    "enemies/simit", "enemies/cay", "enemies/chat", "enemies/grognard", "enemies/sanglier",
    "bosses/eldiez", "bosses/couscoussier", "bosses/doner", "bosses/napoleon",
};

constexpr int FIRST_BOSS = 10;
constexpr float BOSS_SIZE = 160.f;
constexpr std::array<const char *, 4> BOSS_NAMES = {"El Diez", "Le Couscoussier", "Le Döner Géant", "Napoléon"};

constexpr std::array<const char *, 2> MISSILE_SPRITES = {"shots/missile", "shots/enemy_shot"};

struct BossBar {
    std::uint32_t id = 0;
    std::string name;
    int percent = 100;
};

struct RoomInfo {
    std::uint32_t id = 0;
    std::uint8_t freeSlots = 0;
};

class ClientCore {
    enum class Scene {
        Menu,
        Connecting,
        Lobby,
        Character,
        Level,
        Joining,
        Game,
        Leaving,
        Options
    };

    struct Transfer {
        std::uint32_t expected = 0;
        std::map<std::uint32_t, EntityData> entities;
    };

    struct Star {
        float x;
        float y;
        float speed;
        float size;
    };

    Display _display;
    UdpSocket _net;
    World _world;
    Scene _scene = Scene::Menu;
    float _sceneTime = 0.f;
    bool _quit = false;
    std::string _ip;
    std::uint16_t _roomPort = 0;
    std::string _message;
    float _messageTime = 0.f;
    std::vector<RoomInfo> _rooms;
    std::size_t _selected = 0;
    float _refreshTimer = 0.f;
    std::uint8_t _character = 0;
    std::optional<std::uint32_t> _joinTarget;
    std::uint8_t _levelChoice = 0;
    std::optional<std::uint8_t> _requestedLevel;
    std::optional<std::uint8_t> _level;
    std::optional<float> _gameOver;
    std::set<Action> _held;
    std::map<Action, int> _releases;
    float _inputTimer = 0.f;
    std::unordered_set<std::uint32_t> _destroyed;
    std::optional<Transfer> _transfer;
    std::optional<BossBar> _boss;
    std::vector<Star> _stars;
    std::vector<LevelInfo> _levels;
    Settings _settings;
    Scene _optionsReturn = Scene::Menu;
    std::size_t _option = 0;

    void SetScene(Scene scene);
    void ShowMessage(const std::string& message);
    void CheckTimeouts();

    bool Send(Command command, const ArgsList& args);
    void ConnectServer();
    void ReturnToLobby();
    void GoOffline(const std::string& reason);
    void SayGoodbye();
    void ResetRoom();
    void SetHeld(const std::set<Action>& held);
    void SendInput(Action action, InputState state);
    void ResendInputs(float dt);

    void RegisterHandlers();
    void OnConnectAccept();
    void OnDisconnect();
    void OnError(const ArgsList& args);
    void OnRoomList(const ArgsList& args);
    void OnRoomJoined(const ArgsList& args);
    void OnPlayerCreation(const ArgsList& args);
    void OnInitialStateBegin(const ArgsList& args);
    void OnInitialStateEnd(const ArgsList& args);
    void OnEntitySpawn(const ArgsList& args);
    void OnEntityUpdate(const ArgsList& args);
    void OnEntityDestroy(const ArgsList& args);
    void OnBossHealth(const ArgsList& args);
    void OnLevelStart(const ArgsList& args);

    void LoadLevels();
    void LoadSettings();
    void SaveSettings();
    void OpenOptions();
    void OptionsFrame(float dt);
    Rgba Shade(Rgba normal) const;

    void MenuFrame();
    void LobbyFrame(float dt);
    void CharacterFrame(float dt);
    void LevelFrame(float dt);
    void GameFrame(float dt);
    void WaitingFrame(const std::string& text);

    void DrawMenuBackground();
    void DrawTitle();
    void DrawTourBox(float x, float y, float width, float height);
    bool TourButton(const std::string& label, float x, float y, float width, float height);
    void DrawHint(const std::string& text, int y, int size);
    void ScrollStars(float dt);
    void DrawStars();
    void DrawBackground();
    void KeepAlive(float dt);
    void DrawBossBar();
    void GameOverFrame(float dt);
    void LeaveRoom();
    void DrawEntities();
    void DrawFace(std::size_t character, float x, float y, float size);
    void LoadSprite(const char *name);
    void DrawMessage(float dt);
    void DrawCentered(const std::string& text, int y, int size, Rgba color);
    bool Button(const std::string& label, float x, float y, float width, float height);

public:
    explicit ClientCore(std::string ip);
    ~ClientCore() = default;

    ClientCore(const ClientCore&) = delete;
    ClientCore& operator=(const ClientCore&) = delete;

    void Run();
};

}

#endif
