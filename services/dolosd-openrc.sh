#!/sbin/openrc-run

depend() {
	need net
}

name="dolosd"
description="Daemon for dolos, will start proxies to redirect people on the wrong website sometimes"
command="/usr/bin/dolosd"
command_background=true
pidfile="/run/openrc/daemons/dolosd.pid"
output_log="/var/log/dolos/dolosd-service.log"
error_log="/var/log/dolos/dolosd-service.err"

stop() {
	/etc/init.d/dolosd.d/poststop.sh
}

start_pre() {
	/etc/init.d/dolosd.d/prestart.sh
}

