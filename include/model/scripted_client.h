#pragma once 
#include "model/model_client.h"

class ScriptedModelClient : public ModelClient{
    public: 
    using ModelClient::generate;
    explicit ScriptedModelClient(const std::string& path);
    void generate(const Conversation& conv, TokenSink& sink) override;

    private: 
    BlockList blocks_;
    std::size_t next_ =0; 
};