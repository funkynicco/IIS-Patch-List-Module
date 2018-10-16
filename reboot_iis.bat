@echo off

: This batch stops IIS and Windows Process Activation Service (WAS) (which is used to spawn w3wp.exe processes).
: WAS must be stopped to release a loaded native module.
: This batch allows updating the native module.
: Starting IIS (W3SVC) again will automatically start WAS.

net stop W3SVC
net stop WAS

echo Press any key to start W3SVC again ...
pause

net start W3SVC