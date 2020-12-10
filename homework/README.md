# Which practice Hems GLPK in each few part

**Notice** Method Cmake and makefile is using different GLPK version, please ask someone who knows how to set Cmake ```include_directories```
### makefile 
* use ```make all``` or ```make clean``` or ```make rebuild``` to compile or clean or re-complie

### We use CMake instead of makefile. If there is a file compile with CMake and someone want to compile new code by CMake,

1. please change your path in local dir name CMakeLists.txt 
2. ```cd``` to ```build``` dir and commamd ```cmake ..```
3. ```make```
4. check your excute file generated in build dir.

* if change your code and **don't** change CMakeLists.txt in local dir, repeat step 3 ~ end
* if **change** CMakeLists.txt in local dir, repeat step 1 ~ end

---
```three load vs grid version``` and ```three load vs grid and solar version``` is only changed in **balanced function**, so see the ```three load vs grid``` dir balanced function part in you doing the ```three load vs grid and solar``` exercise.

---
Error experience: ```glp_set_col_bnds: j = 1921; column number out of range```. due to column boundary is associate with the variable's total, so make sure that total is correct.
