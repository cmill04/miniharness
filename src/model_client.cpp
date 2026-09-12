#include "model/model_client.h"
#include <fstream>
#include <stdexcept> 

namespace {
class CollectingSink : public TokenSink {
public:
    void on_chunk(std::string_view chunk) override {
        
         text.append(chunk);
    }
    void on_complete() override {}
    std::string text;
};
}

Message ModelClient::generate(const Conversation& conv) 
{
    CollectingSink sink;
    generate (conv,sink); 
    return Message(Role::Assistant, sink.text);
}

namespace {
    std::string rmv_r(std::string s) {
        if(!s.empty() && s.back()=='\r'){
            s.pop_back();
        }
        return s;
    }

    bool starts_with(const std::string& s, const std::string& pre) {
        return ((s.size()>=pre.size()) && (s.compare(0, pre.size(), pre)==0)); 
    }

}

BlockList parse_blocks(const std::string& path) {
    std::ifstream file(path);
    if(!file) {
        throw std::runtime_error("File failed to open" + path);
    }
    BlockList blocks; 
    std::string role;
    std::string content;  
    std::size_t chunk_size =0;
    bool have_role = false;     //init to false since no parsing done
    bool have_content = false;  //init to false since no parsing done 
    std::string line;

    while (std::getline(file, line)){
        line = rmv_r(line); //allows for testing on windows because it ends with a newline and character return (learned from 306 :) )
        if (line == "---") {

            if(have_role){
                blocks.push_back(ScriptBlock{role, content, chunk_size});
            }
            role.clear();
            content.clear();
            chunk_size =0;
            have_content = false; 
            have_role = false;
            continue; 
        }

        if(!have_role){
            if (line.empty()){
                continue; 
            }
        

        if(starts_with(line, "chunk:")){
            chunk_size = std::stoul(line.substr(7));
            continue; 
        }

        if(starts_with(line, "role:")){
            role = line.substr(6);
            have_role = true;
            continue; 
        }

        throw std::runtime_error("expected role: content but got: " +line);
    }

    if (have_content) {
        content += "\n";
    }
    content += line;
    have_content =true;
}


if(have_role){ 
    blocks.push_back(ScriptBlock{role, content, chunk_size});
}
return blocks; 
}