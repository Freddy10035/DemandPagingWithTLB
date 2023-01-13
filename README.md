<h1>Getting Started</h1>

Clone the repository to your local machine
Navigate to the project directory and build the simulation by running the command make

Usage:

    ./main [inputfile] [page table size] [TLB size] [page replacement algorithm]


where:
- inputfile is the name of the input file containing the virtual memory address traces
- page table size is the number of pages that can be stored in the page table (in pages)
- TLB size is the number of entries in the TLB (in pages)
- page replacement algorithm is the algorithm to use for page replacement. Possible options are FIFO, LRU, OPT.
- The output of the simulation will include statistics on the number of page faults, the TLB hit rate, and the page replacement algorithm used
- Find the sample input files in the Input folder

<h1>Dependencies</h1>

g++ (or any c++ compiler)

<h2>Input format</h2>

The input file should contain a list of virtual memory addresses, one per line. Each address should be a decimal number.

<h2>Page Replacement Algorithms</h2>

`FIFO`: First-In, First-Out algorithm </br>
`LRU`: Least Recently Used algorithm </br>
`OPT`: Optimal Page Replacement algorithm
