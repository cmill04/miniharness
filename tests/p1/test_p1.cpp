#include <cassert>
#include <iostream>
#include "core/conversation.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include <memory> 
#include <fstream>

namespace {
    class TestInputSourceTrue : public InputSource{
        public: 
        unsigned int turn_count{0};

        bool read_line(std::string& line) override {
            line = "testing";
            turn_count++;
            return true;
        }
    };

    class TestInputSourceEOF : public InputSource{
        public: 
        unsigned int call_count{0};
        unsigned int calls_before_eof;

        explicit TestInputSourceEOF(unsigned int n):
        calls_before_eof(n){}

        bool read_line(std::string& line) override {
          call_count++;
          if(call_count <= calls_before_eof){
            line ="testing";
            return true;
          }
          return false;
        }
    };

    class TestOutputSink : public OutputSink {
        public:
        void record(const Message& msg) override {

        }
        void display(const std::string& line) override {

        }
    };

    class TestReplyClient : public ModelClient{
        public: 
        using ModelClient::generate;
        void generate(const Conversation& conv, TokenSink& sink) override{
            sink.on_chunk("testing");
            sink.on_complete();
        }
    };

    class TestSentinelReplayClient : public ModelClient{
        public: 
        using ModelClient::generate;
        void generate(const Conversation& conv, TokenSink& sink)override{
            sink.on_chunk("words words <|end_conversation|>");
            sink.on_complete();
        }
    };
}

void test_empty_conversation_bounds(){
    Conversation c; 
    bool catch_flg{false};
    std::cout<<"before inputs"<<std::endl;
    assert(c.size()==0);
    std::cout<<"size is zero"<<std::endl;

    std::cout<<"begin should match end if empty"<<std::endl;
    assert(c.begin()==c.end());
    std::cout<<"Begin matched end"<<std::endl;

    std::cout<<"checking if escapes bounds"<<std::endl;
    try {c.at(0);}
    catch(const std::exception& e){
            catch_flg=true;
        }
        assert(catch_flg);
    std::cout<<"Stayed in bounds"<<std::endl;

}

void test_system_message(){
    Conversation c;
    Message m(Role::System, "system detail");
    c.append(m);
    Message n(Role::Assistant, "assistant detail");
    c.append(n);
    Message i(Role::User, "user detail");
    c.append(i);
    std::cout<<"checking role at to equal system"<<std::endl;
    assert(c.at(0).role()==Role::System);
    assert(c.at(0).content()=="system detail");
    std::cout<<"role was system at start"<<std::endl;

}

void deep_not_shallow(){
    Message m(Role::System, "system detail");
    Conversation real; 
    real.append(m);
    Conversation copy(real);

    std::cout<<"Testing if rule of 5 created deep copy"<<std::endl;
    assert(real.begin()!=copy.begin());
    assert(copy.at(0).role()==Role::System);
    assert(copy.at(0).content()=="system detail");
    std::cout<<"deep copy was created"<<std::endl;


}

void move_test(){

    Message m(Role::System, "system detail");
    Conversation start;
    start.append(m);
    const Message* adr = start.begin();
    Conversation end; 

    end = std::move(start);
    std::cout<<"testing move constructor"<<std::endl;
    assert(end.begin() == adr);
    assert(end.begin()!=start.begin());
    std::cout<<"move constructor works"<<std::endl;
}

void clean_text_scan_test(){
    SentinelScanner scanner("<|end_conversation|>");
    SentinelScanner::Out test1= scanner.feed("no sentinel here!");
    std::string hold = test1.safe_text; 
    std::cout<<"testing no sentinel string"<<std::endl;
    assert(!test1.sentinel_found);
    test1= scanner.flush();
    assert((hold + test1.safe_text)  =="no sentinel here!");
    assert(!test1.sentinel_found);
    std::cout<<"no sentinel passed"<<std::endl;

}

void sentinel_split_test(){

    std::string sen = "<|end_conversation|>";

    for (unsigned int i=1; i < sen.size(); i++){
        SentinelScanner scanner("<|end_conversation|>");

        std::string part1 = sen.substr(0,i);
        std::string part2 = sen.substr(i);

        SentinelScanner::Out test1= scanner.feed(part1);
        assert(!test1.sentinel_found);
        SentinelScanner::Out test2= scanner.feed(part2);
        assert(test2.sentinel_found);
    }
    std::cout<<"sentinel identified under all conditions"<<std::endl;

}

