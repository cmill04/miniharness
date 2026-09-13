#include <cassert>
#include <iostream>
#include "core/conversation.h"

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
    Conversation copy(start);
    Message n(Role::Assistant, "assistant detail");
    start.append(n);
    Conversation end; 

    end.move(start);
    std::cout<<"testing move constructor"<<std::endl;
    assert(end.begin()==copy.begin());
    assert(end.begin()!=start.begin());
    std::Cout<<"move constructor works"<<std::endl;
}

int main(){
    test_empty_conversation_bounds();
    std::cout<<"Bounds test passed!"<<std::endl;

    test_system_message();
    std::cout<<"System test passed!"<<std::endl;

    deep_not_shallow();
    std::cout<<"deep copy test passed!"<<std::endl;
}