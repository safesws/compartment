Here is an attempt to understand Internals of the __Windows Containers Neworking Isolation__

[um_structures](https://github.com/safesws/compartment/blob/main/um_structures) - user mode structures and functions for the __compartments__
[ndis_structures](https://github.com/safesws/compartment/blob/main/ndis_structures) - kernel mode structures and functions for the parts involed to the __compartments__ implementation
[compartment.cpp](https://github.com/safesws/compartment/blob/main/compartment.cpp) - user mode application for the __compartments__ manipulations
[hcn.cpp](https://github.com/safesws/compartment/blob/main/hcn.cpp) - user mode application for the __Host Compute Network__ manipulations
[sws_ndis.h](https://github.com/safesws/compartment/blob/main/sws_ndis.h) - kernel mode code for _playing_ with __compartments__ internals (like set _network isolation_ for an interface)

All information described in the details here: [https://safesws.github.io/windows-containers-network-isolation/](https://safesws.github.io/windows-containers-network-isolation/)
