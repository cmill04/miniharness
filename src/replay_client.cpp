#include "model/replay_client.h"
#include <stdexcept>

namespace {
    BlockList filterAssistant(const BlockList& all){
        BlockList result; 
        for(const ScriptBlock& b : all){
            if(b.role == "assistant"){
                result.push_back(b);
            }
        }
        return result; 
    }
}

ReplayModelClient::ReplayModelClient(const std::string& path):
blocks_(filterAssistant(parse_blocks(path))){}

void ReplayModelClient::generate(const Conversation& conv, TokenSink& sink){
    if(next_>=blocks_.size()){
        throw std::runtime_error("no more replies");
    }
    const ScriptBlock& block = blocks_[next_];
    next_++; 

    sink.on_chunk(block.content);
    sink.on_complete();
}