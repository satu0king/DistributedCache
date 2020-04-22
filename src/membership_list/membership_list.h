#pragma once
#include <iostream>
#include <map>
#include <string>

#include "buffer.h"
#include "gossip_artifact.h"
#include "requests.h"

#define T_FAIL 8
#define T_REMOVE 16
#define RING_SIZE 1021

struct MemberSerialized {
    char ip[20];
    int port;
    long heartbeat;
    int startRange;
};

struct Member {
    Address address;
    long heartbeat;
    long timestamp;
    int startRange;
    Member(std::string ip, int port, int startRange = -1, long heartbeat = 0)
        : Member(Address(ip, port), startRange, heartbeat) {}
    Member(Address address, int startRange = -1, long heartbeat = 0,
           long timestamp = 0)
        : address(address),
          heartbeat(heartbeat),
          timestamp(timestamp),
          startRange(startRange) {}
    Member(MemberSerialized data)
        : Member(Address(data.ip, data.port), data.startRange, data.heartbeat) {
    }
    void update(const Member& other, long timestamp);

    Buffer serialize() const;
};

class MembershipList : public GossipArtifact {
    std::map<Address, int> memberMap;
    std::vector<Member> memberList;
    
    long timestamp;

   public:
    std::map<int, Address> consistentRing;
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

    Address getNearestNode(int key);
};
