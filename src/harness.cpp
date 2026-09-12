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

    }
 } 