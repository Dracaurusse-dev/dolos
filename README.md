# http-redirect
Simple proxy installed on an apache webserver to redirect from a port to another most of the time and sometimes on a troll port.

Create a new entry by calling ./proxy proxy_port apache_port troll_port [chance]

with default chance being 100, 1/100 chance of being redirected to the troll port
