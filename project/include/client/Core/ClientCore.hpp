/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientCore.hpp
*/

#ifndef CLIENT_CORE_HPP
    #define CLIENT_CORE_HPP

    #include <cstdint>
    #include <string>
    #include <vector>

    #include "ClientEvents.hpp"
    #include "ClientNetwork.hpp"
    #include "Display.hpp"
    #include "World.hpp"

namespace r_type::client {

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr std::size_t MAX_IP_LENGTH = 15;
constexpr float ROOM_REFRESH_DELAY = 2.f;
constexpr float WAIT_TIMEOUT = 6.f;
constexpr float MESSAGE_DURATION = 4.f;

namespace palette {
constexpr Rgba BACKGROUND{8, 8, 20, 255};
constexpr Rgba TEXT{235, 235, 245, 255};
constexpr Rgba DIM{140, 140, 160, 255};
constexpr Rgba ACCENT{102, 191, 255, 255};
constexpr Rgba PANEL{24, 24, 44, 255};
constexpr Rgba PANEL_HOVER{40, 40, 72, 255};
constexpr Rgba ALERT{230, 80, 80, 255};
constexpr Rgba ENEMY{190, 33, 55, 255};
constexpr Rgba PROJECTILE{253, 249, 0, 255};
constexpr Rgba UNKNOWN{130, 130, 130, 255};
constexpr Rgba PLAYERS[4] = {{102, 191, 255, 255}, {255, 109, 194, 255}, {0, 200, 80, 255}, {255, 203, 0, 255}};
}

class ClientCore {
    enum class Scene {
        Menu,
        Connecting,
        Lobby,
        Joining,
        Game,
        Leaving
    };

    struct Star {
        float x;
        float y;
        float speed;
        float size;
    };

    Display _display;
    ClientNetwork _network;
    World _world;
    Scene _scene = Scene::Menu;
    bool _quit = false;
    std::string _ip;
    std::string _message;
    float _messageTime = 0.f;
    std::vector<RoomInfo> _rooms;
    std::size_t _selected = 0;
    float _refreshTimer = 0.f;
    float _waitTimer = 0.f;
    std::uint8_t _held = 0;
    std::vector<Star> _stars;

    void HandleEvents();
    void HandleEvent(const NetEvent& event);
    void SetScene(Scene scene);
    void ShowMessage(const std::string& message);

    void MenuFrame();
    void LobbyFrame(float dt);
    void GameFrame();
    void WaitingFrame(const std::string& text, float dt);

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
