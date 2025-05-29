# DNS Resolver Implementation

## Overview
This project implements a DNS resolution system that supports both iterative and recursive lookups using Python and the `dnspython` library. The program demonstrates the fundamental process of DNS resolution by querying nameservers to translate domain names (like google.com) into IP addresses.

## Features
- **Iterative DNS Resolution**: Manually follows the DNS hierarchy by querying:
  1. Root servers
  2. Top-level domain (TLD) servers
  3. Authoritative servers
  
- **Recursive DNS Resolution**: Delegates the entire resolution process to the system's default resolver

- **Error Handling**: Manages timeouts, unreachable servers, and invalid domains


## Installation

1. Clone the repository or download the source code
2. Install the required dependencies

## Usage

The program accepts two command-line arguments:
1. Resolution mode (`iterative` or `recursive`)
2. Domain name to resolve

### Examples:

# Iterative DNS lookup for google.com
python dnsresolver.py iterative google.com

# Recursive DNS lookup for example.com
python dnsresolver.py recursive example.com

## Implementation Details

### 1. Iterative DNS Resolution
The iterative resolution process starts from the root servers and progresses through the DNS hierarchy:

- **Root Servers**: Query predefined root servers to get TLD server information
- **TLD Servers**: Query TLD servers (like .com, .org, .net) to get authoritative nameserver information
- **Authoritative Servers**: Query authoritative nameservers to get the final IP address

This approach follows the actual DNS architecture and shows each step of the resolution process.

### 2. Recursive DNS Resolution
The recursive resolution uses the system's default resolver, which handles the entire resolution process internally:

- Delegates the resolution to external resolvers (e.g., ISP's DNS servers)
- Returns the final result directly

### 3. Key Functions

- `send_dns_query(server, domain)`: Sends a UDP DNS query to a specified nameserver
- `extract_next_nameservers(response)`: Extracts and resolves nameserver records from response
- `iterative_dns_lookup(domain)`: Performs the iterative DNS resolution process
- `recursive_dns_lookup(domain)`: Performs the recursive DNS resolution process

## Error Handling
The implementation includes error handling for various scenarios:
- Timeout when querying a nameserver
- Unable to resolve nameserver hostnames
- Non-existent domains
- Network connectivity issues

## Performance Comparison
As shown in the example output, recursive resolution is typically faster than iterative resolution because:
1. The recursive resolver often has cached results
2. The recursive resolver may have optimizations for querying the DNS hierarchy
3. The iterative approach requires multiple sequential queries

## Implementation Notes
- The iterative approach provides educational value by exposing the DNS resolution process
- The recursive approach is more practical for real-world applications due to its efficiency
- The root server list is a subset of the actual 13 root server IPs for demonstration purposes

## Troubleshooting
If you encounter issues:
1. Ensure `dnspython` is installed correctly
2. Check network connectivity
3. Verify that DNS port 53 is not blocked by firewalls
4. For iterative resolution, ensure the root servers are reachable
