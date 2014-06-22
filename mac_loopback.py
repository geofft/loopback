#!/usr/bin/env python

from contextlib import contextmanager
import re
import subprocess

import SystemConfiguration as sc
from twisted.internet import defer, reactor
from twisted.names import dns, error, server

DOMAIN = 'lo.ldpreload.com'

class Resolver(object):
    REGEX = re.compile('(\d+)\.' + re.escape(DOMAIN))

    def query(self, query, timeout=None):
        name = query.name.name
        match = self.REGEX.search(name)
        if match and int(match.group(1)) < 255:
            payload = dns.Record_A(address='127.1.1.' + match.group(1))
            response = dns.RRHeader(name=name, payload=payload)
            return defer.succeed(([response], [], []))
        return defer.fail(error.DomainError())

@contextmanager
def loopback_addresses(ip_template):
    # Mac OS X (and the BSDs in general, it seems) doesn't let you bind
    # to 127.x.y.z by default, unless the IP is added to the interface.
    for i in range(256):
        subprocess.check_call(['ifconfig', 'lo0',
                               'alias', ip_template.format(i)])
    yield
    for i in range(256):
        subprocess.check_call(['ifconfig', 'lo0',
                               '-alias', ip_template.format(i)])

@contextmanager
def sc_dns(address):
    client_name = '.'.join(reversed(DOMAIN.split('.')))
    store = sc.SCDynamicStoreCreate(None, client_name, None, None)
    sc.SCDynamicStoreAddTemporaryValue(store,
            "State:/Network/Service/0028FE6C-9FA4-4C07-9BA9-F052480E3148/DNS",
            {"ServerAddresses": [address],
             "SupplementalMatchDomains": [DOMAIN]})
    yield sc

def main():
    address = '127.1.1.255'
    with loopback_addresses('127.1.1.{0}'):
        factory = server.DNSServerFactory(authorities=[Resolver()])
        protocol = dns.DNSDatagramProtocol(controller=factory)
        reactor.listenTCP(53, factory, interface=address)
        reactor.listenUDP(53, protocol, interface=address)

        with sc_dns(address):
            sys.stderr.write("Loopback DNS for {0} starting\n".format(DOMAIN))
            reactor.run()

if __name__ == '__main__':
    main()
