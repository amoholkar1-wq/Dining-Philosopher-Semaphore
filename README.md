# Dining Philosophers – Semaphore-Based Solution in C

## Overview
This project is a C implementation of the classic **Dining Philosophers Problem** using **System V semaphores**. It models five philosophers sitting at a round table with five shared forks, simulating the coordination of multiple processes competing for limited resources.

The program ensures:
- No **deadlocks**
- Fair access (each philosopher eats exactly 3 times)
- Safe cleanup of shared semaphores

## How It Works
Each philosopher is launched as a separate process, alternating between thinking and eating. Before eating, they attempt to pick up their left and right forks (represented as semaphores). Semaphore operations (`P` and `V`) are used to coordinate access to these forks and ensure mutual exclusion.

To avoid deadlock:
- Forks are always picked up in a consistent order.
- A separate semaphore tracks how many philosophers are still at the table.

When all philosophers are finished, semaphores are automatically cleaned up by the last process.

## Build Instructions
This program is written in C and uses System V IPC. To compile:
```bash
gcc -o phil philosophers.c
```

## Running the Program
You should start **five** philosopher processes from the command line, each with a unique philosopher ID from 0 to 4:
```bash
./phil 0 &
./phil 1 &
./phil 2 &
./phil 3 &
./phil 4 &
```

Each process will:
- Wait 60–90 (random) seconds before starting
- Think for 10–20 (random) seconds
- Eat for 5–10 (random) seconds
- Repeat this cycle 3 times before exiting

## Notes
- Only philosopher `0` initializes the semaphores.

## Concepts Demonstrated
- Inter-process synchronization
- System V semaphores
- Deadlock avoidance
- Concurrency in Unix/Linux environments
- Proper resource cleanup
