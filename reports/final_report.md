## Lock-free Cuckoo Filter

**Yifei Wang** (yifeiw3)
**Yuchen Wang** (yw7)

### Summary

### 1. Background

#### 1.1 Cuckoo Hash Table

#### 1.2 Cuckoo Filter

#### 1.3 Fine-grained Locked Cuckoo Filter

### 2. Our Approach

#### 2.1 Overview

#### 2.2 Data Structures
This subsection describes several data structures used in our lock-free cuckoo filter implementation.

**4-way Associative Cuckoo Hash Table**
Basically, our cuckoo hash table structure is fairly similar to the one used in cuckoo filter's original paper [1] with several modifications, as shown below.

<img src="/Users/xsxsz/Desktop/15618/project/pic/table.png" style="zoom:30%" />

For each item to be inserted into the hash table, two hash functions will be used to map it to two distinct buckets, namely the primary and the alternative bucket. Instead of directly storing fingerprints inside the hash table, each fingerprint is put in a piece of dynamically allocated memory. Subsequently, we store an augmented 64-bit pointer (elaborated in the subsequent subsection) in the corresponding table entry, pointing to the allocated memory. The primary rationale behind this adjustment is that atomic compare-and-swap operation cannot be performed on a C++ string instance, while GCC provides `__sync_bool_compare_and_swap` intrinsic to update a 64-bit variable, by which we could modifiy table entries in a lock-free manner.

**Table Pointer**
The construction of the table pointer, essentially a conventional x86_64 pointer augmented with additional information, closely follows the approach outlined in paper [2]. Its structural details are illustrated in the figure below.

<img src="/Users/xsxsz/Desktop/15618/project/pic/pointer.png" style="zoom:20%" />

The fundamental concept underlying this data structure involves leveraging unused fields within a 64-bit pointer to incorporate additional information. Specifically, the 48-bit real pointer field encompasses the lower 48 bits of a real x86_64 pointer. On the x86_64 platform, only the lower 48 bits of a 64-bit pointer are used to represent valid pointers, with the remaining bits set to zero. Consequently, we can utilize these lower 48 bits to store a pointer indicating the fingerprint, while the higher 16 bits serve to track the number of times this entry has been relocated. This relocation counter is crucial for preventing false misses in *find* operations. Moreover, due to memory address alignment, the least significant bit of an x86_64 pointer will always be zero. We leverage this bit to store a flag, which indicates whether the pointer is presently being relocated.

#### 2.3 Find, Insert and Remove Operations

**Find Operation**

Locating a matching fingerprint in the hash table involves a sequence of queries on potential hash table entries. In our 4-way associative cuckoo hash table, a total of 8 entries must be examined for each input key. Upon finding a matching fingerprint, the find operation returns the location of the entry.

However, this approach may overlook existing fingerprints if they are replaced during the search, leading to false misses. For instance, a matching fingerprint might be stored in its alternative bucket while the find operation queries the primary bucket. By the time the find operation proceeds to search the alternative bucket, the fingerprint may have been relocated to its primary bucket. In such cases, the find operation fails to locate the fingerprint, even if it exists in the hash table, as illustrated in the figure below.

<img src="/Users/xsxsz/Desktop/15618/project/pic/move.png" style="zoom:25%" />

To tackle this issue, we introduce a replacement rule: each fingerprint can only be replaced to the same entry in other buckets. For example, in the above figure, the fingerprint (more precisly, a table pointer pointing to it) stored in the 3rd entry of the alternative bucket can only be relocated to the 3rd entry in its primary bucket. Consequently, each entry now has only two possible locations, regardless of the number of replacements happened. We employ a two-round query approach to find fingerprints while mitigating false misses. This process utilizes each entry's counter value, which records how many times the current entry has been relocated. It is important to note that for simplicity, we omit discussions on hazard pointer-related functionalities, which will be elaborated in detail in section 2.5.

In the first round of the query, besides performing the same operations as in the naive query method, we also record each entry's counter value. In the second round, we iterate through all entry pairs (i.e., two entries in the primary and alternative buckets at the same location) and compare their current counter value with their previously recorded value in the first round (using the `check_counter` function). If more than two relocations occurred during the search, there is a possibility that the search missed an existing value, hence the *find* operation is restarted. Otherwise, we can confirm that the key does not exist.

