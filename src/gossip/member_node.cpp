
#include "member_node.h"

MembershipList MemberNode::getList() {
    pthread_mutex_lock(&lock);
    MembershipList list = memberList;
    pthread_mutex_unlock(&lock);
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

MemberNode::MemberNode(Address myAddress) : myMember(myAddress) {
    memberList.addMember(myMember);
    registerArtifact(memberList);
    pthread_create(&tid, NULL, &memberloop, this);
};

int MemberNode::heartbeat() { return myMember.heartbeat; }

void MemberNode::loop() {
    myMember.heartbeat++;
    myMember.timestamp++;
    int timestamp = heartbeat();

    pthread_mutex_lock(&lock);

    for (auto& [key, artifact] : gossipArtifacts) {
        artifact->loop(timestamp);
    }

    memberList.addMember(myMember);

    // std::cout << myMember.address.toString() << " : " << memberList.size()
    //           << std::endl;

    std::vector<int> sendList = selectKItems(
        memberList.size(), std::min(GOSSIP_K, (int)memberList.size()));
    auto data = generateGossipPayload();
    for (int i : sendList) sendMemberList(memberList[i], data);
    pthread_mutex_unlock(&lock);
}

int MemberNode::sendMemberList(Member& member, Buffer data) {
    if (member.address == myMember.address) return 0;
    int connection = Address::getConnection(member.address);
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

void MemberNode::receiveGossip(int connection) {
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
        gossipArtifacts[type]->update(data);
    }
    pthread_mutex_unlock(&lock);
    close(connection);
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

