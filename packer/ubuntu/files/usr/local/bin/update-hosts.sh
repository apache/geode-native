#!/bin/bash

# add hostname to /etc/hosts if not set
if (! getent hosts `hostname` >/dev/null); then
  echo `hostname -I` `hostname` >> /etc/hosts
fi
