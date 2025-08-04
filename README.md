# DNS proxy filter dns-proxy-filter-p2B9agE1 for POSIX-compliant environments
A simple DNS proxy server for filtering DNS requests. The configuration file follows this format:
```ini
  [server]
  listen_address = 127.0.0.1
  listen_port = 5300
                    
  [upstream]
  dns1 = 1.1.1.1
  dns2 = 8.8.8.8
  dns3 = 8.8.4.4
                    
  [blacklist]
  yandex.ru = notfind
  ya.ru = refuse
  tutu.ru = 178.248.234.61
```

# Installation

This project uses C++20 modules and C++23 features. It therefore depends on CMake and Ninja (to handle dependencies).
To install Ninja: 
```bash
brew install ninja
```
or (for example, on Debian/Ubuntu Linux):
```bash
sudo apt update
sudo apt install ninja-build
```
Because of the modules and dependency management, this project also requires a recent version of Clang that supports automatic module dependency handling. On macOS and many Linux distributions, you may need to install or update Clang.

Assuming LLVM Clang is installed at `/opt/homebrew/opt/llvm/bin/`, navigate in the terminal to the folder where you want to build the app, then run the following commands (adjust the path `/opt/homebrew/opt/llvm/bin/` as needed):
```bash
git clone --branch main https://github.com/yuryatin/dns-proxy-filter-p2B9agE1.git
cd dns-proxy-filter-p2B9agE1
mkdir build
cd build
cmake -G Ninja -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++  -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_SCAN_FOR_MODULES=ON ..
ninja
```

The assembled executable will be named `dns_proxy_filter_p2B9agE1` and can be found in the `build/` folder.

# Current limitation on Linux

Since Linux (unlike macOS) usually does not come with pre-built Unix headers, additional steps are required to build them as C++20 modules for this C++23 app to compile.

# How to use

After navigating to the folder containing the executable, type its name `./dns_proxy_filter_p2B9agE1` and provide the path to the configuration file. For example:
```bash
./dns_proxy_filter_p2B9agE1 ~/Documents/.config/p2B9agE1.conf
```

Alternatively, you may want to add its directory to your PATH to be able to launch it from anywhere in the terminal.
```bash
export PATH=$PATH:/folder/you/want
```

After the path to the configuration file, you may also specify the number of threads to use.
```bash
./dns_proxy_filter_p2B9agE1 ~/Documents/.config/p2B9agE1.conf 16
```

# How it works

This DNS proxy filter is configurable with four options for any domain (in order of decreasing precedence):
* To respond with 'not found' for DNS records. This applies to all standard DNS queries defined in RFC 1035, Section 3.2.2, (https://datatracker.ietf.org/doc/html/rfc1035#section-3.2.2) such as A, NS, MD, MF, CNAME, SOA, MB, MG, MR, NULL, WKS, PTR, HINFO, MINFO, MX, TXT, as well as additional types like AAAA, PTR, NAPTR, SRV.
* To refuse service for the same set of DNS queries.
* To return a preconfigured IPv4 address (applies to A queries only).
* To return a preconfigured IPv6 address (applies to AAAA queries only).

# How to test this DNS proxy filter

With the Python package dnstester-qboxxbyh (https://github.com/yuryatin/dnstester-qboxxbyh), which generates configuration files in the format required by this DNS proxy filter, you can easily create test configurations. This package can automatically launch the DNS proxy filter immediately after creating a configuration file, allowing the proxy to start with its configuration loaded. After that, the Python tester dynamically displays in the terminal how the proxy successfully handles four random subsamples of pre-verified domain names with different treatments.
This package is available for standard installation via ```pip``` from the PyPI repository.
It displays test results dynamically in the terminal:

<img width="579" height="776" alt="updated_test_results" src="https://github.com/user-attachments/assets/25016c5a-9f60-452c-a367-f22991c22c11" />

# Constraints

This version does not yet support:
* non-POSIX-compliant environments,
* configuration to selectively “not find” or refuse service for specific DNS query types (currently, it affects all standard DNS query types),
* configuration that responds to restricted or preconfigured domains with authoritative responses,
* handling DNS request packets with multiple queries (only the first query is processed). Consequently, if the client sends a DNS request where a restricted domain appears in the second or later position, the request will be forwarded to an upstream DNS server. If the upstream server is able to fully honor this request, the client will receive that response as if no DNS proxy filter were in place.
