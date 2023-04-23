# Suborbital-C-Cannon

![status badge](https://github.com/vusec/scc/actions/workflows/tests.yml/badge.svg)

Suborbital-C-Cannon, or **SCC**, is a framework for generating and mutating
random programs.

SCC relies on an *oracle* binary to drive the process. The oracle is given a
program by SCC and then returns several facts to SCC. These facts include a
score that SCC tries to maximize, whether the program is 'interesting' and
should be saved and what information that the user might find interesting about
the program.

## 🗺️ Repository structure

* `scc/program/` The abstract representation of a program.
* `scc/driver/` Implements the TUI.
* `scc/mutators/` General tools useful for mutating a program.
* `scc/utils/` Generic internal utils and data structures.

## 🔨 How to build

Build via:

```bash
mkdir build
cd build
cmake -GNinja ..
cmake --build .
```

## 📑 Oracle protocol

The oracle can communicate back to the driver via stdout. The exit code and
stderr of the oracle are ignored. The working directory of the oracle is the
working directory of the fuzzer (and therefore the working directory from which
the fuzzer is called).

The following commands are available:

### 📈 Scoring

Syntax: `FUZZ:SCORE:INT`

* Example: `FUZZ:SCORE:123`
* Example: `FUZZ:SCORE:-123`

Returns the value of the fitness function that the fuzzer is trying to maximize.
Limited to signed 64-bit values.


### 💡 Mark as interesting

Syntax: `FUZZ:HIT`

When found, the fuzzer will save the test case and clear
the queue.


### 🪦 Mark dead ends

Syntax: `FUZZ:DEAD`

When found, the fuzzer will clear the queue. This indicates that the fitness
function lead the fuzzer to a known dead end. Used if it's cheaper to start from
scratch than to mutate the current test case to something interesting.


### ✉️ Messaging (for debugging)

Syntax: `FUZZ:MSG:STRING`

Example: `FUZZ:MSG:Mutation is lacking a call to malloc`

Sends a message to the driver that will be displayed to the user in the log/TUI.
This is mostly useful to indicate progress and help with debugging oracles.
