/*

Sharing Data between Threads
----------------------------

1. Problems with sharing data between threads

    1.1 If all shared data is read only , there is no problem, because the data read by one thread is unaffected by whether or not another thread is reading 
        the same data.
    1.2 Invariants - Statements that are always true about a particular data structure.
    1.3 Race Condition - In concurrency , a race condition where the outcome depends on the relative ordering of execution of operation on two or more threads.
    1.4 Data race is a specific type of race condition that arises because of concurrent modification to a single object.Causes undefined behavior.

2. Avoiding Problematic race condition

    2.1 wrap your data structure with a protection mechanism, to ensure that only the thread actually performing a modification can see the intermediate states
        where the invariants are broken. 
    2.2 modify the design of your data structure and its invariants so that modifications are done as a series of indivisible changes, each of which preserves the invariants.
        This is generally referred to as lock-free programming and is difficult to get right
    2.3 Another way of dealing with race conditions is to handle the updates to the data structure as a transaction, just as updates to a database are done within a transaction.
        The required series of data modifications and reads is stored in a transaction log and then committed in a single step. If the commit can’t proceed because the data structure
        has been modified by another thread, the transaction is restarted. This is termed software transactional memory (STM), and it’s an active research area at the time of writing.

3. Protecting shared data with mutexes

    3.1 mutex (mutual exclusion). Before accessing a shared data structure, you lock the mutex associated with that data, and when you’ve finished accessing the data structure, you unlock the mutex. 
        The Thread Library then ensures that once one thread has locked a specific mutex, all other threads that try to lock the same mutex have to wait until the thread that successfully locked the mutex 
        unlocks it. This ensures that all threads see a self-consistent view of the shared data, without any broken invariants.
    
    3.2 It’s common to group the mutex and the protected data together in a class rather than use global variables.
    3.3 Any code that has access to that pointer or reference can now access (and potentially modify) the protected data without locking the mutex.(Big Problem).
    3.4 Don’t pass pointers and references to protected data outside the scope of the lock, whether by returning them from a function, storing them in externally visible memory, or passing them as arguments to user-supplied functions.

4. Spotting race conditions inherent in interfaces

    4.1. Mutex Usage Does Not Guarantee Race Condition Avoidance
        Concept: Even though you may use a mutex to protect shared data, it's essential to identify what data needs protection. In complex data structures, protecting individual elements
         (e.g., nodes in a doubly linked list) isn't sufficient if the entire structure needs to be manipulated atomically.
        Key Example: Deleting a node in a doubly linked list requires protection of three nodes (the node to be deleted and its neighbors). If only pointers are protected individually, a race condition can still occur.
    
    4.2. Single Mutex for Entire Data Structure
        Concept: A simple solution is to use a single mutex that protects the entire data structure. This ensures that operations like deleting a node or accessing multiple nodes are atomic and thread-safe.
        Key Example: For a doubly linked list, using a single mutex ensures the delete operation on multiple nodes is safe.
    
    4.3. Interface Race Condition: Checking empty() and size() in std::stack
         Concept: Even with a mutex-protected stack (like std::stack), the interface itself can introduce race conditions. The values returned by empty() or size() could become invalid immediately after they're checked, leading to incorrect behavior.
         Key Example: If a thread checks empty() or size() and, before it can act on that information, another thread modifies the stack (e.g., push() or pop()), you have a classic race condition.

    4.4. Race Condition Between empty() and top()
         Concept: If a thread calls empty(), checks the stack is not empty, and then calls top() to access the element, another thread might pop() the last element before top() is called, leading to undefined behavior.
         Key Example: In multi-threaded code, checking empty() and then calling top() may not be safe because the stack could change between those calls.
    
    4.5. Interface-Level Solution: Combine top() and pop()
         Concept: The race condition between top() and pop() can be resolved by combining them into a single atomic operation. This avoids the race between checking the top element and popping it.
         Key Example: Instead of splitting top() and pop() into separate calls, you need a combined function that safely retrieves and pops the element in one atomic operation.

    4.6. Exception Safety in Combined top() and pop()
         Concept: If a combined top() and pop() function returns the popped element by value, exceptions (e.g., std::bad_alloc) could occur during the return process, leading to data loss. The object is already removed from the stack, but the copy could fail.
         Key Example: When popping an element of type std::vector<int>, memory allocation for the returned copy might fail, causing the popped value to be lost.

    4.7. Design Choice: Split top() and pop() for Exception Safety
         Concept: The std::stack interface was designed with exception safety in mind. By splitting top() and pop(), the stack remains intact even if copying the top element fails, avoiding loss of data.
         Key Example: Splitting top() and pop() ensures that if the copy constructor throws, the data is still in the stack.

    4.8. Option 1: Pass a Reference to pop()
         Concept: To avoid exceptions when returning by value, one approach is to pass a reference to pop() where the popped value will be stored. This ensures that no copy or move is needed.
         Key Example: some_stack.pop(result) stores the popped value directly into result without requiring memory allocation or copy operations.

    4.9. Option 2: Require a No-Throw Copy/Move Constructor
         Concept: Another solution is to restrict the stack to types that have no-throw copy or move constructors, ensuring the return by value doesn't throw exceptions.
         Key Example: Using std::is_nothrow_copy_constructible and std::is_nothrow_move_constructible, the type safety can be enforced at compile time.

    4.10. Option 3: Return a Pointer to the Popped Item
          Concept: Returning a pointer to the popped item (e.g., std::shared_ptr) avoids exceptions since copying a pointer doesn't throw. This, however, introduces memory management overhead.
          Key Example: Returning a std::shared_ptr to the popped element ensures memory safety and avoids exceptions but may add overhead compared to a simple value return.

    4.11. Option 4: Provide Multiple Pop Overloads
          Concept: For flexibility, both the reference-passing (Option 1) and the pointer-returning (Option 3) approaches can be provided as overloads, allowing the user to choose the most suitable approach.
          Key Example: A thread-safe stack can offer both a pop(result) function that accepts a reference and a pop() that returns a std::shared_ptr, giving flexibility based on performance and exception-safety needs.

5. Deadlock

    5.1 The common advice for avoiding deadlock is to always lock the two mutexes in the same order: if you always lock mutex A before mutex B, then you’ll never deadlock.
    5.2 std::lock— a function that can lock two or more mutexes at once without risk of deadlock
*/