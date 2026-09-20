#include <cassert>
#include <iostream>
#include "core/conversation.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include <memory>
#include <fstream>
#include <vector>

namespace
{
    class TestInputSourceTrue : public InputSource
    {
    public:
        int turn_count{0};

        std::string read_line() override
        {
            turn_count++;
            return "testing";
        }

        bool is_eof() const override
        {
            return false;
        }
    };

    class TestInputSourceEOF : public InputSource
    {
    public:
        unsigned int call_count{0};
        unsigned int calls_before_eof;

        explicit TestInputSourceEOF(unsigned int n) : calls_before_eof(n) {}

        std::string read_line() override
        {
            call_count++;
            if (call_count <= calls_before_eof)
            {
                return "testing";
            }
            return "";
        }

        bool is_eof() const override
        {
            return call_count > calls_before_eof;
        }
    };

    class TestOutputSink : public OutputSink
    {
    public:
        void write(std::string_view /*text*/) override
        {
        }
    };

    class TestReplyClient : public ModelClient
    {
    public:
        using ModelClient::generate;
        void generate(const Conversation &conv, TokenSink &sink) override
        {
            sink.on_chunk("testing");
            sink.on_complete();
        }
    };

    class TestSentinelReplayClient : public ModelClient
    {
    public:
        using ModelClient::generate;
        void generate(const Conversation &conv, TokenSink &sink) override
        {
            sink.on_chunk("words words <|end_conversation|>");
            sink.on_complete();
        }
    };
}

void test_empty_conversation_bounds()
{
    Conversation c;
    bool catch_flg{false};
    std::cout << "before inputs" << std::endl;
    assert(c.size() == 0);
    std::cout << "size is zero" << std::endl;

    std::cout << "begin should match end if empty" << std::endl;
    assert(c.begin() == c.end());
    std::cout << "Begin matched end" << std::endl;

    std::cout << "checking if escapes bounds" << std::endl;
    try
    {
        c.at(0);
    }
    catch (const std::exception &e)
    {
        catch_flg = true;
    }
    assert(catch_flg);
    std::cout << "Stayed in bounds" << std::endl;
}

void test_system_message()
{
    Conversation c;
    Message m(Role::System, "system detail");
    c.append(m);
    Message n(Role::Assistant, "assistant detail");
    c.append(n);
    Message i(Role::User, "user detail");
    c.append(i);
    std::cout << "checking role at to equal system" << std::endl;
    assert(c.at(0).role() == Role::System);
    assert(c.at(0).content() == "system detail");
    std::cout << "role was system at start" << std::endl;
}

void deep_not_shallow()
{
    Message m(Role::System, "system detail");
    Conversation real;
    real.append(m);
    Conversation copy(real);

    std::cout << "Testing if rule of 5 created deep copy" << std::endl;
    assert(real.begin() != copy.begin());
    assert(copy.at(0).role() == Role::System);
    assert(copy.at(0).content() == "system detail");
    std::cout << "deep copy was created" << std::endl;
}

void move_test()
{

    Message m(Role::System, "system detail");
    Conversation start;
    start.append(m);
    const Message *adr = start.begin();
    Conversation end;

    end = std::move(start);
    std::cout << "testing move constructor" << std::endl;
    assert(end.begin() == adr);
    assert(end.begin() != start.begin());
    std::cout << "move constructor works" << std::endl;
}

void clean_text_scan_test()
{
    SentinelScanner scanner("<|end_conversation|>");
    SentinelScanner::Out test1 = scanner.feed("no sentinel here!");
    std::string hold = test1.safe_text;
    std::cout << "testing no sentinel string" << std::endl;
    assert(!test1.sentinel_found);
    test1 = scanner.flush();
    assert((hold + test1.safe_text) == "no sentinel here!");
    assert(!test1.sentinel_found);
    std::cout << "no sentinel passed" << std::endl;
}

