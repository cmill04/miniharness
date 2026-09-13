#pragma once 
#include "model/model_client.h"



class ScriptedModelClient : public ModelClient{
    public: 
    struct SystemInfo{
        bool roleSystem{false};
        std::string systemMsg; 
    };

    using ModelClient::generate;
    explicit ScriptedModelClient(const std::string& path);
    void generate(const Conversation& conv, TokenSink& sink) override;

    const SystemInfo& system_info() const {return SysInfo_;}

    private: 
    BlockList blocks_;
    std::size_t next_ =0; 
    SystemInfo SysInfo_; 
};