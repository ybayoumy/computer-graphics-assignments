# Spaceship Game

## Controls

- The ship will slowly rotate to look in the direction of the cursor. The mouse cursor needs to be on the window for it to register.
- Move the ship forwards using the `W` key and backwards using the `S` key
- Picked up diamonds (and their corresponding fireball) will follow the ship but are no longer part of the game.
- Pick up diamonds to increase your score and scale up the ship's size
    - Once all diamonds have been picked up, they will spin and you win the game!
- Avoid touching the fireballs. If a fireball is hit, you lose the game!
- To restart the game at anytime, press the `R` key.

If the ship speed and rotation is too slow, update the `SHIP_SPEED` and `SHIP_ROTATION_SPEED` values as needed to make the game more enjoyable.

Likewise, update the `FIREBALL_ROTATION_SPEED` value as needed to make the game harder/easier.

## Platform and Compiler

Visual Studio 2022 was used to compile and run the source code

Platform (OS): Windows 10 Pro

Compiler: C++17
