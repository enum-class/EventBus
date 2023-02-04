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
