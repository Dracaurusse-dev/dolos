# dolos
Simple proxy installed on an apache webserver to redirect from a port to another most of the time and sometimes on a troll port.

Change the config in /etc/dolos.conf

Enable the service with your service manager

the proxy's args are:
-I: server's ip (default to localhost) \
-i: redirect ip (default to localhost) \
-w: website port (default to 80) \
-r: redirect port (where the packets are redirected for the joke) (default to 8000) \
-p: proxy port (default to 8080) \
-c: maximum socket connection (default to 5) \
-t: target ip (default to \*) (not implemented yet but will only redirect packets from the target) \
-j: job amount (default to 1, cant be 0 or less) (not implemented yet) \
-T: chance type (default to count) \
-V: chance value (default to 100)

