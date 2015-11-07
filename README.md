MPCP Pause
==========

The office network would occasionally have periods when latency across the
LAN would degrade hideously (several seconds), and then all connectivity
would be lost.

The cause of the failure turned out to be computers that hung during
shutdown, and the network card would fire out a barrage of MPCP pause
frames.

```
17:35:31.148443 93:e6:d5:cb:ba:fe > 01:80:c2:00:00:01, ethertype MPCP (0x8808), length 60: MPCP, Opcode Pause, length 46
        0x0000:  0001 0650 0000 0000 0000 0000 0000 0000
        0x0010:  0000 0000 0000 0000 0000 0000 0000 0000
        0x0020:  0000 0000 0000 0000 0000 0000 0000
```

This should not be a widespread problem, as the destination address,
01:80:c2:00:00:01, will not be forwarded by an 802.1D-compliant
switch<sup>[1](#wikipedia)</sup>.  Unfortunately, these packets are
forwarded by the CISCO switches<sup>[2](#cisco)</sup> we are using.

Every device on the LAN receives the packets, and obligingly suspends
transmission.  One device would even remain inactive even after the source
of MPCP pause frames had stopped.

This tool can be used to identify whether your network is susceptible to
the same problem.

#### Footnotes
1. Wikipedia - [Ethernet flow control](https://en.wikipedia.org/wiki/Ethernet_flow_control#Pause_frame)
2. CISCO - [Small Business 100 Series 24-port 10/100 Unmanaged Switch](http://www.cisco.com/c/en/us/support/switches/sg100-24-24-port-gigabit-switch/model.html)
