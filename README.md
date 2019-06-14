# tinytangle


**Before Compile**

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
