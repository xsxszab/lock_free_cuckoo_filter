# Lock-free Cuckoo Filter

### Team members

**Yifei Wang** (yifeiw3), **Yuchen Wang** (yw7)

### URL

[https://xsxszab.github.io/](https://xsxszab.github.io/)

### Summary
In this project, we are going to implement a lock-free version of the Cuckoo Filter introduced in [2]. The lock-free filter will be benchmarked against its coarse-grained and fine-grained locked counterparts for performance evaluation.

### Background

The Approximate Membership Query Filter (AMQ-Filter) is a probabilistic data structure designed to determine whether a specific item belongs to a set with a certain false positive rate. The widely adopted Bloom Filter, employed in systems such as caches, routers, and databases, faces a drawback: it lacks efficiency in removing items from the set.

To overcome this limitation, [2] introduced the Cuckoo Filter, a novel AMQ-Filter leveraging the Cuckoo Hash Table. This alternative not only addresses the inefficiency in item deletion but also demonstrates superior efficiency in queries compared to the Bloom Filter.

Given the prevalent use of AMQ-Filters in high-concurrency environments, our project aims to implement a fully concurrent version of the Cuckoo Filter capable of supporting multi-threaded access. Our approach involves leveraging lock-free programming to achieve a substantial performance boost when compared to both coarse-grained and fine-grained locked versions.

### The Challenge

The most challenging part of our project is writing a correct lock-free version of the filter. Specifically, the displacement operation during item insertion is the most difficult part to handle. We need to design a correct and efficient way to manage items displaced but not yet reinserted into the hash table. In other words, as an item awaits reinsertion into the hash table, filter users (query threads) should be able to ascertain its existence.

### Resources
* **Computing Resources**: We plan to use our laptops and GHC machines for this project.
* **Code Base**: We will start from scratch to implement the cuckoo filter in C++, without using any starter code. In addition, we found a 15618 project in Spring 2017 that implements lock-free cuckoo hashing, which bears some similarity to what we plan to implement. We will not use any part of their code for our project.
* **Reference materials**: Please refer to the References Section below (this may not be a comprehensive list, we may refer to new books or papers during the project).

### Goals and Deliverables

**Plan to Achieve**:

* Implement a functional, correct Lock-free Cuckoo Filter. This goal must be achieved since the filter is the core part of our project.
* Implement both corse-grained and fine-grained locked version of the cuckoo filter, and compare their performance with our lock-free version under different access patterns.

**Hope to Achieve**:

* Implement the semi-sorting technique mentioned in [2] for space optimization. Since this part will make the filter's code structure significantly more complex, we will only try to implement it after the basic version of cuckoo filter is finished.
* Accelerate the filter through utilizing SIMD instructions.
* Provide a Python wrapper for the cuckoo filter using pybind11.

### Platform Choice

We will implement the cuckoo filter in C++ on Linux platforms (our laptop & GHC machines). We are both familiar with C++ programming so using it would likely result in high productivity.

### Schedule

| Date |  TASKS    |
| ---- | ---- |
| 11/16 - 11/22 | Paper Reading & Implement coarse-grained locked version of the Cuckoo Filter |
| 11/23 - 11/29     | Implement fine-grained locked Cuckoo Filter & start to implement the lock-free version |
| 11/30 - 12/7 | Finish lock-free version, write milestone report |
| 12/8- 12/14 | Debugging and optimization, write final report, create posters |
|   12/15   |   Poster Session   |




### References
[1] N. Nguyen and P. Tsigas, "Lock-Free Cuckoo Hashing," 2014 IEEE 34th International Conference on Distributed Computing Systems, Madrid, Spain, 2014, pp. 627-636.
[2] Fan, Bin, et al. "Cuckoo filter: Practically better than bloom." Proceedings of the 10th ACM International on Conference on emerging Networking Experiments and Technologies. 2014.