**Insert Operation**

The *insert* operation in a lock-free environment closely mirrors its counterpart in the sequential cuckoo filter. Initially, we calculate the locations of the primary and alternative buckets and store the fingerprint in dynamically allocated memory. Subsequently, we execute the following operations within an infinite loop:

1. Attempt to find an empty entry among the 8 possible entries in two buckets.
2. If an empty entry is found, and the Compare-and-Swap (CAS) operation to store the new table pointer succeeds, the *insert* operation returns `true`.
3. If no empty entry is found, we invoke the *relocate* operation on all entries in the primary bucket until an empty entry is created. This relocation is carried out in a random order: we generate an array [0, 1, 2, 3], shuffle it randomly, and use it as the order for evicting entries. This technique ensures the equitable distribution of newly inserted keys among all table entries, fostering better load balance.
4. If at least one *relocate* operation succeeds, the *insert* operation is initiated again. Conversely, if no *relocate* operation succeeds, which signifies insufficient capacity in the hash table, the *insert* operation fails. Due to the inherent characteristic of cuckoo filters, which do not retain original keys, rehashing is not feasible in this scenario. Since this limitation pertains to cuckoo filters rather than our lock-free implementation, it is not addressed in our project.

**Remove Operation**
Deleting a fingerprint in the cuckoo hash table is a trivial process. Initially, we invoke the find operation to determine the location of the matching fingerprint. It is important to note that it is the responsibility of the filter user to ensure that the key to be deleted is stored in the hash table, guaranteeing that the find operation always returns a valid location. Failure to do so results in undefined behavior of the filter, because if so a fingerprint generated by another key may be deleted instead.

Using the location returned by the *find* operation, we make an attempt to remove the entry with an atomic CAS operation (`__sync_bool_compare_and_swap`). If the CAS operation succeeds, the *remove* operation places the removed pointer in a retired pointer list (discussed in subsection 2.5) and returns `true`. In case that the CAS operation fails, the entire removal operation is executed again.

#### 2.4 Relocate Operation

The *relocate* operation is invoked when the *insert* operation is unable to find an empty entry. It initiates a sequence of entry replacements until an empty entry is found and swapped to the destination (i.e., the entry on which *relocate* is called). Essentially, the *relocate* operation seeks a replacement path shorter than a predefined length that terminates at an empty entry.

In line with the replacement rule proposed in the *insert* operation, each entry has only one possible replacement location during relocation. Consequently, a single replacement path involves entries located at the same position in each bucket. For instance, to relocate the $i_{th}$ entry of a bucket, all entries in the replacement path will be the $i_{th}$ entry in a certain bucket. This process is illustrated in the figure below."

<img src="/Users/xsxsz/Desktop/15618/project/pic/relocate.png" style="zoom:25%" />

The relocate operation generally comprises two phases: path discovery and entry replacement. In the path discovery phase, the algorithm tries to find the previously mentioned replacement path. Should it encounter an entry marked for relocation (indicated by its relocation mark set to true), signifying that it is presently in the process of being relocated, the relocate operation invokes a helper function, `help_relocate`, to move the entry to its destination. Subsequently, the path search process resumes. If the search process exceeds the predefined length threshold without locating an empty entry, the *relocate* operation returns `false`. Otherwise, it signifies the discovery of a valid replacement path.

Moving on to the entry replacement phase, adjacent entries in the path are swapped in reverse order using the `help_relocate` function. Eventually, the empty entry is shifted iteratively to the destination location (i.e., the first entry in the path), akin to the operation of bubble sort. However, entries along the replacement path may have been altered by other concurrent threads. Consequently, at the conclusion of this phase, a check is performed to ascertain whether the destination entry is empty. If it is, the *relocate* operation succeeds and returns. Otherwise, it returns to the path discovery phase to initiate a new path search.

In addition to the described relocation algorithm, we conceived a depth-first search (DFS) based replacement algorithm; however, due to time constraints, it is not impelmented yet. Detailed discussion of this alternative approach will be presented in section 4.


