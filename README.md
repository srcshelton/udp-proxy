udp-proxy
=========

I was in need of a way to forward syslog packets on port 514 between networks,
and couldn't find an easy way to achieve this - iptables appears to treat all
UDP packets on the same port as a single connection for tracking purposes, and
so didn't appropriately mangle the packets.

The original source from http://www.brokestream.com/udp_redirect.html has been
modified to support sending data to the destination IP address from a different
source-ip than the listen-ip, allowing proxying on multi-homed hosts.

