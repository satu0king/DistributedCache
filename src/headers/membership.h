#pragma once
#include <string.h>

#include <boost/date_time.hpp>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include "requests.h"

#define HEARTBEAT_PERIOD 100
#define TFAIL 4
#define TREMOVE 8
#define GOSSIP_K 2

struct Buffer {
    std::vector<char> buffer;
    char& operator[](int i) { return buffer[i]; }
    Buffer(int size = 0) : buffer(size){};
    Buffer(void* data, int size) : buffer(size) {
        memcpy(buffer.data(), data, size);
    };
    int size() const { return buffer.size(); }
    void addBuffer(Buffer& other) {
        buffer.insert(buffer.end(), other.buffer.begin(), other.buffer.end());
    }
    void addBuffer(Buffer&& other) {
        buffer.insert(buffer.end(), other.buffer.begin(), other.buffer.end());
    }
    void addBuffer(void* data, int size) {
        buffer.insert(buffer.end(), (char*)data, (char*)data + size);
    }
    char* data() { return buffer.data(); }
};

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
    void update(const Member& other, long timestamp) {
        assert(address == other.address);
        if (other.heartbeat <= heartbeat) return;
        heartbeat = other.heartbeat;
        this->timestamp = timestamp;
    }

    Buffer serialize() const {
        MemberSerialized data;
        strcpy(data.ip, address.ip.c_str());
        data.port = address.port;
        data.heartbeat = heartbeat;

        Buffer bufferdata(&data, sizeof(data));
        return bufferdata;
    }
};

