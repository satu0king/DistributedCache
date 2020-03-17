#pragma once
#include "requests.h"
#include "buffer.h"
#include <string>
#include <map>
#include <iostream>
#include "gossip_artifact.h"

#define T_FAIL 4
#define T_REMOVE 8

struct MemberSerialized {
    char ip[20];
    int port;
    long heartbeat;
};

struct Member {
    Address address;
    long heartbeat;
    long timestamp;
    Member(std::string ip, int port, long heartbeat = 0)
        : Member(Address(ip, port), heartbeat) {}
    Member(Address address, long heartbeat = 0, long timestamp = 0)
        : address(address), heartbeat(heartbeat), timestamp(timestamp) {}
    Member(MemberSerialized data)
        : Member(Address(data.ip, data.port), data.heartbeat) {}
    void update(const Member& other, long timestamp);

    Buffer serialize() const;
};

class MembershipList : public GossipArtifact {
    std::map<Address, int> memberMap;
    std::vector<Member> memberList;
    long timestamp;

   public:
    MembershipList();
    int getTimestamp();
    void updateTimestamp(long timestamp);
    Member& getMember(Address address);
    std::string getType() const;
    Buffer getPayload() const;

    void update(Buffer data);
    void loop(long timestamp);

    void removeDead();

    bool hasMember(Address address);
    void addMember(Member member);

    size_t size() const;

    Member& operator[](int i);

    void deleteMember(Address& address);

    void deleteMember(Member& member);
};
