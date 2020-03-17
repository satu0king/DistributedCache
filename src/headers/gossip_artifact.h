#pragma once 

#include <string>
#include <buffer.h>

class GossipArtifact {
   public:
    virtual std::string getType() const = 0;
    virtual Buffer getPayload() const = 0;
    virtual void update(Buffer data) = 0;
    virtual void loop(long timestamp) = 0;
    GossipArtifact() {}
};