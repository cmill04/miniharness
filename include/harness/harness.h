#pragma once
 
#include <memory>
 
#include "core/conversation.h"
#include "model/model_client.h"
 
struct HarnessConfig;

class InputSource{
    public:
    virtual ~InputSource() = default;
    virtual bool read_line(std::string& line) = 0;
}; 

class OutputSink;
 
class Harness {
public:
    Harness(std::unique_ptr<ModelClient> model, HarnessConfig cfg);
    StopReason run(InputSource& in, OutputSink& out);
 
private:
    std::unique_ptr<ModelClient> model_;
    Conversation                 conv_;
    HarnessConfig                cfg_;
};