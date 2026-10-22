/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientCore.hpp
*/

#ifndef CLIENT_CORE_HPP
    #define CLIENT_CORE_HPP

    #include <array>
    #include <cstdint>
    #include <map>
    #include <optional>
    #include <set>
    #include <string>
    #include <unordered_set>
    #include <vector>

    #include "network.hpp"
    #include "Display.hpp"
    #include "World.hpp"

namespace r_type::client {

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr std::size_t MAX_IP_LENGTH = 15;
constexpr std::size_t MAX_TRANSFER_ENTITIES = 4096;
constexpr float CONNECT_LIMIT = 3.f;
constexpr float ROOM_REFRESH_DELAY = 2.f;
constexpr float INPUT_RESEND_DELAY = 0.1f;
constexpr int RELEASE_RESENDS = 3;
constexpr float MESSAGE_DURATION = 4.f;

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

constexpr std::array<Binding, 5> BINDINGS = {{
    {Key::Up, Action::Up},
    {Key::Down, Action::Down},
    {Key::Left, Action::Left},
    {Key::Right, Action::Right},
    {Key::Space, Action::Fire},
}};

namespace palette {
constexpr Rgba BACKGROUND{8, 8, 20, 255};
constexpr Rgba FOREGROUND{235, 235, 245, 255};
constexpr Rgba DIM{140, 140, 160, 255};
constexpr Rgba ACCENT{102, 191, 255, 255};
constexpr Rgba PANEL{24, 24, 44, 255};
constexpr Rgba PANEL_HOVER{40, 40, 72, 255};
constexpr Rgba ALERT{230, 80, 80, 255};
constexpr Rgba ENEMY{190, 33, 55, 255};
constexpr Rgba PROJECTILE{253, 249, 0, 255};
constexpr Rgba OTHER{130, 130, 130, 255};
constexpr Rgba PLAYERS[4] = {{102, 191, 255, 255}, {255, 109, 194, 255}, {0, 200, 80, 255}, {255, 203, 0, 255}};
}

struct RoomInfo {
    std::uint32_t id = 0;
    std::uint8_t freeSlots = 0;
};

class ClientCore {
    enum class Scene {
        Menu,
        Connecting,
        Lobby,
        Joining,
        Game,
        Leaving
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
    std::set<Action> _held;
    std::map<Action, int> _releases;
    float _inputTimer = 0.f;
    std::unordered_set<std::uint32_t> _destroyed;
    std::optional<Transfer> _transfer;
    std::vector<Star> _stars;

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

    void MenuFrame();
    void LobbyFrame(float dt);
    void GameFrame(float dt);
    void WaitingFrame(const std::string& text);

    void ScrollStars(float dt);
    void DrawStars();
    void DrawEntities();
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
