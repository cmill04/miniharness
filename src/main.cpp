#include <iostream>
#include <memory> 
#include <string> 
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include "model/scripted_client.h"
#include "harness/harness.h"

namespace {
    class ConsoleInputSource : public InputSource {
        public: 
        bool read_line(std::string& line) override {
            if (std::getline(std::cin, line)) 
            {return true;} 
        else 
             {return false;}
}
};

    class ConsoleOutputSink : public OutputSink {
        public:
        explicit ConsoleOutputSink(const std::string& save_path){
            if(!save_path.empty()){
              file_.open(save_path);
                if (!file_){
                    openCheck = false;
                }
                else openCheck = true; 
            }
            else openCheck = false; 
            
        }

                std::string roleStrConv(Role role) {
                switch(role) {
                    case Role::System: return "system";
                    case Role::User: return "user";
                    case Role::Assistant: return "assistant";
                }
                throw std::runtime_error("unknown role");
            }

        void record(const Message& msg) override {
            if(openCheck){
            file_<<"role: "<<roleStrConv(msg.role())<<std::endl;
            file_<<msg.content()<<std::endl;
            file_<<"---"<<std::endl;
            }
            
        }

        void display(const std::string& line) override{
            std::cout<<line<<std::endl;
        }

        private: 
        bool openCheck; 
        std::ofstream file_; 
    };
}

int main(int argc, char** argv){



    std::string script_path;
    std::size_t max_turns =20;
    std::string save_path;
    std::unique_ptr<ModelClient>model 
        = std::make_unique<ScriptedModelClient>(script_path);


        HarnessConfig cfg;
        cfg.max_turns = max_turns; 

        Harness harness(std::move(model), cfg);
        ConsoleInputSource in;
}