#### 2.5 Hazard Pointer Based Memory Management

**Hazard Pointer List and Retired Pointer List**
our lock-free implementation relies on two global data structures to ensure memory deletion correctness. The first, Retired Pointers, records pointers that have been retired (i.e., no longer present in the hash table) but have not yet been freed. The second, Hazard Pointers, preserves pointers currently accessed by at least one thread. To mitigate contention between threads, both structures are designed as per-thread lists. This approach ensures that each thread exclusively writes to its own retired and hazard pointer lists, eliminating contention among different writers. This design is inspired by [4].

**Retired Pointer Deletion**
Leveraging the Retired Pointers and Hazard Pointers data structures, we can safely manage the deletion of retired memories (i.e., fingerprints) in a lock-free environment. During operations such as *find*, *insert*, *remove*, and *relocate*, whenever an entry is accessed, it is added to the current thread's hazard pointer list, safeguarding it from being prematurely freed by other concurrent threads.

In the *remove* operation, when an entry is removed from the hash table, the fingerprint it points to is not immediately freed. Instead, its address is recorded in the current thread's retired pointer list. Upon accumulating a sufficient number of retired pointers (reaching a predefined threshold), a thread initiates the process of cleaning its retired pointers that are no longer in use. Specifically, retired pointers not present in any thread's hazard pointer list are deleted, while those still present in hazard pointer lists remain in the retired list.

### 3. Experimental Results

This section outlines the experiments conducted to assess the performance of the lock-free cuckoo filter. Unless explicitly stated otherwise, all experiments were carried out on GHC machines.

#### 3.1 Performance Comparsion between Three Cuckoo Filters on GHC Machines
In this experiment, we systematically compared the performance of three variants of cuckoo filters: coarse-grained lock, fine-grained lock, and lock-free implementations. The evaluation was conducted under a semi-realistic scenario, simulating a workload comprising 90% find operations, 5% insert operations, and 5% remove operations, with a load factor of 0.4. The results are presented in the figure below.

<img src="/Users/xsxsz/Desktop/15618/project/pic/ghc_compare.png" style="zoom:50%" />

As can be seen from the above figure, our lock-free cuckoo consistently outperforms the coarse-grained lock version, while its performance is comparble with fine-grained lock one. In addition, when the number of threads is smaller than or equal to physical core numbers (in this case 8), the lock-free filter's throughput scales approximately linearly with number of threads. The reason why the lock-free filter sometimes has lower throughput than the fine-grained lock version might be implementation specific (we haven't profiled it yet): In our fine-grained implementation, fingerprints are stored inside the hash table, while in the lock-free version, fingerprints are stored in dynamically allocated memory, therefore the overhead of allocating and deallocating fingerprints may incur significant runtime overhead. 

#### 3.2 Performance Comparsion between Three Cuckoo Filters on PSC Machines
To further test our implementation's scalability on multi-core platforms, we measured our filters' performance on the PSC machine using the same experimental setting, the results are shown below.

<img src="/Users/xsxsz/Desktop/15618/project/pic/psc_compare.png" style="zoom:50%" />

As can be observed from the figure, the lock-free cuckoo filter and the fine-grained lock version have similar performance on the PSC machine, and both of their performance slightly drops as the thread count exceeds 32. For the fine-grained lock version, this is because the granularity of locks used is still coarse to support high councurrency in high thread count environments. For the lock-free version, 


#### 3.3 Impact of Cuckoo Hash Table Associativity

To study the performance impact of hash table associativity (i.e., how many entries a bucket has) on our lock-free filter, we adjusted it to different values while keeping the table's capacity constant, and tested these different versions of lock-free filter's throughput using the semi-realisitic access pattern and 8 threads. The results are shown in the table below.

|   Associativity   |   Throughput (ops/sec)   |
| ---- | ---- |
|   2   |  2238496    |
|   4   |   2189643   |
|   8   |   2116626   |
|    16  |  1945880    |
|   32   |   1747437   |
|  64    |   1385703   |
|   128   |  917151    |

It can be observed that the filter's throughput gradually drops as the degree of associativity increases. An explanation for this result is that as the degree of associativity increases, *insert*, *remove* and *find* operations need to iterate through more entries per function call, which potentially slows down these operations' execution. 