void false_alarm_test(){

    SentinelScanner scanner("<|end_conversation|>");
    SentinelScanner::Out test = scanner.feed("Sentinel fake here! <|end_world|>");
    std::cout<<"testing false sentinel input"<<std::endl;
    assert(!test.sentinel_found);
    std::string hold = test.safe_text;
    test = scanner.flush();
    assert(!test.sentinel_found);
    assert((hold + test.safe_text) == "Sentinel fake here! <|end_world|>");
    std::cout<<"false sentinel ignored"<<std::endl;

}

void harness_turn_limit_test(){
    HarnessConfig cfg;
    cfg.max_turns = 2; 

    Harness harness(std::make_unique<TestReplyClient>(),cfg);
    TestInputSourceTrue in;
    TestOutputSink out;
    StopReason test = harness.run(in, out);
    std::cout<<"testing turn limit return"<<std::endl;
    assert(test.kind == StopReason::Kind::TurnLimit);
    assert(in.turn_count==cfg.max_turns);
    std::cout<<"returned with turn limit"<<std::endl;

}

void harness_EOF_test(){
    HarnessConfig cfg;
    TestInputSourceEOF in(2);
    TestOutputSink out;

    Harness harness(std::make_unique<TestReplyClient>(),cfg);
    StopReason test = harness.run(in, out);
    assert(test.kind == StopReason::Kind::UserExit);
    std::cout<<"exited with user exit"<<std::endl;
}

void harness_sentinel_halt_test(){
    HarnessConfig cfg;
    cfg.max_turns = 6; 

    Harness harness(std::make_unique<TestSentinelReplayClient>(), cfg);
    TestInputSourceTrue in;
    TestOutputSink out;

    StopReason test = harness.run(in,out);
    assert(test.kind == StopReason::Kind::Sentinel);
}

void clean_destruction_test(){
    bool exception_escaped = false;
    try{
        Message m(Role::User, "words");
        Message n(Role::User, "apple");
        Message p(Role::User, "green");
        Conversation c;
        c.append(m);
        c.append(n);
        c.append(p);
        c.at(100); 

    }
    catch(...){
        exception_escaped = true;
    }
    assert(exception_escaped);
    std::cout<<"detected destruction"<<std::endl;
}

void transcript_round_trip_test(){
    std::string path = "/tmp/test.txt";
    std::ofstream file(path);

    file<<"role: user"<<std::endl;
    file<<"Howdy!"<<std::endl;
    file<<"---"<<std::endl;

    file<<"role: assistant"<<std::endl;
    file<<"Im the GOAT!"<<std::endl;
    file<<"---"<<std::endl;

    file<<"role: assistant"<<std::endl;
    file<<"NOOOO Im the GOAT!"<<std::endl;
    file<<"---"<<std::endl;

    file<<"role: user"<<std::endl;
    file<<"Woah fellas chill"<<std::endl;
    file<<"---"<<std::endl;

    ReplayModelClient client(path);
    Conversation c;                
    Message reply1 = client.generate(c);
    assert(reply1.role()==Role::Assistant);
    assert(reply1.content()=="Im the GOAT!");
    Message reply2 = client.generate(c);
    assert(reply2.role()==Role::Assistant);
    assert(reply2.content()=="NOOOO Im the GOAT!");
    bool exhausted = false;
    try{
    client.generate(c);
    }
    catch(...){
    exhausted = true;
    }
    assert(exhausted);


}


int main(){
    test_empty_conversation_bounds();
    std::cout<<"Bounds test passed!"<<std::endl;

    test_system_message();
    std::cout<<"System test passed!"<<std::endl;

    deep_not_shallow();
    std::cout<<"Deep copy test passed!"<<std::endl;

    move_test();
    std::cout<<"Move test passed!"<<std::endl;

    clean_text_scan_test();
    std::cout<<"Clean text test passed!"<<std::endl;

    sentinel_split_test();
    std::cout<<"Sentinel split test passed!"<<std::endl;

    false_alarm_test();
    std::cout<<"False alarm test passed!"<<std::endl;

    harness_turn_limit_test();
    std::cout<<"harnesss turn limit test passed!"<<std::endl;

    harness_EOF_test();
    std::cout<<"EOF test passed!"<<std::endl;

    harness_sentinel_halt_test();
    std::cout<<"Sentinel halt test passed!"<<std::endl;

    clean_destruction_test();
    std::cout<<"Destruction test passed!"<<std::endl;

    transcript_round_trip_test();
    std::cout<<"Round trip test passed!"<<std::endl;

    std::cout<<"ALL tests passed!"<<std::endl;
};