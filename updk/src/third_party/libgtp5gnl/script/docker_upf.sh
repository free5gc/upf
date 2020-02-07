#!/bin/bash
source ./config

cd ${LIBGTP5GNL_TOOLS_PATH}

killall -9 gtp5g-link
./gtp5g-link add gtp5gtest &
sleep 0.2

./gtp5g-tunnel add far gtp5gtest 1 --action 2 --fwd-policy 10
./gtp5g-tunnel add far gtp5gtest 2 --action 2 --hdr-creation 0 87 ${RAN_IP} 2152
./gtp5g-tunnel add pdr gtp5gtest 1 --pcd 1 --hdr-rm 0 --ue-ipv4 ${UE_IP} --f-teid 78 ${UPF_IP} --far-id 1
./gtp5g-tunnel add pdr gtp5gtest 2 --pcd 2 --ue-ipv4 ${UE_IP} --far-id 2

ip r add ${UE_CIDR} dev gtp5gtest

while :; do sleep 1; done