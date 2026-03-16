#!/bin/bash
# PA3 Section 4: Write and Read Performance
# Run from factory_src. Uses localhost; for Khoury cluster use actual IPs.
# Spec: no messages during execution (client prints stats at end only).

set -e
cd "$(dirname "$0")"
CLIENT="./client"
SERVER="./server"
HOST="${HOST:-127.0.0.1}"

# Ports for 1/2/3 server setups (avoid overlap)
P0=12501
P1=12502
P2=12503

# --- Write performance (Section 4) ---
# "Change the number of server programs from 1 to 3. Use 8 customer threads.
#  Run one client program for 10 seconds or longer ... measure mean latency (us) and throughput (orders/s)."
# 8 customers, enough orders to run 10+ sec (~1500 orders/sec -> 15000+ total -> 2000 per customer)
WRITE_CUSTOMERS=8
WRITE_ORDERS_PER_CUSTOMER=2000

run_write_1_server() {
  echo "========== Write performance: 1 server =========="
  $SERVER $P0 0 0 &
  S0=$!
  sleep 2
  $CLIENT $HOST $P0 $WRITE_CUSTOMERS $WRITE_ORDERS_PER_CUSTOMER 1
  kill $S0 2>/dev/null || true
  wait $S0 2>/dev/null || true
  sleep 1
}

run_write_2_servers() {
  echo "========== Write performance: 2 servers =========="
  $SERVER $P0 0 1 1 $HOST $P1 &
  S0=$!
  $SERVER $P1 1 1 0 $HOST $P0 &
  S1=$!
  sleep 2
  $CLIENT $HOST $P0 $WRITE_CUSTOMERS $WRITE_ORDERS_PER_CUSTOMER 1
  kill $S0 $S1 2>/dev/null || true
  wait $S0 $S1 2>/dev/null || true
  sleep 1
}

run_write_3_servers() {
  echo "========== Write performance: 3 servers =========="
  $SERVER $P0 0 2 1 $HOST $P1 2 $HOST $P2 &
  S0=$!
  $SERVER $P1 1 2 0 $HOST $P0 2 $HOST $P2 &
  S1=$!
  $SERVER $P2 2 2 0 $HOST $P0 1 $HOST $P1 &
  S2=$!
  sleep 2
  $CLIENT $HOST $P0 $WRITE_CUSTOMERS $WRITE_ORDERS_PER_CUSTOMER 1
  kill $S0 $S1 $S2 2>/dev/null || true
  wait $S0 $S1 $S2 2>/dev/null || true
  sleep 1
}

# --- Read performance (Section 4) ---
# "Use one client to fill: 128 customers, 4 requests each. Then for each server program,
#  run a client with 128 threads issuing read requests (type 2) for over 10 seconds.
#  Change from 1 server+1 client to 2 servers+2 clients, run concurrently, sum throughput."
# Fill: 128 customers, 4 orders each (type 1)
# Read: 128 threads, enough reads for 10+ sec (~500 reads/s -> 5000+ total -> 50 per thread)
READ_FILL_CUSTOMERS=128
READ_FILL_ORDERS=4
READ_CUSTOMERS=128
READ_REQUESTS_PER_THREAD=100

run_read_fill() {
  # Populate servers with customer data (run against primary)
  echo "========== Read performance: filling data (128 customers x 4 orders) =========="
  $CLIENT $HOST $1 $READ_FILL_CUSTOMERS $READ_FILL_ORDERS 1
}

run_read_1_server() {
  echo "========== Read performance: 1 server, 1 client (128 threads, read ~10+ sec) =========="
  $SERVER $P0 0 0 &
  S0=$!
  sleep 2
  run_read_fill $P0
  $CLIENT $HOST $P0 $READ_CUSTOMERS $READ_REQUESTS_PER_THREAD 2
  kill $S0 2>/dev/null || true
  wait $S0 2>/dev/null || true
  sleep 1
}

run_read_2_servers() {
  echo "========== Read performance: 2 servers, 2 clients concurrent (sum throughput) =========="
  $SERVER $P0 0 1 1 $HOST $P1 &
  S0=$!
  $SERVER $P1 1 1 0 $HOST $P0 &
  S1=$!
  sleep 2
  # Fill via primary
  run_read_fill $P0
  sleep 1
  # Run two read clients concurrently, each to one server; sum their throughput manually from output
  $CLIENT $HOST $P0 $READ_CUSTOMERS $READ_REQUESTS_PER_THREAD 2 > /tmp/pa3_read_s0.txt &
  C0=$!
  $CLIENT $HOST $P1 $READ_CUSTOMERS $READ_REQUESTS_PER_THREAD 2 > /tmp/pa3_read_s1.txt &
  C1=$!
  wait $C0 $C1
  echo "--- Server 0 ---"
  cat /tmp/pa3_read_s0.txt
  echo "--- Server 1 ---"
  cat /tmp/pa3_read_s1.txt
  echo "Sum throughput = (add Throughput from both outputs above)"
  kill $S0 $S1 2>/dev/null || true
  wait $S0 $S1 2>/dev/null || true
  sleep 1
}

# --- Main: run what you need ---
usage() {
  echo "Usage: $0 [write1|write2|write3|read1|read2|write|read|all]"
  echo "  write1/write2/write3  - write perf with 1/2/3 servers"
  echo "  read1/read2           - read perf with 1 server or 2 servers+2 clients"
  echo "  write                 - write perf 1,2,3 in sequence"
  echo "  read                  - read perf 1 then 2"
  echo "  all                   - write then read (all experiments)"
}

case "${1:-all}" in
  write1) run_write_1_server ;;
  write2) run_write_2_servers ;;
  write3) run_write_3_servers ;;
  write)  run_write_1_server; run_write_2_servers; run_write_3_servers ;;
  read1) run_read_1_server ;;
  read2) run_read_2_servers ;;
  read)  run_read_1_server; run_read_2_servers ;;
  all)   run_write_1_server; run_write_2_servers; run_write_3_servers
        run_read_1_server; run_read_2_servers ;;
  *)     usage; exit 1 ;;
esac
