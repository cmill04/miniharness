#include "harness/harness.h"

namespace{
    class HarnessSink : public TokenSink {
        public: 
        HarnessSink(SentinelScanner& scanner, OutputSink& out):
        scanner_(scanner), out_(out){}

        const std::string& text() const { return text_; }
        bool sentinel_found() const { return sentinelFound_; }

        void on_chunk(std::string_view chunk) override{
            text_ += chunk; 
            SentinelScanner::Out result= scanner_.feed(chunk);
            if(!result.safe_text.empty()){
                out_.display(result.safe_text);
            } 
            if(result.sentinel_found){
                sentinelFound_ = true; 
            }
        };

        void on_complete()override {
            SentinelScanner::Out result = scanner_.flush();
            if (!result.safe_text.empty()) {
                out_.display(result.safe_text);
            }
        };

        private: 
            SentinelScanner& scanner_;
            OutputSink& out_;
            std::string text_; 
            bool sentinelFound_{false}; 
    };
}

Harness::Harness(std::unique_ptr<ModelClient> model, HarnessConfig cfg):
 model_(std::move(model)), cfg_(cfg) {}

 StopReason Harness::run(InputSource& in, OutputSink& out){
    while(true){

        if(!cfg_.sys_string.empty()){
            Message b{Role::System, cfg_.sys_string};
            conv_.append(b);
            out.record(b);
        }

        if((conv_.size()/2)>=cfg_.max_turns){
            return StopReason{StopReason::Kind::TurnLimit,"Turn Limit Reached!"};
        }

        std::string line; 
        if(!in.read_line(line)){
            return StopReason{StopReason::Kind::UserExit, "User Exited"};
        }
        else{
            Message m(Role::User, line); 
            conv_.append(m);
            out.record(m);
        }
        SentinelScanner scanner(cfg_.sentinel);
        HarnessSink sink(scanner, out);
        try {model_->generate(conv_,sink);}
        catch(const std::exception& e){
            return StopReason{StopReason::Kind::ClientError, "Error Exit!"};
        }
        Message a(Role::Assistant, sink.text());
        conv_.append(a);
        out.record(a);
        if(sink.sentinel_found()){
            return StopReason{StopReason::Kind::Sentinel, "Sentinel Exit"};
        }
        
    }
 } 