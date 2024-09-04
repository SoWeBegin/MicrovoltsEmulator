# Introduction
This is a server-emulator for MicroVolts, completely written in C++(20).
Here's a quick video demonstration:
https://streamable.com/8xrk6e

## Reasons for open-sourceness
- Give an idea on how the emulator is written, and possibly inspire new people to create their own better version.
- Learning purposes.
- Make sure that the game stays alive even in the future.
- Allow people to further enhance the game's possibilities.
  
## Instructions
The instructions to build, compile and finally start the emulator will not be shown and the process is left to the programmer. 
We don't want people to host their own servers without having the necessary knowledge on how to solve possible new issues.
The goal here is to make the game better. Not to host multiple identical servers for no reason.

# Current state in brief
It is hard to give an in-depth overview of what works and what doesn't. But in brief:
## Authentication server
Almost completely works. 
Some things may be done differently, e.g. when one user enters an account that is busy, one could decide to kick the player who's currently in that account from the server.
## Main Server
Overall at a good point. 
Missing: Clan support, Mailbox support, Joining rooms through the "Join" button, Boxes and special items support.
Bugs: Equipping a set inside a room sometimes fails to work correctly (i.e. other players don't see the new set). There are other minor bugs here and there.
Possible improvements: Rooms have a few minor bugs that can benefit from further fixes.
## Cast Server
This is the one that requires the most work. Overall the multiplayer works, but there are known bugs that need to be addressed, for example:
- Switching the host sometimes causes cast-server crashes.
That is the worst one we have faced during our test phase.
Also, important things are still missing, like complete handling of the ending scoreboard, and persistence-related stuff to matches.



