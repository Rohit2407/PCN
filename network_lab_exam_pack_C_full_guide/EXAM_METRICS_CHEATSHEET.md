# Network Lab Metrics Cheat Sheet

| Metric | Formula / meaning |
|---|---|
| Throughput | Received bits / elapsed seconds |
| Goodput | Successfully delivered application payload bits / elapsed seconds |
| RTT | Time from request send to reply arrival |
| Average end-to-end delay | Mean(receive timestamp − send timestamp) |
| PDR | Received packets / sent packets × 100% |
| Packet loss | Sent packets − received packets |
| Retransmissions | Extra transmissions after a timeout / loss |
| DNS latency | Time taken by system name resolution |
| HTTP download rate | Downloaded bits / download time |
| CRC | Polynomial division; zero remainder means no error detected |
| Checksum | One's-complement sum used for error detection |
| ARP | Maps local IPv4 address to link-layer/MAC address on a LAN |

## Experiment parameter table
| Parameter | Value from question |
|---|---|
| Nodes / topology | |
| Link bandwidth / Wi-Fi PHY rate | |
| Link delay | |
| Queue type and capacity | |
| Packet size | |
| Offered traffic rate | |
| Transport protocol / TCP variant | |
| Simulation duration | |

## Comparison table
| Run | Tx | Rx | Loss | PDR (%) | Throughput (Mbps) | Avg delay (ms) | RTT (ms) |
|---|---:|---:|---:|---:|---:|---:|---:|
| TCP | | | | | | | |
| UDP | | | | | | | |
| TCP variant 1 | | | | | | | |
| TCP variant 2 | | | | | | | |
| Wireless | | | | | | | |

## Exam interpretation
- TCP: reliable, ordered byte stream; OS handles retransmissions and congestion control.
- UDP: datagrams, no built-in delivery guarantee; an application can add sequence numbers, timers and retransmission.
- More delay generally increases RTT and can slow TCP feedback.
- A small queue can reduce queueing delay but may drop packets during bursts.
- Wireless results depend on distance, interference, contention and MAC retries.
- CRC/checksum detect some errors; they do not correct errors or encrypt the data.
- Use the values printed by your run; do not invent performance numbers.