void sentinel_split_test()
{

    std::string sen = "<|end_conversation|>";

    for (unsigned int i = 1; i < sen.size(); i++)
    {
        SentinelScanner scanner("<|end_conversation|>");

        std::string part1 = sen.substr(0, i);
        std::string part2 = sen.substr(i);

        SentinelScanner::Out test1 = scanner.feed(part1);
        assert(!test1.sentinel_found);
        SentinelScanner::Out test2 = scanner.feed(part2);
        assert(test2.sentinel_found);
    }
    std::cout << "sentinel identified under all conditions" << std::endl;
}

void false_alarm_test()
{

    SentinelScanner scanner("<|end_conversation|>");
    SentinelScanner::Out test = scanner.feed("Sentinel fake here! <|end_world|>");
    std::cout << "testing false sentinel input" << std::endl;
    assert(!test.sentinel_found);
    std::string hold = test.safe_text;
    test = scanner.flush();
    assert(!test.sentinel_found);
    assert((hold + test.safe_text) == "Sentinel fake here! <|end_world|>");
    std::cout << "false sentinel ignored" << std::endl;
}

void harness_turn_limit_test()
{
    HarnessConfig cfg;
    cfg.max_turns = 2;

    Harness harness(std::make_unique<TestReplyClient>(), cfg);
    TestInputSourceTrue in;
    TestOutputSink out;
    StopReason test = harness.run(in, out);
    std::cout << "testing turn limit return" << std::endl;
    assert(test.kind == StopReason::Kind::TurnLimit);
    assert(in.turn_count == cfg.max_turns);
    std::cout << "returned with turn limit" << std::endl;
}

void harness_EOF_test()
{
    HarnessConfig cfg;
    TestInputSourceEOF in(2);
    TestOutputSink out;

    Harness harness(std::make_unique<TestReplyClient>(), cfg);
    StopReason test = harness.run(in, out);
    assert(test.kind == StopReason::Kind::UserExit);
    std::cout << "exited with user exit" << std::endl;
}

void harness_sentinel_halt_test()
{
    HarnessConfig cfg;
    cfg.max_turns = 6;

    Harness harness(std::make_unique<TestSentinelReplayClient>(), cfg);
    TestInputSourceTrue in;
    TestOutputSink out;

    StopReason test = harness.run(in, out);
    assert(test.kind == StopReason::Kind::Sentinel);
}

void clean_destruction_test()
{
    bool exception_escaped = false;
    try
    {
        Message m(Role::User, "words");
        Message n(Role::User, "apple");
        Message p(Role::User, "green");
        Conversation c;
        c.append(m);
        c.append(n);
        c.append(p);
        c.at(100);
    }
    catch (...)
    {
        exception_escaped = true;
    }
    assert(exception_escaped);
    std::cout << "detected destruction" << std::endl;
}

void transcript_round_trip_test()
{
    std::string path = "/tmp/test.txt";
    std::ofstream file(path);

    file << "role: user" << std::endl;
    file << "Howdy!" << std::endl;
    file << "---" << std::endl;

    file << "role: assistant" << std::endl;
    file << "Im the GOAT!" << std::endl;
    file << "---" << std::endl;

    file << "role: assistant" << std::endl;
    file << "NOOOO Im the GOAT!" << std::endl;
    file << "---" << std::endl;

    file << "role: user" << std::endl;
    file << "Woah fellas chill" << std::endl;
    file << "---" << std::endl;

    ReplayModelClient client(path);
    Conversation c;
    Message reply1 = client.generate(c);
    assert(reply1.role() == Role::Assistant);
    assert(reply1.content() == "Im the GOAT!");
    Message reply2 = client.generate(c);
    assert(reply2.role() == Role::Assistant);
    assert(reply2.content() == "NOOOO Im the GOAT!");
    bool exhausted = false;
    try
    {
        client.generate(c);
    }
    catch (...)
    {
        exhausted = true;
    }
    assert(exhausted);
}

