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

### Idea
##### 1. 
I have add batch read/write. I mean acquire memory in batch and then fill them and send to reduce contention between writer threads.
In acquire function :
```cpp
+  static constexpr int BATCH = 5;                                                                                      
+  static thread_local int index = BATCH;                                                                               
+  static thread_local uint8_t *data = nullptr;                                                                         
+                                                                                                                       
+  if (index == BATCH) [[unlikely]] {                                                                                   
+    data = static_cast<uint8_t *>(superqueue::dequeue<superqueue::SyncType::MULTI_THREAD,                              
+                                        superqueue::Behavior::FIXED>(mempool->pool, BATCH));                           
+    if (data == nullptr) [[unlikely]]                                                                                  
+        return nullptr;                                                                                                
+    else                                                                                                               
+        index = 0;                                                                                                     
+  }                                                                                                                    
+                                                                                                                       
+  return data - (index++ * BLOCK);                                                                                     

```
But After this change there is not improvment and the reason is current bottle-neck is in single thread consumer side.
I will try multi-consumer mode.

##### 2.
As mentioned in section 1, we have problem in reader thread, so according to profiler we have spend considreable amount of time in vtable find of related to process function of events. As events are one layer polymorphicy call process function, I have expected compiler optimized them out, but ot is wrong expectation. So we will try static polymorphism or pass static function and use plain struct as events.
