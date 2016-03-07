#!/usr/bin/env bash
cd /vagrant/provisioner

sudo su

# update everything
apt-get update
apt-get -y upgrade

# install NTP daemon
apt-get -y install ntp

# set timezone to CET
ln -sf /usr/share/zoneinfo/CET /etc/localtime

### configuring UFW ###
ufw default deny incoming
ufw default allow outgoing
ufw allow ssh
echo y | ufw enable

# install Docker
curl -sSL https://get.docker.com/ | sh
# be able to run Docker as a non-root user
usermod -aG docker vagrant