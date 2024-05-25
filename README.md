# Network Traffic Monitoring & Simulation of Tree Topology Network

## Contributors
- @arezyhs https://github.com/arezyhs
- @shylniac https://github.com/shylniac
- @nofiayuwulandari

## Overview
This program simulates a network environment with a hierarchical tree topology, consisting of a router, switches, and PCs. It allows administrators and users to interact with the network, monitor traffic, manage devices, and simulate network attacks such as DDoS. The program is designed to run in a console environment with a menu-driven interface for both administrators and users.

## Features

### Administrator Features
- **Add/Remove PCs:** Connect or disconnect PCs to/from switches.
- **Change Configuration:** Modify IP addresses and gateways of PCs, switches, and the router.
- **Start/Stop Devices:** Control the power state of devices (PCs, switches, router).
- **Simulate DDoS Attacks:** Launch and mitigate DDoS attacks on network devices.
- **Monitor Network:** Display network topology, logs, registered IPs, bandwidth usage, and adjacency list.
- **Process Packets:** Administer the processing of network packets queued for delivery.

### User Features
- **Send/Receive Packets:** Transmit messages to other PCs and view received packets.
- **Ping Router:** Check connectivity and measure round-trip time to the router.
- **View Bandwidth Usage:** Display the bandwidth usage of the user's PC.
- **Block/Unblock IPs:** Manage blocked IP addresses to control access.
- **Start/Stop PC:** Turn the user's PC on or off.

## Network Topology

The network is structured in a tree topology with the following hierarchy:
- **Router:** Central device connecting multiple switches.
- **Switches:** Intermediate devices connecting multiple PCs.
- **PCs:** End devices connected to switches.

## Logging

The program logs all significant events and actions, including device status changes, packet transmissions, DDoS attacks, and user actions. Logs are stored both in memory and written to a file (`network_logs.txt`).

## Simulation Details

### Packet Handling
- Packets are enqueued for processing and can be accepted or rejected by the administrator.
- Bandwidth usage is tracked for both sent and received packets.

### DDoS Simulation
- DDoS attacks can be simulated by generating a large number of packets from multiple sources to a target IP.
- Mitigation involves shutting down the target device and the sources of the attack.

### Tree Topology Display
- The network topology is displayed in a hierarchical tree format, showing the router, switches, and connected PCs.
## Usage

### Administrator Mode:
- Login with username `admin` and password `admin`.
- Use the menu to configure the network, monitor traffic, and simulate attacks.

### User Mode:
- Login with the username and password assigned to the connected PC.
- Use the menu to send/receive packets, ping the router, and manage the PC.

## File Structure
- `networkTrafficMonitoringSimulation.cpp`: this one implement graph structure, a bit fragile. 
- `networkTrafficMonitoringSimulationTanpaGraf.cpp`: This one without implementation of graph structure, more stable. 
- `network_logs.txt`: Log file to store network events.
