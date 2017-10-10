Combinatorial Optimization Algorithms 
===================

This block currently contains an implementation of state-of-the-art BBMCI (and other related variants), an exact algorithm for the **maximum clique problem (MCP)**, as well as several utilities necessary for MCP such as different vertex orderings by degree or greedy sequential independent set coloring to compute bounds.

BBMCI is described in [1-2] and several applications of BBMCS are reported in literature connected to the Maximum Common Subgraph problem, as in [3-4]. It is currently being actively developped and several improvements have been proposed such as [5]. The algorithm uses several improvements over previously known algorithms based on a bitstring encoding. This implementation uses the [BITSCAN](https://www.biicode.com/pablodev/bitscan) library for bitstring and [GRAPH](https://www.biicode.com/pablodev/graph) library for graphs. Both libraries are available in the Biicode repository.


The source code has been tested on **Windows** and **Linux** platforms from Biicode host ([see doc](http://docs.biicode.com/c++/gettingstarted.html#basics)).

The code has been reasonably optimized. Performance should therefore be similar to that described in the original papers. The developer is also the author of BBMCI.

A command line application with parameterized interface is available [here](https://www.biicode.com/pablodev/examples_clique).



Terms and conditions
-------------------------------

Please feel free to use this code. *Just remember it is a research prototype and may not work for you*.The only condition is that you cite references [1-2][5] in your work.

Finally your feedback is gladly welcome. If you run into any problems just send your specific question to Biicode forum or, if its a general issue, to *StackOverflow* (tags Biicode, C++).

Acknowledgements
-------------------------------

This work has also been partially funded by the Spanish Ministry MINECO with grant [DPI2010-21247-C02-01](http://intelligentcontrol.disam.etsii.upm.es/arabot/).

References
-------------------------------
[1] *[An exact bit-parallel algorithm for the maximum clique problem](http://dl.acm.org/citation.cfm?id=1860369%20)*.San Segundo, P.; Rodriguez-Losada, D.; Jimenez, A. Computers & Operations Research, 38(2), 2011, 571-581.

[2] *[An improved bit parallel exact maximum clique algorithm](http://link.springer.com/article/10.1007%2Fs11590-011-0431-y)*. San Segundo, P.; Rodriguez-Losada, D.; Jimenez, A. Computers & Operations Research, 38(2), 2011, 571-581.

[3] *[Robust Global Feature Based Data Association With a Sparse Bit Optimized Maximum Clique Algorithm](http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=6527958).* San Segundo, P.; Rodriguez-Losada, D. IEEE Transactions on Robotics, 29(5), 2013, 1332-1339.

[4] *[A novel clique formulation for the visual feature matching problem](http://link.springer.com/article/10.1007%2Fs10489-015-0646-1#page-1)*. San Segundo, P.; Artieda, J. Applied Intelligence (online), 2015

[5] *[Relaxed approximate coloring in exact maximum clique search](http://dl.acm.org/citation.cfm?id=2566230)*. San Segundo, P.; Tapia, C. Computers & Operations Research, 44, 2014, 185-192.


   