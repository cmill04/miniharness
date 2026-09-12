#pragma once 
#include "model/model_client.h"

class ReplayModelClient : public ModelClient{
    public: 
    using ModelClient::generate; 
    explicit ReplayModelClient(const std::string& path);
    void generate(const Conversation& conv, TokenSink& sink) override; 

    private: 
    BlockList blocks_;
    std::size_t next_=0; 
};    
