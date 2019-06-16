# tinytangle

## What is tinytangle  

It is a blockchain, but it is not a chain. Because the connection relationship of these blocks is a *directed acyclic graph*. The rototype of DAG here is IOTA's TANGLE. It is a special blockchain for IOT.  
In this project, a node will gets IOT data from China Telecom IOT cloud platform and the serial port which is just for experiment. Then the node packages these data into a unit by tips selection and encryption. Finally the unit is stored and broadcast.  

## Introduction
The following are some of the basic technologies involved.  
Language - *C/C++*  
Tips selection - Random which should have been *Markov Chain Monte Carlo(MCMC)*  
Encryption - *Crypto++/RSA* & *PoW* with a very low difficulty  
Web text - *JSON*  
local storage - *Sqlite3*  
Netowrk - *LAN/Socket* which should have been p2p.  
But, unfortunately I don't have enough time and energy to finish a p2p network or the connection with China Telecom IOT.


## Installation
**Before Compiling**

Make sure you have installed *boost*, *crypto++* and *cmake*.  

```sh
$ sudo apt-get install libboost-all-dev
$ apt-cache pkgnames | grep -i crypto++
$ sudo apt-get install libcrypto++9v5 libcrypto++-dev
$ sudo apt-get install cmake
```
You can use *sqlitebrowser* to view the database  

```sh
$ sudo apt-get install sqlitebrowser
```

**Compile**  

```sh
$ sh ./install.sh
```

**run**  

```sh
$ sh ./run.sh
```

**command**  
You can send a command by udp client.  
```
"once"  ----generate a unit  
"start" -----generate units all the time  
"pause" -----stop genertating  
"tips" ----- show tips  
"quit" ------ quit  
```  
## At last but not least
It's `Bin Cao` that took me to learn the blockchain and improve my ability. I am very grateful to Teacher `Bin Cao`. Although he used to say that he did not teach me anything.
