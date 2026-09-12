#include "harness/harness.h"

namespace{
    class HarnessSink : public TokenSink {
        public: 
        HarnessSink(SentinelScanner& scanner, OutputSink& out):
        scanner_(scanner), out_(out){}

        void on_chunk(std::string_view chunk) override{

        }

        void on_complete()override {

        }

        private: 
            SentinelScanner& scanner_;
            OutputSink& out_;
    };
}

Harness::harness(std::unique_ptr<ModelClient> model, HarnessConfig cfg):
 model_(std::move(model)), cfg_(cfg) {}

 StopReason Harness::run(InputSource& in, OutputSink& out){
    while(true){

    }
 } 