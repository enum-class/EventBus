# EventBus
Thread safe event-bus

## Build <a name="build"></a>
### Prerequisites <a name="pre"></a>
```
sudo apt-get install python3
pip3 install conan
```
### Linux <a name="linux"></a>
```
cmake --preset={unix-release/unix-dev}
cmake --build {Release/Debug}
```
### Windows <a name="windows"></a>
```
cmake --preset={windows-release-x64/windows-dev-x64}
cd {Release/Debug}
msbuild.exe HFT.sln /p:Configuration={Release/Debug} /p:Platform=x64
```
## Benchmark <a name="benchmark"></a>
CPU : 12th Gen Intel(R) Core(TM) i7-1265U

| Queue size 1000 with 64 byte blocks |                   |
|--------------------------------------|-------------------|
|     Million  operation per second    | Number of threads |
| 16                                   | 1                 |
| 10                                   | 2                 |
| 8                                    | 4                 |
| 7                                    | 8                 |

## Profile

### Prerequisites
```
cmake --preset unix-profile
cmake --build Profile
```
### valgrind
```
./scripts/valgrind.sh Profile/BenchMark
```

### gprof
```
./BenchMark
ls gmon.out
gprof Profile/BenchMark gmon.out > profile.txt
```

### valgrind/callgrind
```
valgrind --tool=callgrind ./BenchMark
kcachegrind profile.callgrind
```
