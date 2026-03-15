Outputs in this directory are generated using the following commands:

Paging with no caching and default parameters for 3 inputs in the `inputs` directory (i varies from 1 to 3): 

`./paging.py inputs/input${i}.txt > outputs/output${i}_paging.txt`

Paging with TLB caching, default page size (128 bytes), and FIFO replacement policy:

`./paging_tlb_fifo.py inputs/input${i}.txt > outputs/output${i}_paging_tlb_fifo.txt`


Paging with TLB caching, default page size (128 bytes), and either LRU or MIN replacement policy:

```
./paging_tlb -s LRU inputs/input${i}.txt > outputs/output${i}_paging_tlb_LRU.txt

./paging_tlb.py -s MIN inputs/input${i}.txt > outputs/output${i}_paging_tlb_MIN.txt
```