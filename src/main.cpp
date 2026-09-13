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
    unsigned int max_turns =20;
    std::string save_path;
    
    for(int i =1;i<argc; i++){
        std::string arg = argv[i];
        if(arg == "--script" && i+1 <argc){
            script_path = argv[i+1];
            i++;
        }
        if(arg == "--max-turns" && i+1 <argc){
            max_turns = static_cast<unsigned int>(std::stoul(argv[i+1]));
            i++;
        }
        if(arg == "--save" && i+1 <argc){
            save_path = argv[i+1];
            i++;
        }
    }

    std::unique_ptr<ScriptedModelClient>model 
        = std::make_unique<ScriptedModelClient>(script_path);

        HarnessConfig cfg;
        cfg.max_turns = max_turns; 

        if(model->system_info().roleSystem){
            cfg.sys_string = model->system_info().systemMsg; 
        }


        Harness harness(std::move(model), cfg);
        ConsoleInputSource in;

        ConsoleOutputSink out(save_path);
        StopReason result = harness.run(in, out);
        std::cout<<result.detail<<std::endl; 
};