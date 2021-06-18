# PsyDoom Lua Scripting Overview
As an extended modding feature, PsyDoom features a limited Lua scripting interface (currently Lua 5.4) to allow for more control over line and sector specials. This feature is deliberately limited in scope and mostly provides additional flexibility for events and interactive objects in the map. It was inspired somewhat by the Doom 64 engine's 'Macro' scripting system.

## Capabilities
- The ability to trigger scripted actions for switches, line triggers etc.
- More control over timing and the exact behavior of ceilings, platforms and floors.
- The ability to spawn custom sector specials (e.g custom line scroll)

## Limitations
- Writing to global variables is **completely disallowed**. This restriction is made to keep level data easily serializable. If you need a global data store however, you can work around this limitation by encoding data in sector fields (e.g. floor or ceiling height).
- Just one script is allowed per map.
- Scripts cannot add new enemies or object types or change enemy behavior.
- Only a limited subset of the game's API and data structures are exposed to scripting.

# Setting up a basic Lua script
1. A `SCRIPTS` lump must first be added to the `MAP[XX].WAD` file. In order to work well with PSX Doom Builder this should be placed *before* the map number marker; failure to do so may result in the map format not being recognized. For example, if placing a script in `MAP01.WAD` then the lump order should look like this:
```
SCRIPTS
MAP01
THINGS
LINEDEFS
...
```
2. A script file consists of a series of actions defined using the function `SetAction`. When PsyDoom first loads the script, it executes the entire file and all calls to `SetAction`. This builds a list of scripted actions that the engine can trigger via line and sector specials. *Note: once this initial execution phase is over `SetAction` becomes unavailable and no other actions can be registered.*
3. The `SetAction` function takes 2 parameters, the first being the number of the action. This number is referenced by line and sector specials (via their 'tag' fields) and is used to identify which action to execute. The 2nd parameter is a Lua function containing the logic to execute for the action. Here is a very basic example defining action `1` which prints "Hello World" to the console (if PsyDoom is launched via a terminal):
```
SetAction(1, function()
    print("Hello world!")
end)
```
4. In order to trigger the action in-game, use a PsyDoom specific 'scripted' line or sector special. For a line special the line tag identifies which action number to execute. For a sector special the sector tag identifies which action to execute. For example, assigning action `300` to a line (`W1 Do Script Action (Player only)`) and setting the line tag to `1` will cause script action `1` to be executed one time only, when the player walks over the line.

# Available Script Execution Hooks
## Scripted Linedef Special Types
Below is a list of PsyDoom specific line special numbers which trigger scripted actions. The prefix `W` means walk over the line, `S` means use as a switch, `G` means shoot and `D` means use as a door. The suffix `1` means once, and `R` means repeatable.
- 300: W1 Do Script Action (Player only)
- 301: W1 Do Script Action (Monsters only)
- 302: W1 Do Script Action (Player + Monsters)
- 310: WR Do Script Action (Player only)
- 311: WR Do Script Action (Monsters only)
- 312: WR Do Script Action (Player + Monsters)
- 320: S1 Do Script Action (Player only)
- 330: SR Do Script Action (Player only)
- 340: G1 Do Script Action (Player only)
- 341: G1 Do Script Action (Monsters only)
- 342: G1 Do Script Action (Player + Monsters)
- 350: GR Do Script Action (Player only)
- 351: GR Do Script Action (Monsters only)
- 352: GR Do Script Action (Player + Monsters)
- 360: D1 Scripted Door (Player only)
- 361: D1 Scripted Door (Monsters only)
- 362: D1 Scripted Door (Player + Monsters)
- 370: DR Scripted Door (Player only)
- 371: DR Scripted Door (Monsters only)
- 372: DR Scripted Door (Player + Monsters)
- 380: Scripted Spawn Line Special
    - Executes once during map loading for the affected lines. It allows for custom effects to be spawned.

## Scripted Sector Special Types
Below is a list of PsyDoom specific sector special numbers which trigger scripted actions.
- 300: Scripted Sector Special Spawn
    - Executes once on startup for a sector. It allows for custom effects to be spawned.
- 301: Scripted Player In Special Sector Action
    - Executes every game tick (15 times per second) that the player is in the sector.

# Available Lua APIs
1. PsyDoom provides a subset of the `base` Lua library for scripts. Available functions are:
```
tonumber    tostring
print       error
pairs       ipairs
next        select
pcall       xpcall
type
```
2. The entire `math` library is also available.
