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

After navigating in the terminal to a folder where you’d like to build the app, you can assemble it on macOS or Linux using the following commands:
```
git clone https://github.com/yuryatin/dns-proxy-filter-p2B9agE1.git
cd dns-proxy-filter-p2B9agE1
make
```
The assembled executable will be named ```dns_proxy_filter_p2B9agE1```.

# How to use

After navigating to the folder containing the executable, type its name ```./dns_proxy_filter_p2B9agE1``` and provide the path to the configuration file. For example:
```
./dns_proxy_filter_p2B9agE1 ~/Documents/.config/p2B9agE1.conf
```
Alternatively, you may want to add its directory to your PATH to be able to launch it from anywhere in the terminal.
```
export PATH=$PATH:/folder/you/want
```
After the path to the configuration file, you may also specify the number of threads to use.
```
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

![Example screenshot](https://raw.githubusercontent.com/yuryatin/dnstester-qboxxbyh/main/pics/updated_test_results.png)

# Constraints

This version does not yet support:
* non-POSIX-compliant environments,
* configuration to selectively “not find” or refuse service for specific DNS query types (currently, it affects all standard DNS query types),
* configuration that responds to restricted or preconfigured domains with authoritative responses,
* handling DNS request packets with multiple queries (only the first query is processed). Consequently, if the client sends a DNS request where a restricted domain appears in the second or later position, the request will be forwarded to an upstream DNS server. If the upstream server is able to fully honor this request, the client will receive that response as if no DNS proxy filter were in place.
