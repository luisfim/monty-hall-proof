# Monty Hall Simulator

A small terminal program written in C that uses randomized simulation to demonstrate one of probability theory's most counterintuitive results: in the Monty Hall problem, **switching doors wins approximately two-thirds of the time**, while staying with the first choice wins approximately one-third of the time.

## Screenshot

Add a screenshot of the terminal output to `assets/screenshot.png`, then uncomment the line below:

<!-- ![Monty Hall Simulator terminal output](assets/screenshot.png) -->

## Why I built this

I learned about the Monty Hall problem a long time ago, but its explanation never completely made sense in my head. The result is extremely counterintuitive: after one losing door is opened, only two closed doors remain, so it feels as though each one should have a 50% chance.

I could follow the mathematical explanation, but part of me still did not fully believe it. A simulation was the only thing that could truly convince me that the result was real, so I built one myself.

## The problem

Imagine a game with three closed doors:

- One door hides a prize.
- Two doors hide goats.
- The player chooses one door.
- The host knows where the prize is.
- The host opens a different door and always reveals a goat.
- The player may stay with the original choice or switch to the only other closed door.

The question is:

> Is it better to stay, switch, or does it make no difference?

The correct strategy is to **switch**.

## Why switching works

At the moment of the first choice, the player has:

\[
P(\text{first choice is correct}) = \frac{1}{3}
\]

\[
P(\text{first choice is wrong}) = \frac{2}{3}
\]

The host's action does not make the original choice more likely to be correct. The original door still represents the initial \(1/3\) chance.

### Staying

Staying wins only when the first random choice was already correct:

\[
P(\text{win by staying}) = \frac{1}{3}
\]

### Switching

Switching wins whenever the first random choice was wrong. That happens for either of the two goat doors:

\[
P(\text{win by switching}) = \frac{2}{3}
\]

| Initial choice | Probability | Stay | Switch |
|---|---:|---:|---:|
| Prize | \(1/3\) | Win | Lose |
| Goat | \(2/3\) | Lose | Win |

The host's knowledge is the important detail. The host is not opening a random door that might contain the prize. The host deliberately removes a losing option, concentrating the original \(2/3\) probability of the two unchosen doors onto the single unopened door that remains.

Another way to visualize it is to imagine 100 doors. You choose one, with only a 1% chance of being correct. The host then opens 98 losing doors and leaves your door plus one other door closed. Your first door does not suddenly become a 50% choice; it is still the unlikely 1% choice. The other closed door carries the remaining 99% chance.

## What the program does

For every trial, the program:

1. Places the prize behind a random door.
2. Makes a random initial player choice.
3. Makes the host open a valid goat door.
4. Evaluates the result if the player stays.
5. Evaluates the result if the player switches.

The same randomized trials are used for both strategies. Therefore, entering `100` means:

- 100 outcomes for always staying
- 100 outcomes for always switching
- 200 strategy outcomes in total

The report displays:

- Correct and incorrect initial choices
- Wins and losses for each strategy
- Win percentages
- ASCII comparison bars
- The difference in percentage points
- The ratio between the two observed win rates
- The theoretical values of 33.33% and 66.67%

Small samples can vary considerably. With tens of thousands or millions of trials, the observed results should move increasingly close to the theoretical probabilities.

## Important assumptions

The result depends on the standard rules of the puzzle:

1. The host knows where the prize is.
2. The host always opens a door the player did not choose.
3. The host always reveals a goat, never the prize.
4. The host always offers the player the opportunity to switch.
5. When the host has two valid goat doors available, this program chooses between them at random.

Changing the host's behavior can change the probabilities.

## Build and run

### Linux and macOS

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 monty_hall.c -o monty_hall
./monty_hall
```

### Windows with MinGW/GCC

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 monty_hall.c -o monty_hall.exe
.\monty_hall.exe
```

No external libraries are required.

### Disable terminal colors

The interface uses ANSI colors where supported. Set the [`NO_COLOR`](https://no-color.org/) environment variable to disable them.

Linux or macOS:

```bash
NO_COLOR=1 ./monty_hall
```

Windows PowerShell:

```powershell
$env:NO_COLOR = "1"
.\monty_hall.exe
```

## Example

```text
How many games should each strategy play?
Enter a number from 1 to 100,000,000: 1000000

STRATEGY RESULTS

  STAY WITH THE FIRST DOOR
  Wins   : approximately 333,333
  Losses : approximately 666,667

  SWITCH TO THE OTHER DOOR
  Wins   : approximately 666,667
  Losses : approximately 333,333
```

The exact numbers change on every run because the doors and first choices are randomized.

## Does this prove the result?

Not by itself. A simulation provides **empirical evidence**, while the probability calculation provides the mathematical proof.

However, simulation is particularly useful here because the correct result clashes so strongly with intuition. Watching the percentages repeatedly converge toward one-third and two-thirds makes the abstract explanation much easier to trust.

## Technical details

- Language: C11
- Interface: terminal
- Simulation type: Monte Carlo experiment
- Random generator: SplitMix64, seeded at runtime
- Maximum trials per strategy: 100,000,000
- Dependencies: standard C library only
- Supported environments: Linux, macOS, and modern Windows terminals

SplitMix64 is used only to generate pseudorandom simulation values. It is not a cryptographic random-number generator.

## Background and sources

The puzzle is modeled after the American television game show *Let's Make a Deal* and is named after its longtime host, Monty Hall. Statistician Steve Selvin presented the problem in game-show form in letters published in *The American Statistician* in 1975. It became widely known after Marilyn vos Savant answered a reader's version of the puzzle in her *Parade* column in 1990.

The mathematical puzzle uses a precise host protocol. The real television show did not necessarily follow these exact rules in every situation.

Further reading:

- Steve Selvin, ["A Problem in Probability"](https://www.jstor.org/stable/2683689), *The American Statistician*, 1975
- [Utah State University: The Monty Hall Problem](https://www.usu.edu/math/schneit/StatsHistory/ProbabilityPrompts/MontyHall)
- [UC San Diego: The Monty Hall Page](https://math.ucsd.edu/~crypto/Monty/Montytitle.html)
- [Parade: The Two Goats, Three Doors Question and Solution](https://parade.com/533284/npond/the-two-goats-three-doors-question-and-solution/)

## Project structure

```text
monty-hall-simulator/
├── monty_hall.c
└── README.md
```
