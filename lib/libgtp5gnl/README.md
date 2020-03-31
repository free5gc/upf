# libgtp5gnl - netlink library for Linux kernel module 5G GTP-U

In order to control the kernel-side GTP-U plane, a netlink based control
interface between GTP-C in userspace and GTP-U in kernelspace was invented.

The encoding and decoding of these control messages is implemented in
the libgtp5gnl (library for GTP netlink).

libgtp5gnl is a project based on [libgtpnl](https://github.com/osmocom/libgtpnl)
which is part of the [Osmocom](https://osmocom.org/) Open Source Mobile
Communications project.
