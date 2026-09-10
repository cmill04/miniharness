#pragma once
 
#include <memory>
 
#include "core/conversation.h"
#include "model/model_client.h"
 
struct HarnessConfig;
class InputSource;
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