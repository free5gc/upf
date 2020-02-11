# linux_kernel_gtp
GTP-5G kernel module & libgtp5gnl for communicating with GTP-5G from user space.

## Prerequisites
Run `./build.sh` before running `./run.sh`

For running scripts under `./scripts`
```bash
sudo apt install bridge-utils -y
```

## Development Guide
GTP-5G is a customized linux kernel gtp module:
- `src/gtp.h` is modified from official linux repository `v5.3`, supported compiling on kernel v4.15+
- `src/gtp.c` is modified from official linux repository `v5.3`, supported compiling on kernel v4.15+
