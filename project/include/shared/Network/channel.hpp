#ifndef CHANNEL_HPP
    #define CHANNEL_HPP

    #include <algorithm>
    #include <chrono>
    #include <cstddef>
    #include <cstdint>
    #include <deque>
    #include <iterator>
    #include <map>
    #include <optional>
    #include <set>
    #include <utility>
    #include <vector>

    #include "packet.hpp"
    #include "protocol.hpp"

namespace r_type {

using TimePoint = std::chrono::steady_clock::time_point;

constexpr std::chrono::milliseconds RESEND_DELAY{200};
constexpr std::chrono::milliseconds FRAGMENT_REQUEST_DELAY{50};
constexpr unsigned MAX_RESENDS = 5;
constexpr std::size_t MAX_PARTIAL_MESSAGES = 8;
constexpr std::size_t SEEN_WINDOW = 1024;

class Channel {
public:
    struct Output {
        std::vector<Datagram> resend;
        ArgsList acks;
        ArgsList requests;
    };

private:
    struct Pending {
        std::vector<Datagram> datagrams;
        TimePoint sentAt;
        unsigned resends = 0;
    };

    struct Partial {
        Command command;
        std::uint32_t count;
        bool reliable;
        std::map<std::uint32_t, ArgsList> parts;
        TimePoint touchedAt;
        unsigned requests = 0;
    };

    std::map<std::uint32_t, Pending> _pending;
    std::map<std::uint32_t, Partial> _partials;
    std::deque<std::uint32_t> _seenOrder;
    std::set<std::uint32_t> _seen;
    std::vector<std::uint32_t> _acks;

    std::optional<ArgsList> Assemble(const DecodeResult& fragment, bool reliable, TimePoint now);
    void Remember(std::uint32_t messageId);
    void QueueAck(std::uint32_t messageId);

public:
    void Track(std::uint32_t messageId, std::vector<Datagram> datagrams, TimePoint now);
    void Acknowledge(std::uint32_t messageId);
    const Datagram *FindFragment(std::uint32_t messageId, std::uint32_t fragmentIndex) const;

    bool Repeat(std::uint32_t messageId);
    std::optional<ArgsList> Receive(const DecodeResult& fragment, bool reliable, TimePoint now);
    ArgsList TakeAcks();

    Output Update(TimePoint now);
    bool Idle() const;
};

}

#endif
