# Bjorn's AI MERVBot Plugin

## About

This is an AI plugin for MERVBot originally written by Bjorn.

## Build Instructions

On Debian Bullseye

```
# install external dependencies
sudo apt install cmake build-essential

# checkout internal dependencies
git submodule update --init --recursive

# install dependencies required by mervbot (see `dep/mervbot/README.md`)
# ...

mkdir build
cd build
cmake ..
make
```

## Execution

In `build/Spawns.txt` all bot instances are defined. Each bot instance has
a configuration field for its username, password, the arena it enter,
a list of plugins (in the form of dynamically linked libraries) it
will load, a staff password and additional plugin parameters.

In order to start the bots run `cd build && ./mervbot`. To exit press
any key.

Players can interact with the bots by private messaging them. `build/Operators.txt`
defines who gets which privileges when providing the correct password.
Players have to private message the bot with `/!login $PASSWORD` to login.
`/!help` lists additional commands that can be issued given the current
privileges of the player. `/!version` reports loaded plugins.

For an exhaustive list of commands please check `build/Commands.txt` which
can also be used for defining additional command aliases.

The AI functionality can be configured in `cfg/AI.ini` which gets propagated
to `build/AI.ini` when building the project.