int getConnection(Address addr) {
    int IP = inet_addr(addr.ip.c_str());  // INADDR_ANY;
    struct sockaddr_in server, client;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
    server.sin_port = htons(addr.port);

    if (connect(sd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        return -1;
    }

    return sd;
}

struct GossipPayload {
    int gossipArtifactCount;
    char* data[0];
};

struct PayloadHeader {
    char type[20];
    int size;  // (not including header size)
};

struct Payload {
    PayloadHeader header;
    char* data[0];

    int getSize() { return sizeof(struct Payload) + header.size; }
};

class GossipArtifact {
   public:
    virtual std::string getType() const = 0;
    virtual Buffer getPayload(long timestamp) const = 0;
    virtual void update(Buffer data, long timestamp) = 0;
    GossipArtifact() {}
};

struct MembershipList : GossipArtifact {
    std::map<Address, int> memberMap;
    std::vector<Member> memberList;

    Member& getMember(Address address) {
        assert(hasMember(address));
        return memberList[memberMap[address]];
    }
    std::string getType() const { return "membershiplist"; }
    Buffer getPayload(long timestamp) const {
        Buffer memberData;
        // char *ptr = (char *)&payload->data[0];
        for (auto& member : memberList) {
            if (member.timestamp + TFAIL > timestamp)
                memberData.addBuffer(member.serialize());
        }
        Payload payload;
        strcpy(payload.header.type, getType().c_str());
        payload.header.size = memberData.size();
        Buffer data(&payload, sizeof(payload));
        data.addBuffer(memberData);
        return data;
    }

    void update(Buffer data, long timestamp) {
        int size = data.size();
        int recordCount = size / sizeof(MemberSerialized);
        assert(size % sizeof(MemberSerialized) == 0);

        if (recordCount < memberList.size()) {
            // std::cout <<"Loss " << recordCount << std::endl;
        }

        MemberSerialized* records =
            reinterpret_cast<MemberSerialized*>(data.data());
        for (int i = 0; i < recordCount; ++i) {
            addMember(Member(records[i]), timestamp);
        }
    }

    bool hasMember(Address address) { return memberMap.count(address); }
    void addMember(Member member, long timestamp) {
        if (hasMember(member.address)) {
            getMember(member.address).update(member, timestamp);
        } else {
            std::cout << "New ";
            std::cout << member.address.toString() << std::endl;
            member.timestamp = timestamp;

            memberMap[member.address] = memberList.size();
            memberList.push_back(member);
        }
    }

    size_t size() const { return memberList.size(); }

    Member& operator[](int i) { return memberList[i]; }

    void deleteMember(Address& address) {
        assert(hasMember(address));

        std::cout << "Member Lost!" << std::endl;
        std::cout << address.toString() << std::endl;

        int i = memberMap[address];

        memberList[i] = memberList.back();
        memberMap[memberList.back().address] = i;

        memberList.pop_back();
        memberMap.erase(address);
    }

    void deleteMember(Member& member) { deleteMember(member.address); }
};

class MemberNode {
    Member myMember;
    MembershipList memberList;
    pthread_t tid;

    std::unordered_map<std::string, GossipArtifact*> gossipArtifacts;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

   public:
    MembershipList getList() {
        pthread_mutex_lock(&lock);
        MembershipList list = memberList;
        pthread_mutex_unlock(&lock);
        return list;
    }

    void addIntroducer(std::string ip, int port) {
        memberList.addMember(Member(ip, port), 0);
    }

    MemberNode(Address myAddress);

    void registerArtifact(GossipArtifact& artifact) {
        gossipArtifacts.emplace(artifact.getType(), &artifact);
    }

    void removeDead() {
        auto& list = memberList.memberList;
        std::vector<Member> to_delete;

        int timestamp = heartbeat();
        for (auto& member : list) {
            if (member.timestamp + TREMOVE < timestamp)
                to_delete.push_back(member);
        }

        for (auto& member : to_delete) memberList.deleteMember(member);
    }

    Buffer generateGossipPayload() {
        std::vector<Payload*> payloads;
        Buffer payloadData;
        for (auto& [key, artifact] : gossipArtifacts) {
            payloadData.addBuffer(artifact->getPayload(heartbeat()));
        }
        GossipPayload payloadHeader;
        payloadHeader.gossipArtifactCount = gossipArtifacts.size();

        Buffer data(&payloadHeader, sizeof(payloadHeader));
        data.addBuffer(payloadData);

        return data;
    }

    int heartbeat() { return myMember.heartbeat; }

    void loop() {
        myMember.heartbeat++;
        myMember.timestamp++;
        int timestamp = heartbeat();

        pthread_mutex_lock(&lock);
        removeDead();
        memberList.addMember(myMember, timestamp);

        // std::cout <<"XXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
        std::cout << myMember.address.toString() << " : " << memberList.size()
                  << std::endl;
        for (auto member : memberList.memberList) {
            // std::cout << member.address.toString() << " - " <<
            // member.timestamp << std::endl;
        }

        std::vector<int> sendList = selectKItems(
            memberList.size(), std::min(GOSSIP_K, (int)memberList.size()));
        auto data = generateGossipPayload();
        for (int i : sendList) sendMemberList(memberList[i], data);
        pthread_mutex_unlock(&lock);
    }

    int sendMemberList(Member& member, Buffer data) {
        if (member.address == myMember.address) return 0;
        int connection = getConnection(member.address);
        if (connection == -1) {
            std::cout << "connection drop" << std::endl;
            return -1;
        }
        RequestType type = RequestType::GOSSIP;
        loop_write(connection, &type, sizeof(type));
        loop_write(connection, data.data(), data.size());
        close(connection);
        return 0;
    }

    void receiveGossip(int connection) {
        GossipPayload header;
        int read_bytes = loop_read(connection, &header, sizeof(header));
        assert(read_bytes == sizeof(header));

        int gossipArtifactCount = header.gossipArtifactCount;

        pthread_mutex_lock(&lock);
        while (gossipArtifactCount--) {
            PayloadHeader header;
            read_bytes = loop_read(connection, &header, sizeof(header));
            assert(read_bytes == sizeof(header));
            std::string type = header.type;
            assert(gossipArtifacts.count(type));
            Buffer data(header.size);
            read_bytes = loop_read(connection, data.data(), header.size);
            assert(read_bytes == header.size);
            gossipArtifacts[type]->update(data, heartbeat());
        }
        pthread_mutex_unlock(&lock);
        close(connection);
    }

    std::vector<int> selectKItems(int n, int k) {
        assert(n >= k);
        int i;
        std::vector<int> reservoir(k);
        for (i = 0; i < k; i++) reservoir[i] = i;
        for (; i < n; i++) {
            int j = rand() % (i + 1);
            if (j < k) reservoir[j] = i;
        }
        return reservoir;
    }
};

void* memberloop(void* ptr) {
    usleep(500 * 1000);
    srand(time(NULL));
    MemberNode* node = (MemberNode*)ptr;
    while (1) {
        // std::cout << node->heartbeat() << std::endl;
        node->loop();
        usleep(HEARTBEAT_PERIOD * 1000);
    }

    return NULL;
}

MemberNode::MemberNode(Address myAddress) : myMember(myAddress) {
    memberList.addMember(myMember, 0);
    registerArtifact(memberList);
    pthread_create(&tid, NULL, &memberloop, this);
};