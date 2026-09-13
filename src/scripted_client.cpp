#include "model/scripted_client.h"
#include <algorithm> 
#include <stdexcept>

ScriptedModelClient::ScriptedModelClient(const std::string& path):
blocks_(parse_blocks(path)){
    if((!blocks_.empty())&&(blocks_.front().role == "system")){
        SysInfo_.roleSystem = true;
        SysInfo_.systemMsg = blocks_.front().content; 
        blocks_.erase(blocks_.begin());
    }

}
void ScriptedModelClient::generate(const Conversation& conv, TokenSink& sink){
    if(next_ >= blocks_.size()){
        throw std::runtime_error("no more scripted replies");
    }
    const ScriptBlock& block = blocks_[next_];
    next_++;

    if(block.chunk_size==0){
        sink.on_chunk(block.content);
    }
    else {
        std::size_t pos =0; 
        while (pos < block.content.size()){
            std::size_t len = std::min(block.chunk_size,block.content.size() - pos);
            sink.on_chunk(std::string_view(block.content).substr(pos, len));
            pos += len; 
        }
    }
    sink.on_complete();
}