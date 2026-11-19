#include "channel.hpp"

namespace r_type {

void Channel::Track(std::uint32_t messageId, std::vector<Datagram> datagrams, TimePoint now) {
    _pending.insert_or_assign(messageId, Pending{std::move(datagrams), now, 0});
}

void Channel::Acknowledge(std::uint32_t messageId) {
    _pending.erase(messageId);
}

const Datagram *Channel::FindFragment(std::uint32_t messageId, std::uint32_t fragmentIndex) const {
    const auto it = _pending.find(messageId);
    if (it == _pending.end() || fragmentIndex == 0 || fragmentIndex > it->second.datagrams.size())
        return nullptr;
    return &it->second.datagrams[fragmentIndex - 1];
}

bool Channel::Repeat(std::uint32_t messageId) {
    if (!_seen.contains(messageId))
        return false;
    QueueAck(messageId);
    return true;
}

std::optional<ArgsList> Channel::Receive(const DecodeResult& fragment, bool reliable, TimePoint now) {
    std::optional<ArgsList> message;
    if (fragment.fragmentCount == FRAGMENT_COUNT)
        message = fragment.args;
    else
        message = Assemble(fragment, reliable, now);
    if (message && reliable) {
        Remember(fragment.messageId);
        QueueAck(fragment.messageId);
    }
    return message;
}

std::optional<ArgsList> Channel::Assemble(const DecodeResult& fragment, bool reliable, TimePoint now) {
    auto it = _partials.find(fragment.messageId);
    if (it == _partials.end()) {
        if (_partials.size() >= MAX_PARTIAL_MESSAGES)
            return std::nullopt;
        it = _partials.emplace(fragment.messageId,
            Partial{fragment.command, fragment.fragmentCount, reliable, {}, now, 0}).first;
    } else if (it->second.command != fragment.command || it->second.count != fragment.fragmentCount) {
        _partials.erase(it);
        return std::nullopt;
    }

    Partial& partial = it->second;
    if (partial.parts.try_emplace(fragment.fragmentIndex, fragment.args).second) {
        partial.touchedAt = now;
        partial.requests = 0;
    }
    if (partial.parts.size() < partial.count)
        return std::nullopt;

    ArgsList message;
    for (auto& [index, records] : partial.parts)
        message.insert(message.end(), std::make_move_iterator(records.begin()), std::make_move_iterator(records.end()));
    _partials.erase(it);
    return message;
}

void Channel::Remember(std::uint32_t messageId) {
    if (!_seen.insert(messageId).second)
        return;
    _seenOrder.push_back(messageId);
    if (_seenOrder.size() > SEEN_WINDOW) {
        _seen.erase(_seenOrder.front());
        _seenOrder.pop_front();
    }
}

void Channel::QueueAck(std::uint32_t messageId) {
    if (std::find(_acks.begin(), _acks.end(), messageId) == _acks.end())
        _acks.push_back(messageId);
}

ArgsList Channel::TakeAcks() {
    ArgsList acks;
    acks.reserve(_acks.size());
    for (const std::uint32_t messageId : _acks)
        acks.push_back(Args{messageId});
    _acks.clear();
    return acks;
}

Channel::Output Channel::Update(TimePoint now) {
    Output output;

    for (auto it = _pending.begin(); it != _pending.end();) {
        Pending& pending = it->second;
        if (now - pending.sentAt < RESEND_DELAY) {
            ++it;
        } else if (pending.resends >= MAX_RESENDS) {
            it = _pending.erase(it);
        } else {
            output.resend.insert(output.resend.end(), pending.datagrams.begin(), pending.datagrams.end());
            pending.sentAt = now;
            pending.resends++;
            ++it;
        }
    }

    for (auto it = _partials.begin(); it != _partials.end();) {
        Partial& partial = it->second;
        if (now - partial.touchedAt < FRAGMENT_REQUEST_DELAY) {
            ++it;
        } else if (!partial.reliable || partial.requests >= MAX_RESENDS) {
            it = _partials.erase(it);
        } else {
            for (std::uint32_t index = 1; index <= partial.count; index++) {
                if (!partial.parts.contains(index))
                    output.requests.push_back(Args{it->first, index});
            }
            partial.touchedAt = now;
            partial.requests++;
            ++it;
        }
    }

    output.acks = TakeAcks();
    return output;
}

bool Channel::Idle() const {
    return _pending.empty() && _partials.empty() && _seen.empty() && _acks.empty();
}

}