#### 3.3 Impact of Load Factor

We further investigate the impact of load factor on three cuckoo filters, as shown below. This experiments uses the same  setting used in above experiments and the thread count is 8. 

<img src="/Users/xsxsz/Desktop/15618/project/pic/ghc_load_factor.png" style="zoom:50%" />

As can be seen from the above figure, as the load factor increases, the cuckoo filters' performance drops slightly. This is because as the load factor increases, the *insert* operation need to search more entries for an empty one, and the expensive *relocation* operation will also be called more frequently, resulting in lower throughput. Note that when the load factor is larger than or equal to 0.5, cuckoo filter's performance will drop significantly. Since this is an inherent feature of cuckoo filter rather than our lock-free implementation, we did not address this issue in our project.


### 4. Future Work

In our lock-free implementation, we put restrictions on entry relocatoin that the ith entry in a bucket can only be relocated to the ith entry in other buckets. This design reduces the complexity of false miss prevention, since with this rule false miss can be prevented by only checking only two entries' counter value. However, this restriction also potentially slows down the *relocate* operation. For example, if this restriction is lifted, we could perform Breadth First Search on the hash table,  which, in general, should be able to find an empty slot in fewer steps. 

In addition, when iterating through a bucket's entries, SIMD instruction can be used to speed up this process.  For instance, the *insert* operation iterate through all entries in a bucket to try to find an empty one. This operation can be optimizated using a 256-width SIMD operation that reads all four entries and check their values simultaneously, which could potentially speed up our implementation.

### 5. Conclusion & Acknowledgement

In this project, we implemented an efficient lock-free cuckoo filter. It has significant performance improvemnt in highly concurrent environments compared with its coarse-grained lock  counterpart,  while having comparable performance with the fine-grained lock version. Experiments have shown that our filter's throughput can scale approximately linearly with number of threads when the thread count does not exceed physical core number on GHC mahines. Although this project is overall complete, it is a pity that many possible optimizations, including those discussed in section 4, are not implemented due to time constraints. We could make this project better if we are not being chased by so many final exams :).

Our work is heavily inspired by paper [1] and [2], which proposes the cuckoo filter and a lock-free algorithm for cuckoo hashing, respectively. We also used some codes and libraries like the OpenSSL library and the Cycletimer provided by 15418's course resources.

### 6. Work Distribution

Yifei Wang and Yuchen Wang contribute 60% and 40% to this project respectively.

| Task | Yifei Wang |   Yuchen Wang   |
| ---- | :----------: | :----: |
| Implement coarse-grained lock version | ✔ |  |
| Implement fine-grained lock version |  | ✔ |
| Implement lock-free version | ✔ |  |
| Debugging and Profiling | ✔ | ✔ |
| Project Proposal | ✔ |  |
| Milestone Report | ✔ |      |
| Final Report | ✔ | ✔ |
| Project Poster | ✔ | ✔ |


### References

[1] N. Nguyen and P. Tsigas, "Lock-Free Cuckoo Hashing," 2014 IEEE 34th International Conference on Distributed Computing Systems, Madrid, Spain, 2014, pp. 627-636, doi: 10.1109/ICDCS.2014.70.

[2] Bin Fan, Dave G. Andersen, Michael Kaminsky, and Michael D. Mitzenmacher. 2014. Cuckoo Filter: Practically Better Than Bloom. In Proceedings of the 10th ACM International on Conference on emerging Networking Experiments and Technologies (CoNEXT '14). Association for Computing Machinery, New York, NY, USA, 75–88.

[3] Bin Fan, David G. Andersen, and Michael Kaminsky. 2013. MemC3: compact and concurrent MemCache with dumber caching and smarter hashing. In Proceedings of the 10th USENIX conference on Networked Systems Design and Implementation (nsdi'13). USENIX Association, USA, 371–384.

[4] M. M. Michael, "Hazard pointers: safe memory reclamation for lock-free objects," in *IEEE Transactions on Parallel and Distributed Systems*, vol. 15, no. 6, pp. 491-504, June 2004, doi: 10.1109/TPDS.2004.8.

