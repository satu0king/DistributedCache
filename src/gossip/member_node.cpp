
#include "member_node.h"


#define TIMECHECK(x, message, limit)                                                         \
                                                                            \
        auto start_##message = std::chrono::high_resolution_clock::now();              \
        x;                                                                   \
        auto finish_##message = std::chrono::high_resolution_clock::now();             \
        auto time_##message = std::chrono::duration_cast<std::chrono::milliseconds>(   \
                        finish_##message - start_##message)                                      \
                        .count();                                            \
        if (time_##message > limit)                                                       \
            std::cout << "Time taken to run " << #message << " " << time_##message << std::endl;                                  \
    

MembershipList MemberNode::getList() {
    std::lock_guard guard(lock);
    MembershipList list = memberList;
    return list;
}

void MemberNode::addIntroducer(std::string ip, int port) {
    memberList.addMember(Member(ip, port));
}

void MemberNode::registerArtifact(GossipArtifact& artifact) {
    gossipArtifacts.emplace(artifact.getType(), &artifact);
}

Buffer MemberNode::generateGossipPayload() {
    Buffer payloadData;
    for (auto& [key, artifact] : gossipArtifacts) {
        payloadData.addBuffer(artifact->getPayload());
    }
    GossipPayload payloadHeader;
    payloadHeader.gossipArtifactCount = gossipArtifacts.size();

    Buffer data(&payloadHeader, sizeof(payloadHeader));
    data.addBuffer(payloadData);

    return data;
}

MemberNode::MemberNode(Address myAddress, int startRange)
    : myMember(myAddress, startRange) {
    memberList.addMember(myMember);
    registerArtifact(memberList);
    tid = std::thread(&MemberNode::threadLoop, this);
};

int MemberNode::heartbeat() { return myMember.heartbeat; }

void MemberNode::loop() {
    TIMECHECK(std::unique_lock unique_lock(lock), lock_time, 0);

    myMember.heartbeat++;
    myMember.timestamp++;
    int timestamp = heartbeat();

    for (auto& [key, artifact] : gossipArtifacts) artifact->loop(timestamp);

    memberList.addMember(myMember);

    // std::cout << myMember.address.toString() << " : " << memberList.size()
    //           << " " << heartbeat() << std::endl;

    std::vector<int> sendList = selectKItems(
        memberList.size(), std::min(GOSSIP_K, (int)memberList.size()));
        
    auto data = generateGossipPayload();
    unique_lock.unlock();

    for (int i : sendList) {
        TIMECHECK(sendMemberList(memberList[i], data), send_gossip, 0);
    }
}

int MemberNode::sendMemberList(Member& member, Buffer& data) {
    if (member.address == myMember.address) return 0;
    TIMECHECK(int connection = Address::getConnection(member.address), Connection_Acquire_Time, 0);
    if (connection == -1) {
        std::cout << "connection drop to " << (std::string)member.address
                  << std::endl;
        return -1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    RequestType type = RequestType::GOSSIP;
    loop_write(connection, &type, sizeof(type));
    loop_write(connection, data.data(), data.size());
    close(connection);
    auto finish = std::chrono::high_resolution_clock::now();

    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(finish - start)
            .count();
    if (time)
        std::cout << "Time taken to " << member.address.toString() << " "
                  << time << std::endl;

    return 0;
}

void MemberNode::receiveGossip(int connection) {
    GossipPayload header;
    int read_bytes = loop_read(connection, &header, sizeof(header));
    assert(read_bytes == sizeof(header));

    int gossipArtifactCount = header.gossipArtifactCount;

    while (gossipArtifactCount--) {
        PayloadHeader header;
        read_bytes = loop_read(connection, &header, sizeof(header));
        assert(read_bytes == sizeof(header));
        std::string type = header.type;
        assert(gossipArtifacts.count(type));
        Buffer data(header.size);
        read_bytes = loop_read(connection, data.data(), header.size);
        assert(read_bytes == header.size);

        std::lock_guard guard(lock);
        gossipArtifacts[type]->update(data);
    }
}

Address MemberNode::getNearestNode(int key) {
    std::lock_guard guard(lock);
    auto v = memberList.getNearestNode(key);
    return v;
}
std::vector<int> MemberNode::selectKItems(int n, int k) {
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

void MemberNode::threadLoop() {
    usleep(500 * 1000);
    srand(time(NULL));

    int run_times = 50;
    while (run_times--) {
        TIMECHECK(loop(), main_loop, 10000);

        usleep(HEARTBEAT_PERIOD * 1000);
    }

    std::cout << "OVER\n";
}
