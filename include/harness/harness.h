#pragma once
 
#include <memory>
 
#include "core/conversation.h"
#include "core/message.h"
#include "model/model_client.h"
 
struct HarnessConfig{
    std::string sentinel{"<|end_conversation|>"}; 
    unsigned int max_turns{20}; 
    std::string sys_string; 
};

class InputSource{
    public:
    virtual ~InputSource() = default;
    virtual bool read_line(std::string& line) = 0;
}; 

class OutputSink{
    public:
    virtual ~OutputSink() = default;
    virtual void record(const Message& msg) = 0;
    virtual void display(const std::string& line) = 0; 
};
 
class Harness {
public:
    Harness(std::unique_ptr<ModelClient> model, HarnessConfig cfg);
    StopReason run(InputSource& in, OutputSink& out);
 
private:
    std::unique_ptr<ModelClient> model_;
    Conversation                 conv_;
    HarnessConfig                cfg_;
};