# :ship: CBattleships
Like Battleship game, but with intriguing addons. This game was created as an assignment for one of computer science courses (in the late 2021).

```
  000000000011111111112222222222
  012345678901234567890123456789
00       ????????????????? ?????
01       ???????????????     ???
02  @     ?????????????       ??
03  !#   ??????????????       ??
04  x    ????????????? %+!@    ?
05  +    ??????????????       ??
06  %   ???????????????       ??
07?? ???????????????????     ???
08???????????????????????# ?????
09??????????????????????????????
10??????????????????????????????
11??????????????????????????????
12??????????????????????????????
13??????????????????????????????
14???????????   ????????????????
15??????????? # ????????????????
16???????????   ????????????????
17??????????????????????????????
18??????????????????????????????
19??????????????????????????????
```
:point_up: Exemplary look of board printed by player A
It can be recreated using these commands:
```
[state]
BOARD_SIZE 20 30
NEXT_PLAYER B
INIT_POSITION A 0 0 9 9
SET_FLEET A 1 2 3 4
SHIP A 2 2 N 0 CAR 11011
SHIP A 4 24 E 0 BAT 1111
INIT_POSITION B 11 0 20 9
SET_FLEET B 1 2 3 4
REEF 3 3
REEF 15 12
REEF 8 23
EXTENDED_SHIPS
[state]
```
though it won't recreate spies, here at position (15, 12) (as it was not allowed by project specification).

## General info
By default there is 21x10 board and each player has 1 ship of size 5 (carrier), 2 ships of size 4 (battleships), 3 ships of size 3 (cruisers), 4 ships of size 2 (destroyers). Player can have maximally 10 ships of a given type. When providing ship classes in commands abbreviations should be used:

- CAR for carriers
- BAT for battleships
- CRU for cruisers
- DES for destroyers

By default player A can place ships in rows 0..=9 and player B in rows 11..=20. Players can shoot each other in turns. If any of the players destroy all of enemy ships then he wins the game.

### Extended logic
As it was mentioned in description - this game has some interesting extension and these features are described in this section.
This logic must be enabled using state command named EXTENDED\_SHIPS (see below how to use commands, command groups).
Shortly, in this mode players can move their ships, board may be set to custom size, ships are composed of functional parts like engine, cannon and radar. Also only fields in radar range are printed, everything else is covered in "war fog". In extended logic mode game may be saved using SAVE command.

#### Functional parts
Ship may look like that:
```
@!++%
```
- @ - radar - only fields in radar range are visible to player (rest are covered by ?). Its range is length of the ship or 1 if the radar is destroyed
- ! - cannon - allows ship to shoot if not broken. It has unlimited range for carriers, for the rest it is up to radius of ship's length. Ship may shoot ship's length times in one turn
- \+ - healthy part
- % - engine - allows ship to move if not broken


## Commands
### State
These comamnds are meant to be used in \[state\] group.
- PRINT \<TYPE\>
  - 0 - basic version
    - '+' - field occupied by ship
    - 'x' - destroyed part of ship
    - ' ' - empty field
  - 1 - advanced version
- SET\_FLEET \<PLAYER\_NUMBER\> \<CARRIERS\> \<BATTLESHIPS\> \<CRUISER\>
  \<DESTROYER\>
- NEXT\_PLAYER \<PLAYER\_NUMBER\> - sets PLAYER\_NUMBER player turn to be now
- BOARD\_SIZE \<Y\> \<X\> - sets board size to Y rows and X columns
- INIT\_POSITION \<PLAYER\_NUMBER\> \<Y1\> \<X1\> \<Y2\> \<X2\> - sets region
represented by points (Y1, X1) and (Y2, X2) as an area where PLAYER\_NUMBER
player can place his ships (inclusive)
- REEF \<Y\> \<X\> - places reef at position (Y, X). Reef means a field
where no part of any ship may be placed or moved
- SHIP \<PLAYER\_NUMBER\> \<Y\> \<X\> \<DIRECTION\> \<IDX\> \<CLASS\> \<PARTS_HEALTH\> - places PLAYER\_NUMBER player ship (IDX-th) of class CLASS at position (Y, X) pointing to DIRECTION
	- PARTS\_HEALTH informs if parts of placed ship are destroyed or not. For example if a ship length is 4 then 0110 would mean that first and last part of the ship are destroyed
- EXTENDED_SHIPS - enables advanced logic of game. Ships will be composed of functional parts like cannon, radar etc.
- SRAND \<SEED\> - sets seed of random number generator to SEED
- SAVE - save the game state as a sequence of state commands (printed in terminal). Game can be later fully loaded by using these commands.

### Player
These command are to be used by players, in [playerA] or [playerB] command group.

- PLACE\_SHIP \<Y\> \<X\> \<DIRECTION\> \<IDX\> \<CLASS\> - place IDX-th ship of CLASS at position (Y, X) pointing DIRECTION
- SHOOT \<Y\> \<X\> - shoot field (Y, X). Shooting is only possible if all ships were placed
- MOVE \<IDX\> \<CLASS\> \<FORWARD|LEFT|RIGHT\> - move IDX-th ship of CLASS FORWARD or TO THE LEFT or TO THE RIGHT
- SHOOT \<IDX\> \<CLASS\> \<Y\> \<X\> - shoot from IDX-th ship of CLASS at (Y, X) field
- SPY \<IDX\> \<Y\> \<X\> - send a spy plane from IDX-th carrier to position (Y, X). It can be sent only by any earlier placed carrier (in previous turn) as many times as carrier can shoot (so 5 for each carrier). Each spy uncovers 3x3 region of map around (Y, X) point. Every sent spy counts as a shoot
- PRINT \<TYPE\> - usage as in state, but only parts visible to the player will be shown

## Command groups
```
[state]
SET_FLEET A 1 1 0 1
SET_FLEET B 0 1 1 0
[state]
[playerA]
PLACE_SHIP 9 4 W 0 BAT
PLACE_SHIP 9 0 W 0 DES
PLACE_SHIP 5 9 N 0 CAR
[playerA]
[playerB]
PLACE_SHIP 11 7 W 0 CRU
PLACE_SHIP 11 4 E 0 BAT
[playerB]
```
:point_up: Example of using command groups. In different groups different commands are available and some commands may work in the other way. [state] commands should be used by "admin" and [playerX] groups - by individual players.
