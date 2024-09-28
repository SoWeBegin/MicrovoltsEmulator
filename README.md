# Disclaimer
If you use our emulator for your own server, please don't forget to mention this GitHub page. Also remember that we may add new updates, enhancements or bug fixes anytime, so make sure to stay updated!
We do not take any responsability for public servers.

# Introduction
*If you appreciate our work and the public source code base, make sure to give us a star!*

This is the first public server-emulator for MicroVolts, completely written in C++(20).
Here's a quick video demonstration:

https://github.com/user-attachments/assets/24b7a12a-517d-42a9-9554-fe90d8a9f5a9

# Discord Server
We have a discord server for discussing the development of the emulator and other general discussions.
You can join us through this invite link: https://discord.gg/6pxcxEQNuE

## Contributions
Contributors are always appreciated.
- Feel free to open a new Issue if you find new bugs or missing features (i.e. possible enhancements).
- If you are able to fix an existing bug, feel free to apply it and make a merge request. I will look into it ASAP.

## Reasons for open-sourceness
- Give an idea on how the emulator is written, and possibly inspire new people to create their own better version.
- Learning purposes.
- Make sure that the game stays alive even in the future.
- Allow people to further enhance the game's possibilities.
  
## Instructions
The instructions to build, compile and finally start the emulator will not be shown and the process is left to the programmer. 
We don't want people to host their own servers without having the necessary knowledge on how to solve possible new issues.
The goal here is to make the game better. Not to host multiple identical servers for no reason.

## Dependencies
Most dependencies are already provided inside the `vcpkg.json` file. Instructions: </br>
- Open your terminal and `cd <YourProjectPath>/ExternalLibraries`
- then, `git clone https://github.com/microsoft/vcpkg.git`
- then, `cd vcpkg` and next `.\bootstrap-vcpkg.bat`
- integrate with visual studio: `.\vcpkg integrate install`
- next, move the `vcpkg.json` inside the newly created `vcpkg` folder
- on command prompt: `vcpkg install` (or `.\vcpkg install`)
- open the project on visual studio. For each project (MainServer, CastServer, AuthServer, Common): Project properties => Release => Configuration properties => vcpkg => In "Installed Directory", add the path to the installed directory of vcpkg (example: `..\ExternalLibraries\vcpkg\vcpkg_installed`)
- N.B only `release` mode has all relative (necessary) paths. If you want to use `debug` mode, you'll need to copy paste the paths from the `release` one.

## Client Version
The emulator is targeted for two main versions.
- ENG_0.8_13.37: This is the client that is still being worked on. (Constant database password: `??KVM-7400-$PETER$??`) </br>
- ENG_1.1.1.158 (Surge): This is currently not worked on (discontinued). (Constant database password: `!dptmzpdl@xmfkdlvhtm@goqm!`) </br>
For other versions, the emulator may not work correctly.</br>
*Note: The client will not be published here and you're required to find it on your own.*

# Current state in brief
It is hard to give an in-depth overview of what works and what doesn't. But in brief:
## Authentication server
Almost completely works. 
## Main Server
Overall at a good point. 
Missing: Clan support, Room invites, and some other stuff.
Bugs: See "Issues".
## Cast Server
This is the one that requires the most work. Overall the multiplayer works, but there are known bugs that need to be addressed. See "Issues".


# Contributors
- [@SoWeBegin](https://github.com/SoWeBegin) - Most Core stuff, original author
- [@avlad](https://github.com/avlad171) - (Linux support, CMake => Temporarily removed) & Core stuff
- [@TipicoDev](https://github.com/TipicoDev) - some Core stuff
- [@sw1ndle777](https://github.com/sw1ndle777) - bugs & fixes

