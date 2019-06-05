#! /bin/bash
apt install libxml2
apt install libxml2-dev
ln -s /usr/include/libxml2/libxml/ /usr/include/
wget https://igraph.org/nightly/get/c/igraph-0.7.1.tar.gz
tar zxvf igraph-0.7.1.tar.gz
rm igraph-0.7.1.tar.gz
cd igraph-0.7.1/
./configure
make && make install
