# Design Log — Project 2: The Conversation Loop

## Growth factor choice for dynamic array

The array grows when the size reaches the capacity. However, when it meets capacity it grows the array by double the size. This is a more efficient way than growing the array every time it has something added to it. For example growing one at a time would be (1+2+3+4+...n) vs doubling (1+2+4+8+16+...n). This doubling method has a cost of 2n-1. To find the amortized cost per append is divided by the number of appends which is n. Also 2n-1 is equivalent to O(n) and n is equivalent to O(n). (2n-1)/n = 2-(1/n) O(2-1/n) = O(1). This shows that amortized cost is O(1).

## Rule of Five handling

First of all inside the conversation class all five elements of the rule of 5 are there: copy constructor, destructor, assignment operator, move constructor, and move assignment. These elements are also created to avoid the "common pitfalls". The copy creates a deep copy rather than a shallow copy and this is tested in test "deep_not_shallow" of the test file. The test shows that addresses are not the same when copied. The move constructor is also tested by showing that the pointer is copied and the old object is cleared out and this is tested in "move_constructor_test". The assignment operator allows for safe copying and is shown in the "copy_assignment_test". The destructor calls delete [] data_ unconditionally which protects against an empty Conversation because delete will do nothing when the pointer is null.

## How pending buffer never exceeds the sentinel length

The sentinel scanner keeps text that does not match the sentinel in the pending buffer. It does this by releasing known text not including the sentinel text as safe text because the sentinel could arrive in two separate feed calls depending on the chunk size. This allows for a sentinel to be detected when it is received in multiple chunks as well as one chunk. The scanner guarantees that the size of pending never exceeds the size of the sentinel -1. When text is fed and the sentinel is not found the code keeps only the last sentinel size -1 characters in pending like this (pending_ = pending_.substr(pending_.size()-(sentinel_.size()-1), sentinel_.size()-1)). The rest is placed into safe text because we know it does not contain any portion of the sentinel string. This length ensures that every call never lets pending exceed the sentinel length.

## What would I do differently

I think overall I am very happy with how my code turned out. Having the file structure breaking the tasks up into the different files was nice and I like that. When I started the first couple of header files I ended up including a lot of the logic inside of the header files but then switched to including the logic in the cpp files and I wish I had started with that design as it would have saved time that I spend changing those couple of sections over to be separated between .h and cpp files. I also could have separated the harness run structure into different functions to improve readability and organization considering it was one of the more complex parts of the project.