void growth_reallocation_test()
{
    Conversation c;
    std::vector<std::string> expected;

    for (int i = 0; i < 20; i++)
    {
        std::string content = "message " + std::to_string(i);
        Role r = (i % 2 == 0) ? Role::User : Role::Assistant;
        Message msg(r, content);
        c.append(msg);
        expected.push_back(content);

        assert(c.size() == static_cast<std::size_t>(i + 1));
    }

    std::cout << "checking all 20 messages survived reallocation" << std::endl;
    for (std::size_t i = 0; i < expected.size(); i++)
    {
        Role expected_role = (i % 2 == 0) ? Role::User : Role::Assistant;
        assert(c.at(i).role() == expected_role);
        assert(c.at(i).content() == expected[i]);
    }
    std::cout << "no corruption across reallocations" << std::endl;

    bool caught = false;
    try
    {
        c.at(20);
    }
    catch (const std::exception &)
    {
        caught = true;
    }
    assert(caught);
    std::cout << "bounds still enforced after growth" << std::endl;
}

void move_constructor_test()
{
    Message m(Role::System, "system detail");
    Conversation start;
    start.append(m);
    const Message *adr = start.begin();

    Conversation moved(std::move(start));

    std::cout << "testing move constructor steals pointer and zeroes source" << std::endl;
    assert(moved.begin() == adr);
    assert(moved.size() == 1);
    assert(moved.at(0).content() == "system detail");

    assert(start.size() == 0);
    assert(start.begin() == start.end());
    std::cout << "move constructor left source empty and valid" << std::endl;
}

void scanner_bounded_memory_test()
{
    const std::string sentinel = "<|end_conversation|>";
    const std::string unit = "<|end_";
    const std::size_t repeats = 50000;

    SentinelScanner scanner(sentinel);
    std::string full_input;
    std::string reassembled;

    for (std::size_t r = 0; r < repeats; r++)
    {
        for (char ch : unit)
        {
            full_input += ch;
            SentinelScanner::Out out = scanner.feed(std::string(1, ch));
            assert(!out.sentinel_found);
            assert(scanner.pending_size() <= sentinel.size() - 1);
            reassembled += out.safe_text;
        }
    }

    SentinelScanner::Out final_out = scanner.flush();
    assert(!final_out.sentinel_found);
    reassembled += final_out.safe_text;

    std::cout << "verifying reassembled text matches input exactly" << std::endl;
    assert(reassembled == full_input);
    std::cout << "pending_ stayed bounded across " << full_input.size() << " byte-at-a-time feeds" << std::endl;
}

int main()
{
    test_empty_conversation_bounds();
    std::cout << "Bounds test passed!" << std::endl;

    test_system_message();
    std::cout << "System test passed!" << std::endl;

    deep_not_shallow();
    std::cout << "Deep copy test passed!" << std::endl;

    move_test();
    std::cout << "Move test passed!" << std::endl;

    clean_text_scan_test();
    std::cout << "Clean text test passed!" << std::endl;

    sentinel_split_test();
    std::cout << "Sentinel split test passed!" << std::endl;

    false_alarm_test();
    std::cout << "False alarm test passed!" << std::endl;

    harness_turn_limit_test();
    std::cout << "harnesss turn limit test passed!" << std::endl;

    harness_EOF_test();
    std::cout << "harness EOF test passed!" << std::endl;

    harness_sentinel_halt_test();
    std::cout << "harness sentinel halt test passed!" << std::endl;

    clean_destruction_test();
    std::cout << "clean destruction test passed!" << std::endl;

    transcript_round_trip_test();
    std::cout << "transcript round trip test passed!" << std::endl;

    growth_reallocation_test();
    std::cout << "growth reallocation test passed!" << std::endl;

    move_constructor_test();
    std::cout << "constructor test passed!" << std::endl;

    scanner_bounded_memory_test();
    std::cout << "bounded memory test passed!" << std::endl;
};
