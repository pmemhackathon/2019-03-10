#
# NVM Workshop Hackathon
# March 10, 2019
# 1:30pm - 4:30pm
# UCSD, Price Center East Ballroom
#
# These are the materials used during the hackathon:
#	slides.pdf contains the slides shown
#	Commmands demonstrated during the hackathon are listed below.
#	Source files for programming examnples are in this repo.
#
# These instructions are designed to work on the Hackathon guest VMs,
# running Ubuntu 18.04.1 (bionic).  Start by cloning this repo, so you
# can easily cut and paste commands from this README into your shell.
#
# Many of the sys admin steps described here can be found in the
# Getting Started Guide on pmem.io:
https://docs.pmem.io/getting-started-guide

#
# Start by making a clone of this repo...
#
git clone https://github.com/pmemhackathon/2019-03-10
cd 2019-03-10

#
# making sure your platform provides an NFIT table
ndctl list -BN     # check the "provider" field

#
# Checking to make sure your kernel supports pmem:
#
uname -r	# see kernel currently running
grep -i pmem /boot/config-`uname -r`
grep -i nvdimm /boot/config-`uname -r`

#
# Use ndctl to show that you have pmem installed:
#
ndctl list -u

#
# Intel-specific (for demo purposes, won't work on hackathon VMs)
#
#ipmctl show -topology
#ipmctl show -dimm
#ipmctl show -memoryresources

#
# create namespace:
#
sudo ndctl create-namespace -f -e namespace0.0 --mode fsdax

#
# Create a DAX-capable file system and mount it:
#
sudo mkdir /mnt/pmem-fsdax
sudo mkfs.ext4 /dev/pmem0
sudo mount -o dax /dev/pmem0 /mnt/pmem-fsdax
sudo chmod 777 /mnt/pmem-fsdax
df -h

#
# Install the latest libmemkind for the volatile pmem example.
# The extra stuff in front of ./build.sh prevents running the tests.
# On hackathon VMs, the tests will run the system out of memory.
#
cd
sudo apt-get install libnuma-dev libtool
git clone https://github.com/memkind/memkind
cd memkind
MAKEOPTS=check_PROGRAMS= ./build.sh
sudo make install

#
# Some of the examples use PMDK.  Many distros include PMDK, but
# it takes some time for the latest version to flow intro the distros
# so here are the steps to build the latest source and install it in
# the usual locations.
#
cd
sudo apt-get install autoconf pkg-config libndctl-dev libdaxctl-dev
git clone https://github.com/pmem/pmdk
cd pmdk
make
sudo make install prefix=/usr

#
# Also install the C++ bindings for libpmemobj for the C++ example.
#
cd
sudo apt-get install cmake
git clone https://github.com/pmem/libpmemobj-cpp
cd libpmemobj-cpp
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install

#
# Now everything in the hackathon repo should build...
#
cd
cd 2019-03-10
make

#
# raw.c contains the simplest possible memory-mapped pmem example
# this illustrates how to use mmap().  nothing transactional here!
#
dd if=/dev/zero of=/mnt/pmem-fsdax/daxfile bs=4k count=1
od -c /mnt/pmem-fsdax/daxfile
./raw /mnt/pmem-fsdax/daxfile
od -c /mnt/pmem-fsdax/daxfile

#
# volatile_pmem.c demonstrates using libmemkind to allocate both
# normal memory and pmem for volatile use.  nothing persistent here!
#
./volatile_pmem /mnt/pmem-fsdax 100

#
# freq.c
#
# Simple C program for counting word frequency on a list on text files.
# It uses a hash table with linked lists in each bucket.
#
# This just sets the stage with a simple programming example.  Nothing
# related to pmem yet.
#
./freq -p words.txt

#
# freq_mt.c
#
# Convert freq.c to be multi-threaded.  A separate thread is created
# for each text file.
#
./freq_mt -p words.txt words.txt words.txt

#
# freq_pmem.c
#
# Convert freq_mt.c to mode the hash table to persistent memory and use
# libpmemobj for pmem allocation, locking, and transactions.
#
# We also use freq_pmem_print.c to display the currrent hash table contents.
#
pmempool create obj --layout=freq -s 100M /mnt/pmem-fsdax/freqcount
pmempool info /mnt/pmem-fsdax/freqcount
./freq_pmem_print /mnt/pmem-fsdax/freqcount
./freq_pmem /mnt/pmem-fsdax/freqcount words.txt words.txt words.txt
./freq_pmem_print /mnt/pmem-fsdax/freqcount

#
# freq_pmem_cpp.c
#
# Similar to freq_pmem.c but uses the C++ bindings to libpmemobj.
#
./freq_pmem_cpp /mnt/pmem-fsdax/freqcount words.txt words.txt words.txt

#
# You brought in many more examples when you cloned the PMDK tree:
#
cd
cd pmdk/src/examples
