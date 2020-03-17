#include "membership_list.h"


void Member::update(const Member& other, long timestamp) {
    assert(address == other.address);
    if (other.heartbeat <= heartbeat) return;
    heartbeat = other.heartbeat;
    this->timestamp = timestamp;
}

Buffer Member::serialize() const {
    MemberSerialized data;
    strcpy(data.ip, address.ip.c_str());
    data.port = address.port;
    data.heartbeat = heartbeat;

    Buffer bufferdata(&data, sizeof(data));
    return bufferdata;
}



    MembershipList::MembershipList() : timestamp(0) {}
    int MembershipList::getTimestamp() { return timestamp; }
    void MembershipList::updateTimestamp(long timestamp) {
        this->timestamp = std::max(timestamp, this->timestamp);
    }
    Member& MembershipList::getMember(Address address) {
        assert(hasMember(address));
        return memberList[memberMap[address]];
    }
    std::string MembershipList::getType() const { return "membershiplist"; }
    Buffer MembershipList::getPayload() const {
        Buffer memberData;
        for (auto& member : memberList) {
            if (member.timestamp + T_FAIL > timestamp)
                memberData.addBuffer(member.serialize());
        }
        PayloadHeader payload;
        strcpy(payload.type, getType().c_str());
        payload.size = memberData.size();
        Buffer data(&payload, sizeof(payload));
        data.addBuffer(memberData);
        return data;
    }

    void MembershipList::update(Buffer data) {
        int size = data.size();
        int recordCount = size / sizeof(MemberSerialized);
        assert(size % sizeof(MemberSerialized) == 0);

        MemberSerialized* records =
            reinterpret_cast<MemberSerialized*>(data.data());
        for (int i = 0; i < recordCount; ++i) {
            addMember(Member(records[i]));
        }
    }

    void MembershipList::loop(long timestamp) {
        updateTimestamp(timestamp);
        removeDead();
    }

    void MembershipList::removeDead() {
        std::vector<Member> to_delete;

        for (auto& member : memberList) {
            if (member.timestamp + T_REMOVE < timestamp)
                to_delete.push_back(member);
        }

        for (auto& member : to_delete) deleteMember(member);
    }

    bool MembershipList::hasMember(Address address) { return memberMap.count(address); }
    void MembershipList::addMember(Member member) {
        updateTimestamp(timestamp);
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

    size_t MembershipList::size() const { return memberList.size(); }

    Member& MembershipList::operator[](int i) { return memberList[i]; }

    void MembershipList::deleteMember(Address& address) {
        assert(hasMember(address));

        std::cout << "Member Lost!" << std::endl;
        std::cout << address.toString() << std::endl;

        int i = memberMap[address];

        memberList[i] = memberList.back();
        memberMap[memberList.back().address] = i;

        memberList.pop_back();
        memberMap.erase(address);
    }

    void MembershipList::deleteMember(Member& member) { deleteMember(member.address); }
