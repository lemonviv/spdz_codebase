# SPDZ Connector

This repository gives a c++ codebase example for connecting to MP-SPDZ engines. 
To start with the example, please clone the [MP-SPDZ](https://github.com/data61/MP-SPDZ/tree/72c2d4b3f73fff93c4d7c2cc58bc6e146eaf3a9b) 
library with the specific branch. 

## MP-SPDZ Installation

After cloning the MP-SPDZ library, replace two files with in the `spdz_replace` folder:
one is the `Makefile` file to `${SPDZ_HOME}/`, the other is the `Setup.h` file to 
`${SPDZ_HOME}/Math/Setup.h`. For the latter file, need to change the prefix `PREP_DIR`
to the cloned MP-SPDZ home. Besides, to run the MP-SPDZ programs in an insecure mode, 
copy the `CONFIG.mine` file to `${SPDZ_HOME}/`. 

Next, run the following scripts under the MP-SPDZ home to install MP-SPDZ as a library, 
and generate the necessary certificates, executables, and so on.

```
# clean the folder
make clean

# install the MP-SPDZ dependency library mpir
make mpir

# install MP-SPDZ as library, will generate libSPDZ.a and libSPDZ.so
make lib

# compile semi-party.x for running additive secret sharing protocol
make -j 8 Player-Online.x
make -j 8 Fake-Offline.x
make -j 8 semi-party.x

# setup certificates and keys, assume there are three SPDZ parties
bash Scripts/setup-clients.sh 3
bash Scripts/setup-online.sh 3 128 128
bash Scripts/setup-ssl.sh 3 128 128
bash Scripts/setup-online.sh 3 192 128
bash Scripts/setup-ssl.sh 3 192
```

Then, copy the `add_computation.mpc` MPC program to 
`${SPDZ_HOME}/Programs/Source/` folder, and compile it using the following 
script.

```
./compile.py Programs/Source/add_computation.mpc
```

Finally, open three terminals and run the following scripts in the three
terminals to start the MPC computing engines, where `-p` denotes the MPC
party id, `-N` denotes the total MPC party number, more notations can be
found in the MP-SPDZ page.

```
./semi-party.x -F -N 3 -I -p 0 add_computation
./semi-party.x -F -N 3 -I -p 1 add_computation
./semi-party.x -F -N 3 -I -p 2 add_computation
```

After that, if seeing the following in the terminals, then the MPC engines are
started correctly.

```
Start listening on thread 140153657132800
Party 1 is listening on port 14001 for external client connections.
Listening for socket connections on base port 14000
Starting a new iteration.
Thread 140153657132800 found server.
```

## Connector

For the connector, need to change the `SPDZ_HOME` in `CMakeLIsts.txt` to the 
cloned path. Then, can compile the program using the following scripts:

```
mkdir build
cmake -Bbuild -H.
cd build/
make
```

Next, go to the build path, and run three parties in three terminals as follows, 
where the first argument is the number of total parties, and the second is the 
party id. 

```
./spdz_codebase 3 0
./spdz_codebase 3 1
./spdz_codebase 3 2
```

These three parties will connect to the three MPC engines, and each party will 
send a private value to the MPC engines, the MPC engines aggregate the numbers 
and return the result. If seeing the following result, the example is correct.

```
Hello, World!
begin connect to spdz parties
party_num = 3
Set up socket connections for 0-th spdz party succeed, sockets = 0x6120000031c0, port_num = 14000.
Set up socket connections for 1-th spdz party succeed, sockets = 0x6120000061c0, port_num = 14001.
Set up socket connections for 2-th spdz party succeed, sockets = 0x612000007cc0, port_num = 14002.
Finish setup socket connections to spdz engines.
Using prime 170141183460469231731687303715885907969
Finish initializing gfp field.
receive spdz computation result
Receive mpc computation result from the SPDZ engine
v = 6
```