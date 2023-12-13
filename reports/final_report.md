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

Rather than directly storing fingerprints in the hash table, for each fingerprint we put it in dynamically allocated memory and store an argumented 64-bit pointer (will be explained in the following subsection) pointing to it in the corresponding table entry. The main reason for this modification is that atomic compare-and-swap operation cannot be performed on a C++ string instance, while GCC provides `__sync_bool_compare_and_swap` intrinsic to update a 64-bit variable, by which we could atomically modifiy table entries in a lock-free environment.

**Table Pointer**
The way we construct the table pointer (i.e., a normal x86_64 pointer augmented with additional information) is basically the same as in paper [2], its structure is shown in the following figure. 

<img src="/Users/xsxsz/Desktop/15618/project/pic/pointer.png" style="zoom:20%" />

The basic idea behind this data structure is utilizing unused fields in a 64-bits pointer to store additional information. The 48-bits real pointer fleid contains the lower-48 bits of the actual pointer that points to the fingerprint. Currently x86_64 platform only uses the lower-48bits of a 64-bits pointer to represent valid pointers and the remaining bits are fill with zero. Hence, we could  use the lower 48 bits to record a pointer that points to a fingerprint, and utilize the higher 16 bits to store the number of times this entry has been relocated, which is used to prevent false miss in Find operations. In addition, due to memory address alignment, the least significant bit of a x86_64 pointer is always zero, so we use this bit to indicate whether this pointer is currently being relocated. 

#### 2.3 Find, Insert and Remove Operations

**Find Operation**
Finding a key with same fingerprint in the hash table involves a series of queries of possible hash table entries. In our 4-way associative cuckoo hash table, the total number of entries that should be checked for each input key is 8. If a matching fingerprint is found, the *find* operation returns the entry's location.

However, the above method could miss existing fingerprints if they are replaced during the search, resulting in false miss. For example, a matching fingerprint is stored in its alternative bucket while the *find* operation is querying the primary bucket. When the *find* operation moves on to search the alternative bucket, the fingerprint has been relocated to its primary bucket. In this case, the *find* operation cannot find the fingerprint even if it exists in the hash table.

To address this issue, we uses a two-round query for finding fingerprints, as shown in the following pseudo-code.


In our implementation, each fingerprint can only be replaced to the alternative bucket's same entry (the reasoning behind this design will be explained in detail later). The first round query is the same as above, additionally we record their curent counter value. In the second round, we iterate through all entry pairs (i.e., two entries in the primary and alternative buckets' same location) and compare their current counter value with their previous value (function `check_counter`). If more than two relocations happended during the search, it is possible that the search missed an existing value, thus the *find* operation is restarted. Otherwise, we can assert that the key does not exist.

**Insert Operation**

The program below shows the pseudo code of the *insert* operation. 


Basically, we first calculate the locations of the primary and alternative bucket and store the fingerprint in dynamically allocated memory. Then we perform the following operations in an infinite loop: (1) try to find an empty entry among the 8 possible entries. (2) If an empty entry is found and the CAS operation to store the table pointer succeeds, the *insert operation* return with true. (3) If no empty entry is found, we call the *relocate* operation on all entries in the primary bucket until an empty entry is created. Specifically, we perform the relocation in random order: We generate an array [0, 1, 2, 3] and randomly shuffle it, then use it as the order for kicking out entries. This optimization distributes new inserted keys among all table entries, resulting in better load balance. (4) If at least one *relocate* operation succeeds, the *insert* is started again, otherwise, which means the hash table does not have enough capacity, the *insert* operation fails. Due to the fact that cuckoo filter does not keep original keys, rehashing cannot be performed in this situation. Since this is an issue of cuckoo filter rather than our lock-free implementation, we do not address it in our project.

**Remove Operation**
Deleting a fingerprint in the cuckoo hash table is trivial. First, we call the find operation to determine the location of the matching fingerprint. It is worth noting that it is the filter user's responsibility to ensure that the key to be deleted is stored in the hash table (i.e., the find operation will always return a valid location), otherwise the filter's behavior is undefined. After that, we attempt to remove the entry using atomic compare-and-swap. If the CAS operation succeed, we put the pointer in a retired pointer list (will be discussed in subsection 2.5) and return, otherwise the whole remove operation is executed again.

#### 2.4 Relocate Operation
The *relocate* operation will be called when the *insert* operation cannot find an empty entry. 


#### 2.5 Hazard Pointer Based Memory Management

**Hazard Pointer List and Retired Pointer List**
Since C++ does not come with a built-in garbage collector, additional data structures are required to maintain correctness during memory deletion. In our lock-free implementaion, we use two global data structures for this purpose. The first one is Retired_Pointers, which records pointers retired (i.e., no longer used and removed from the hash table) but not yet freed by each thread. The another one is Hazard_Pointers, which stores pointers that are currently being accessed by at least one thread. It is worth noting that in order to reduce contention between threads, both of them are per-thread list, meaning that each thread will only write its own retired and hazard pointer lists, thus eliminating the contention between different writers.

**Memory **

#### 2.6 Possible Optimizations
This subsection describes several optimizations or alternative approaches we have designed but not yet implemented due to time constraints.

### 3. Experimental Results

#### 3.1 Performance under Semi-realisitic Access Patterns

#### 3.2 Performance under all Insert & Remove Access Patterns


### 4. Future Work

### 5. Conclusion

### 6. Work Distribution

### References

[1] N. Nguyen and P. Tsigas, "Lock-Free Cuckoo Hashing," 2014 IEEE 34th International Conference on Distributed Computing Systems, Madrid, Spain, 2014, pp. 627-636, doi: 10.1109/ICDCS.2014.70.

[2] Bin Fan, Dave G. Andersen, Michael Kaminsky, and Michael D. Mitzenmacher. 2014. Cuckoo Filter: Practically Better Than Bloom. In Proceedings of the 10th ACM International on Conference on emerging Networking Experiments and Technologies (CoNEXT '14). Association for Computing Machinery, New York, NY, USA, 75–88.

[3] Bin Fan, David G. Andersen, and Michael Kaminsky. 2013. MemC3: compact and concurrent MemCache with dumber caching and smarter hashing. In Proceedings of the 10th USENIX conference on Networked Systems Design and Implementation (nsdi'13). USENIX Association, USA, 371–384.

[4] M. M. Michael, "Hazard pointers: safe memory reclamation for lock-free objects," in *IEEE Transactions on Parallel and Distributed Systems*, vol. 15, no. 6, pp. 491-504, June 2004, doi: 10.1109/TPDS.2004.8.

