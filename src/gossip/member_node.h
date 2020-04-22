#pragma once
#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include "buffer.h"
#include "gossip_artifact.h"
#include "membership_list.h"
#include "requests.h"

#define HEARTBEAT_PERIOD 100
#define GOSSIP_K 2

class MemberNode {
    Member myMember;
    MembershipList memberList;
    pthread_t tid;

    std::unordered_map<std::string, GossipArtifact*> gossipArtifacts;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

   public:
    MembershipList getList();

    void addIntroducer(std::string ip, int port);

    MemberNode(Address myAddress, int startRange = -1);

    void registerArtifact(GossipArtifact& artifact);

    Buffer generateGossipPayload();

    int heartbeat();

    void loop();

    int sendMemberList(Member& member, Buffer data);

    void receiveGossip(int connection);

    std::vector<int> selectKItems(int n, int k);

    Address getNearestNode(int key);
    Address getAddress() { return myMember.address; }
};

void* memberloop(void* ptr);