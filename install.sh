#/usr/bin/env bash

cp services/dolosd-openrc.sh /etc/init.d/dolosd
chown root:root !$
chmod +x !$

cp services/prestart.sh /etc/init.d/dolosd.d/
chown root:root /etc/init.d/dolosd.d/prestart.sh
chmod +x !$

cp services/poststop.sh /etc/init.d/dolosd.d/
chown root:root /etc/init.d/dolosd.d/poststop.sh
chmod +x !$

cp bin/dolosd 
