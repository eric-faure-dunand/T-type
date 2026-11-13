# Accessibility

What R-Type World Tour offers to players with disabilities, following the three categories of the subject. The options are in the **Options** screen: press **O** in the menu or in the room list. They are saved in `r-type_settings.json` next to the game, so they are kept for the next game.

| Option | Default | Effect |
|---|---|---|
| Auto-fire | off | The ship fires on its own: no need to hold SPACE. Also switched in game with **F** |
| Reduce motion | off | Stops the scrolling background, the stars, the menu slideshow and the blinking cursor |
| Darker backgrounds | off | Darkens the photos behind the menu and the game, so ships, missiles and text stand out |

## Physical and motor disabilities

- **Auto-fire**: playing needs only the movement keys, instead of holding SPACE while moving. Shooting games like *Ikaruga* or *Enter the Gungeon* offer the same idea.
- **Two sets of movement keys**: arrows or WASD (ZQSD on a French keyboard). The player picks the hand and the keys that suit them. Raylib reads the key position, so WASD works on any keyboard layout.
- **No key combinations, no double press, no time limit** in the menus. Every screen works with a few keys (arrows, ENTER, ESC), and the menus also work with the mouse.
- **Inputs are resent** by the client (see [Client](Client.md#inputs)): a key held down keeps the ship moving even if a packet is lost, so the player does not need to press again.

## Visual disabilities

- **Players are not told apart by colour only**: each ship is the face of a different person, and the game writes *You are Ana* at the top of the screen. Player missiles and enemy missiles have different sprites, not only different colours. This helps colour-blind players.
- **Readable text**: Exo 2, a clear sans-serif font, light text with a dark shadow so it stays readable on any photo.
- **Darker backgrounds** option for players who need more contrast.
- **The keys are highlighted** in yellow in every help line (`ENTER to connect  -  ESC to quit`).

## Hearing disabilities

The game does not need sound to be played: every important event is also shown on screen.

| Event | On screen |
|---|---|
| Level start | Message with the level name and country |
| Boss arrives | `BOSS : <name>` message and a health bar at the top |
| Own ship destroyed | *Game over* screen |
| Connection lost, room full, room closed | Message at the bottom of the screen |

When sound is added, each new sound must keep a visual equivalent.

## Mental and cognitive disabilities

- **Same keys on every screen**: arrows to choose, ENTER to confirm, ESC to go back.
- **Every screen says what to do**: a help line at the bottom lists the keys, and the game screen reminds the controls during the whole game.
- **Reduce motion** removes moving elements that are not needed to play, for players who get distracted or feel sick with movement (also useful against motion sickness and for photosensitive players).
- **Auto-fire** leaves one thing to do at a time: moving.
- **Clear feedback**: the boss health bar shows the progress, and the *Game over* screen explains what happened before going back.

## Not done yet

- Changing the keys from the game (only the two built-in sets are available).
- Gamepad support.
- Bigger text option.
- Difficulty setting (slower enemies, more health), as the *Assist mode* of *Celeste*. It belongs to the server, since the server decides the game rules.
