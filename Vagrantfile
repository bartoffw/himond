# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  # Every Vagrant virtual environment requires a box to build off of.
  config.vm.box = "ubuntu/trusty64"

  # networking stuff
  config.vm.network :forwarded_port, guest: 80, host: 80
  config.vm.network :forwarded_port, guest: 3000, host: 3000
  config.vm.network :forwarded_port, guest: 8125, host: 8125

  # memory limits
  config.vm.provider "virtualbox" do |v|
    v.memory = 2048
  end

  # shared folders
  config.vm.synced_folder "./src", "/vagrant"

  # provisioning
  config.vm.provision :shell, :path => "provisioner/bootstrap.sh"
  #config.vm.provision :shell, :path => "provisioner/bootstrap-always.sh", run: "always"
end
