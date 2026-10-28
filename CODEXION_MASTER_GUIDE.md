# CODEXION — THE MASTER GUIDE

> A single document to read once and walk away able to explain and defend the whole project.
> Every claim here comes from the real code in this folder. Where the code is unfinished, you will see a box marked **⚠️ GAP / TO VERIFY** instead of a comfortable lie.

A tiny vocabulary note before we start, because these three words appear immediately:

- **Source code** = the text files you write (`main.c`, etc.).
- **Compile** = translate that human text into a file the computer can actually run (a "binary", or "executable").
- **Run / execute** = tell the computer to start doing what the binary says.

---

# PART 0 — WHAT CODEXION IS

### For a 10-year-old (5 sentences)

1. Imagine 3 coders sitting around a table, and between each pair of them there is **one** shared USB dongle (a little stick you plug in — here it is a thing you must hold to be allowed to work).
2. Each coder keeps repeating the same three jobs forever: **compile**, **debug**, **refactor** (three kinds of work on their code).
3. To **compile**, a coder must be holding the dongle on their left AND the dongle on their right at the same time — but there are not enough dongles for everyone to compile at once, so they have to take turns.
4. If a coder waits too long and never gets to compile in time, they **burn out** (give up, exhausted) — and that must never happen.
5. The program's whole job is to let all these coders work at the same time, share the dongles fairly, print what each one is doing with a timestamp, and never freeze up with everyone waiting on everyone else.

### The same thing in real technical words

1. Codexion is a **concurrency simulation** (a program where several lines of work run at the same time) with `N` worker **threads** (independent streams of execution inside one program) called *coders*, arranged in a ring, each sharing a **mutex** (a lock that only one thread can hold at a time) called a *dongle* with each neighbour.
2. Each coder thread runs an infinite **work loop**: `compile → debug → refactor`, repeating.
3. Compiling requires the coder to **lock two specific mutexes** (its left dongle and its right dongle) simultaneously; because dongles are shared, threads contend for them and must block (wait) until both are free.
4. A **monitor** is meant to detect **burnout** — a coder going longer than `time_to_bo` milliseconds without starting a compile — and stop the simulation; burnout is the failure condition.
5. The program must achieve this with no **deadlock** (a frozen state where threads wait on each other in a cycle), no **data race** (two threads touching the same memory unsafely), and no **memory leak** (allocated memory never given back).

This is the classic "Dining Philosophers" problem wearing a software-engineering costume. Philosophers → coders. Forks → USB dongles. Eating → compiling. Starving → burning out.

### What Codexion is NOT (so you don't carry wrong ideas)

- It is **not** a real compiler. The words "compile / debug / refactor" are just names for "spend some milliseconds holding resources or waiting." Nothing is actually built.
- It is **not** using multiple **processes** (separate programs). It is one process with many **threads** inside it (Part 2 explains the difference).
- It does **not** talk to the network, disk, or any real USB device. A "dongle" is a `pthread_mutex_t` in memory, nothing more.
- It is **not 100% finished.** As of this writing the burnout/quota **monitor exists in a file but is not wired into the program**, so the current build runs forever. This is documented honestly in the gap boxes below — do not claim in a defence that burnout works today, because it does not yet.

---

# PART 1 — THE MAP

### Every file, one line each

| File | What it does | Why it exists |
|---|---|---|
| `codexion.h` | The **header** (a shared file of type definitions and function declarations that every `.c` includes). Holds all structs and every function prototype. | So every `.c` file agrees on the same data shapes and function names. |
| `main.c` | The entry point: parse → init → create coder threads → wait for them → clean up. | Every C program needs one `main`; this is the conductor. |
| `parser.c` | Reads the 8 command-line arguments, checks they are valid, converts text to numbers. | Garbage input must be rejected *before* anything is built. |
| `utils_time.c` | Time helpers: `now_in_ms`, `precise_wait`, and `should_stop`. | Timing is the heartbeat of the whole sim; stop-checking lives here too. |
| `init.c` | Allocates the coder/dongle arrays, initializes every mutex, records start time, and `cleanup` frees it all. | Builds the world before threads run, and tears it down after. |
| `sim.c` | `log_state`: prints one timestamped status line, safely. | All output must go through one gate so lines never scramble. |
| `coder_routine.c` | The code each coder thread runs: take dongles → compile → release → debug → refactor, looping. | This is the actual behaviour being simulated. |
| `monitor.c` | **(exists, not yet connected)** Burnout detection + compile-quota detection; flips the stop flag. | Meant to be the referee that ends the sim. See gap box. |
| `Makefile` | The build recipe: how to turn the `.c` files into the `codexion` binary. | So you type `make` instead of a long compiler command. |
| `objs/` | Folder of compiled `.o` **object files** (half-built pieces, one per `.c`). | Lets `make` rebuild only what changed. Not source; generated. |
| `codexion` | The final built **executable** (the runnable program). | This is the thing you actually run. Generated, not source. |

> **⚠️ GAP / TO VERIFY — `monitor.c` is an orphan**
> `monitor.c` defines `monitor_routine`, but:
> - it is **missing from the `Makefile`** `SRCS` line, so it is never compiled;
> - it has **no prototype** in `codexion.h`;
> - `main.c` never calls `pthread_create` for it and never joins it.
> Result: today the monitor code does nothing. The program compiles and runs, but loops forever. To make it real you must add `monitor.c` to the Makefile, add `void *monitor_routine(void *data);` to the header, and create+join the monitor thread in `main.c`. This is tracked again in Part 6.

### ASCII dependency diagram — who calls into whom

"Calls into" means "uses a function defined in." An arrow `A ──▶ B` means file A calls a function that lives in file B.

```
                         ┌──────────────┐
                         │  codexion.h  │  (included by ALL — the shared contract)
                         └──────────────┘
                                ▲
        every .c file does  #include "codexion.h"

   ┌────────┐   parse_args    ┌──────────┐
   │ main.c │ ───────────────▶│ parser.c │
   │        │                 └──────────┘
   │        │   init_sim / cleanup   ┌────────┐
   │        │ ──────────────────────▶│ init.c │
   │        │                        └────────┘
   │        │   pthread_create(coder_routine)   ┌──────────────────┐
   │        │ ─────────────────────────────────▶│ coder_routine.c  │
   └────────┘                                   └──────────────────┘
                                                   │        │
                              log_state │          │        │ precise_wait / should_stop
                                        ▼          │        ▼
                                   ┌────────┐      │   ┌───────────────┐
                                   │ sim.c  │      │   │ utils_time.c  │
                                   └────────┘      │   └───────────────┘
                                        │          │        ▲
                                        └── now_in_ms ───────┘  (sim.c also calls now_in_ms)

   ┌───────────┐  (SHOULD be created by main, currently is NOT)
   │ monitor.c │ ·····▶ reads coder state, calls should_stop / now_in_ms, flips stop
   └───────────┘
```

### Every struct, field by field

A **struct** (short for "structure") is a custom box that groups several related variables under one name. A **field** (also called a "member") is one variable inside that box.

#### 1. `t_scheduler` (an **enum** — a named list of fixed choices)

```c
typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;
```
An **enum** (enumeration) is a type whose value can only be one of a few named options. Here the only legal values are `FIFO` (first-in-first-out ordering) and `EDF` (earliest-deadline-first ordering). Internally `FIFO` is just the number 0 and `EDF` is 1, but the names make the code readable.

- Who touches it: `parse_scheduler` in `parser.c` sets it from the word `"fifo"` or `"edf"`.
> **⚠️ GAP / TO VERIFY:** the scheduler choice is **stored but not used anywhere** yet. No code behaves differently for FIFO vs EDF right now. That is planned later work.

#### 2. `t_codex` — the configuration (the 8 settings you pass on the command line)

```c
typedef struct s_codex
{
	int			coders;                        // how many coder threads
	long		time_to_bo;                    // ms allowed before burnout
	long		time_to_compile;               // ms a compile takes
	long		time_to_debug;                 // ms a debug takes
	long		time_to_refactor;              // ms a refactor takes
	int			number_of_compiles_required;   // 0 = run forever; N = stop when all did N
	long		dongle_cooldown;               // ms a dongle must rest after use
	t_scheduler	scheduler;                     // FIFO or EDF
}	t_codex;
```
- `coders`: the number of threads/dongles to make. **In the sample run it is 3.**
- `time_to_bo`: "bo" = **burnout**. If a coder goes this many milliseconds (ms, a thousandth of a second) without starting a compile, it is considered burned out. Sample: 800.
- `time_to_compile` / `time_to_debug` / `time_to_refactor`: how long each phase lasts. Sample: 200 / 200 / 200.
- `number_of_compiles_required`: if greater than 0, the sim should stop once every coder has compiled this many times. If 0, there is no quota and it runs until burnout. Sample: 0.
- `dongle_cooldown`: a rest period a dongle must sit idle after being used. ⚠️ **GAP:** stored, not used yet. Sample: 0.
- `scheduler`: FIFO or EDF (see gap above).
- Who touches it: filled by `assign_numbers` + `parse_scheduler` in `parser.c`; read everywhere via `coder->sim->cfg`.

#### 3. `t_dongle` — one shared lock (a "fork" in the philosophers problem)

```c
typedef struct s_dongle
{
	int				id;             // 1-based label for printing/debugging
	pthread_mutex_t	lock;           // THE lock: holding it = holding the dongle
	pthread_cond_t	cond;           // a "waiting room" primitive (declared, unused)
	int				available;      // 1 if free, 0 if held (declared, unused)
	long			available_at;   // earliest ms it may be taken again (cooldown; unused)
	struct s_sim	*sim;            // back-pointer to the whole world
}	t_dongle;
```
- `id`: a number (1..N) only used for human-readable identification.
- `lock`: a **`pthread_mutex_t`** — the actual mutex (lock). A coder "has the dongle" precisely when it holds this lock. This is the one field that truly matters today.
- `cond`: a **`pthread_cond_t`** (condition variable — a mechanism to make a thread sleep until another thread signals it). ⚠️ **GAP:** declared, never used. Planned for the "waiting queue" feature.
- `available`, `available_at`: ⚠️ **GAP:** declared, never read or written. Intended for the cooldown feature.
- `sim`: a **pointer** (a variable holding the memory address of something else) back to the master `t_sim`, so a dongle can reach shared data. Set, but not currently read.
- Who touches it: the `lock` is locked/unlocked in `coder_routine.c`; initialized in `init.c`.

#### 4. `t_coder` — one worker thread's data

```c
typedef struct s_coder
{
	int				id;                  // 1-based label printed in logs
	pthread_t		thread;              // the OS thread handle
	long			last_compile_start;  // ms (sim-relative) of last compile start
	long			compiles_done;       // how many compiles finished
	pthread_mutex_t	state_lock;          // guards the two fields above
	int				left;                // index of the left dongle
	int				right;               // index of the right dongle
	struct s_sim	*sim;                // back-pointer to the whole world
}	t_coder;
```
- `id`: 1..N, for printing ("coder 2 is compiling").
- `thread`: a **`pthread_t`** — a handle (an ID the system gives you) for the real operating-system thread this coder runs on. Filled by `pthread_create`.
- `last_compile_start`: the timestamp when this coder most recently **began** a compile. The monitor is meant to compare `now - last_compile_start` against `time_to_bo` to detect burnout.
- `compiles_done`: a running count, incremented each time a compile finishes. The monitor is meant to read this to check the quota.
- `state_lock`: a mutex that protects `last_compile_start` and `compiles_done`, because one thread (the coder) writes them while another (the monitor) reads them.
- `left`, `right`: the **array indices** (position numbers, starting at 0) of the two dongles this coder needs. Set in `init.c`.
- `sim`: back-pointer to the shared world — this is how a thread, given only its own `t_coder`, can reach the locks and config.
- Who touches it: `init.c` sets ids/left/right/sim; `coder_routine.c` reads/writes the state fields; the monitor is meant to read them.

#### 5. `t_sim` — the whole world in one box

```c
typedef struct s_sim
{
	t_codex			cfg;          // the 8 settings
	long			start_time;   // absolute ms when the sim began (time "zero")
	t_dongle		*dongles;     // array of N dongles
	t_coder			*coder;       // array of N coders
	pthread_mutex_t	print_lock;   // serializes all printing
	pthread_mutex_t	sim_lock;     // guards the stop flag
	int				stop;         // 0 = keep going, 1 = everyone stop
	long			seq_counter;  // a counter (declared; unused today)
	pthread_t		monitor;      // the monitor thread handle (unused today)
}	t_sim;
```
- `cfg`: the parsed configuration (the `t_codex` above).
- `start_time`: the absolute time the sim began, captured once. Every printed timestamp is `now_in_ms() - start_time`, so logs start near 0.
- `dongles`: a pointer to a heap-allocated array of N `t_dongle`. (**Heap** = the pool of memory you request at runtime with `malloc` and must return with `free`.)
- `coder`: pointer to the heap-allocated array of N `t_coder`.
- `print_lock`: the mutex every `log_state` call locks, so two threads never print at the same instant and scramble a line.
- `sim_lock`: the mutex guarding `stop`.
- `stop`: the **flag** (a single variable used as an on/off switch) that tells all threads to end. 0 = run, 1 = stop. Written by the monitor (eventually), read by every coder each loop via `should_stop`.
- `seq_counter`: ⚠️ **GAP:** declared, never used.
- `monitor`: the `pthread_t` handle for the monitor thread. ⚠️ **GAP:** never filled, because the monitor is not created yet.
- Who touches it: created on `main`'s stack; passed by address everywhere; this single struct IS the shared state.

**If I deleted this file?** Delete `codexion.h` and nothing compiles — every `.c` depends on it. Delete `init.c` and there is no world to run. Delete `coder_routine.c` and the threads have nothing to do. Delete `sim.c` and nothing prints. Delete `monitor.c` today and **nothing changes**, because it is not wired in (that is the gap).

---

# PART 2 — THREADS, FROM ABSOLUTE ZERO

### a) What a running program is. What a process is.

**Analogy:** A recipe printed on paper is just text. When a cook actually stands in a kitchen and starts following it — using counter space, ingredients, a timer — *that living activity* is different from the paper.

**Machine:** Your source code compiled into the `codexion` binary is the "paper." When you run it, the operating system (**OS** — the master program that manages the computer, e.g. Linux) creates a **process**: a live instance of the program with its own private memory, its own open files, and at least one stream of execution. The paper is the binary; the cooking is the process.

**Real example:** Typing `./codexion 3 800 200 200 200 0 0 fifo` makes the OS create one process. Inside it, `main` in `main.c` starts running at line 25.

**Where the analogy breaks:** a kitchen cook is one body; a process can split its cooking into several simultaneous hands — that is threads, next.

### b) What a thread is, and how it differs from a process

**Analogy:** One shared kitchen (the process) can have several cooks (threads) working at the same time. They share the same fridge, counters, and oven (memory), but each cook has their own hands and their own place in the recipe (their own current step).

**Machine:** A **thread** is one independent stream of execution inside a process. A process can contain many threads. All threads of a process **share the same memory**, but each has its **own stack** (its private scratch space for local variables and "where am I in the code" bookkeeping).

**Difference from a process:** two separate processes do NOT share memory — they are separate kitchens. Two threads in one process DO share memory — same kitchen, many cooks. Codexion uses **one process, many threads**, which is exactly why locks matter: the cooks keep reaching for the same oven.

**Real example:** `main.c` line 36 calls `pthread_create` once per coder, creating 3 coder threads. All 3 run `coder_routine` at the same time, all reading the same `sim` struct in the same memory.

### c) What threads share, what each owns privately — draw the memory

**Analogy:** Shared: the fridge, the oven, the sign-up sheet on the wall. Private: each cook's own notepad in their pocket.

**Machine:** Threads share the **heap** (runtime-allocated memory) and **globals**. Each thread has its **own stack** and its **own CPU registers** (tiny fast storage inside the processor).

```
   ONE PROCESS ( ./codexion )
   ┌──────────────────────────────────────────────────────────┐
   │  SHARED by all threads (heap + the sim struct):            │
   │     t_sim sim ─ cfg, start_time, stop, print_lock,         │
   │     sim_lock, dongles[0..2], coder[0..2]                   │
   │     (coder[] and dongles[] are malloc'd on the heap)       │
   ├───────────────┬───────────────┬───────────────┬───────────┤
   │  main stack   │ thread1 stack │ thread2 stack │ thread3 st│
   │  (sim, i)     │ coder,first,  │ coder,first,  │ coder,... │
   │               │ second        │ second        │           │
   └───────────────┴───────────────┴───────────────┴───────────┘
   PRIVATE to each thread: its own stack (its own first/second)
```

**Real example:** In `coder_routine.c` lines 41–43, each coder thread has its own local `coder`, `first`, `second` on its own stack — three separate copies. But all three reach the *same* `sim->dongles[0].lock` in shared memory, which is why they can block each other.

**Where the analogy breaks:** real threads can be moved between CPU cores by the OS at any instant, mid-step, with no warning — a cook who can be teleported to another counter between two knife cuts. That unpredictability is the whole reason races exist.

### d) Why threads at all, in THIS project

**Analogy:** If the problem is literally "several coders working at the same time and fighting over shared dongles," you cannot model that with one cook doing everything in sequence — the fight only exists if they are simultaneous.

**Machine:** The simulation's entire point is **concurrency** (things happening at the same time) and **contention** (many wanting the same resource). Threads give real simultaneity and real blocking, so the deadlock/race/burnout problems are genuine, not faked.

**Real example:** In the sample output, at timestamp `201` coder 1 starts compiling at the very moment coder 2 moves to debugging — two coders genuinely active at once. One thread doing it sequentially could never produce that overlap.

### e) The real thread functions we use

Every signature below is the real POSIX (a Unix standard) `pthread` ("POSIX thread") function. We only use two of the three the prompt mentions; I note that honestly.

#### `pthread_create`
```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
```
- `thread`: address where the new thread's handle will be stored.
- `attr`: thread options; `NULL` means "defaults."
- `start_routine`: the function the new thread will run. Its type is "a function taking a `void *` and returning a `void *`." (`void *` = "pointer to anything.")
- `arg`: the single argument handed to that function.
- **Returns** 0 on success, or an error number (not -1) on failure. On failure no thread is created.
- **Our real call** (`main.c:36`): `pthread_create(&sim.coder[i].thread, NULL, coder_routine, &sim.coder[i]);` — store the handle in that coder's `thread` field, default options, run `coder_routine`, hand it the address of that coder.
> **⚠️ GAP / TO VERIFY:** the return value is **not checked** in `main.c`. If creation failed mid-loop, the code would still try to join a thread that was never created. Robust code checks it.

#### `pthread_join`
```c
int pthread_join(pthread_t thread, void **retval);
```
- `thread`: the handle of the thread to wait for.
- `retval`: where to store that thread's return value; `NULL` means "I don't want it."
- **Behaviour:** the calling thread **blocks** (stops and waits) until that thread finishes. Then it reclaims the thread's resources (prevents a leak of thread bookkeeping).
- **Returns** 0 on success, error number otherwise.
- **Our real call** (`main.c:40`): `pthread_join(sim.coder[i].thread, NULL);` — main waits for each coder in turn, ignoring return values.

#### `pthread_detach`
We do **not** use `pthread_detach` anywhere. (It would mark a thread so its resources are auto-released on exit without a join.) Mentioned only so you know it exists and that we chose join instead — join lets `main` wait for a clean, ordered shutdown.

#### `pthread_mutex_init` / `_lock` / `_unlock` / `_destroy`
```c
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```
- `init`: prepare a mutex for use. `attr = NULL` = defaults. **Our call:** `init.c:65` and nearby.
- `lock`: take the mutex; if someone else holds it, **block until free**. **Our call:** `coder_routine.c:19`.
- `unlock`: release it so a waiter can take it. **Our call:** `coder_routine.c:32`.
- `destroy`: free the mutex's resources when done. **Our call:** `init.c:23`.
- All return 0 on success.

#### `usleep` and `gettimeofday` (not pthread, but part of our threading timing)
- `usleep(useconds_t usec)`: sleep this thread for that many **microseconds** (millionths of a second). **Our call:** `utils_time.c:29`, `usleep(200)`.
- `gettimeofday(struct timeval *tv, ...)`: read the wall clock. **Our call:** `utils_time.c:19`, wrapped by `now_in_ms`.

### f) RACE CONDITION — built from our own data

**Analogy:** Two cooks share one wall tally of "cakes baked." Each wants to add 1. Cook A reads "5," gets distracted; Cook B reads "5," writes "6"; Cook A wakes up, writes "6." Two cakes were baked, the wall says 6, not 7. One cake vanished.

**Machine:** A **race condition** (or **data race**) is when two threads touch the same memory at the same time and at least one is writing, with no lock — so the result depends on the exact, unpredictable timing. `compiles_done++` is not one instruction; it is **read, add 1, write back**, and those three steps can interleave.

**Real example — `compiles_done++` from `coder_routine.c:35` WITHOUT its lock.** Suppose two coders somehow shared one `compiles_done` (imagine a shared counter), starting at 5:

| Step | Coder 1 does | Coder 2 does | value in memory |
|---|---|---|---|
| 1 | read compiles_done → sees 5 | | 5 |
| 2 | | read compiles_done → sees 5 | 5 |
| 3 | add 1 → 6 (in its register) | | 5 |
| 4 | | add 1 → 6 (in its register) | 5 |
| 5 | write 6 back | | 6 |
| 6 | | write 6 back | 6 |

Two increments happened; the counter should read 7; it reads **6**. One compile silently lost. If the monitor needs "every coder did N compiles" to stop, a lost count could mean the sim never stops, or stops wrong.

**The fix that is actually in the code** (`coder_routine.c:34–36`):
```c
pthread_mutex_lock(&coder->state_lock);
coder->compiles_done++;
pthread_mutex_unlock(&coder->state_lock);
```
With the lock held, steps 1–5 for one thread finish completely before the other can start, so the result is always 7. (In our real code each coder has its *own* `compiles_done`, but the monitor reads it from another thread, so the same lock is still required against that reader.)

### g) MUTEX — what physically happens

**Analogy:** One bathroom, one physical key, five people. To enter you must hold the key. If the key is gone, you stand at the door and wait. When the holder comes out and hands the key over, the next person goes in. Only one inside, ever.

**Machine:** A **mutex** ("mutual exclusion") is a lock with exactly two states: free or held-by-one-thread. `pthread_mutex_lock` on a free mutex takes it and continues. On a held mutex, the OS **puts the calling thread to sleep** (it uses no CPU) and wakes it when the mutex is released. `pthread_mutex_unlock` releases it and also **publishes memory**: changes made while holding it become visible to the next locker.

**Real example — what each lock protects:**

| Lock | Lives in | Protects | Why |
|---|---|---|---|
| `dongles[i].lock` | each dongle | the right to "hold" dongle i | two coders must never hold the same dongle |
| `coder[i].state_lock` | each coder | `last_compile_start`, `compiles_done` | coder writes them; monitor reads them |
| `print_lock` | sim | stdout (the output stream) | two prints must not scramble into one line |
| `sim_lock` | sim | `stop` | monitor writes it; all coders read it |

The locking in `take_dongles` (`coder_routine.c:19–22`): lock `first`, log, lock `second`, log. If `second` is held by a neighbour, this thread sleeps at line 21 until the neighbour unlocks it — that is a coder waiting its turn for a shared dongle.

### h) DEADLOCK — build one, then prevent it

**Analogy:** Four people at a round table, one fork between each pair, and a rule "pick up your left fork, then your right." Everyone grabs their left at once. Now everyone holds one fork and waits forever for the right one, which their neighbour is holding. Nobody eats. Frozen.

**Machine:** A **deadlock** is a cycle of threads each holding a lock the next one needs. It needs four things at once (**Coffman conditions**): mutual exclusion, hold-and-wait, no preemption (you can't force a lock away), and **circular wait**. Break any one and deadlock is impossible.

**The broken version (do NOT do this):** every coder locks `left` then `right`:
```c
// BROKEN: everyone same order
lock(dongles[coder->left].lock);
lock(dongles[coder->right].lock);
```
With 3 coders, the frozen state:
```
coder 1 holds dongle 0, waits for dongle 1
coder 2 holds dongle 1, waits for dongle 2
coder 3 holds dongle 2, waits for dongle 0   ← cycle closes → DEADLOCK
```
Symptom you would see: the program prints a few "has taken a dongle" lines, then **total silence forever**, 0% CPU. It is not crashed; it is frozen.

**The real fix in the code — `set_order` (`coder_routine.c:3–15`):** always lock the **lower-numbered** dongle first, whoever you are.
```c
if (coder->left < coder->right) { *first = coder->left;  *second = coder->right; }
else                            { *first = coder->right; *second = coder->left;  }
```
Why it works: coder 3's natural order is left=2, right=0, but `2 < 0` is false, so it takes **0 first**. Now dongle 0 is contested by coder 1 and coder 3; only one wins; the loser holds nothing, so dongle 2 stays free, coder 2 proceeds, and the cycle can never close. This kills the **circular wait** condition. It is **deadlock prevention** (making the bad state unreachable), not detection-and-recovery.

**Real example it is working:** the sample run prints steadily forever with no silent freeze — three coders cycling, which only happens because no deadlock forms.

### i) Why this design, and what we rejected

- **Lock ordering (chosen).** Cost: you must be disciplined — every lock of a pair must use the same global order. Benefit: zero deadlock, no timeouts, no retries, no wasted CPU. This is what the code does (`set_order`).
- **Grab-both-or-release (rejected): `trylock`, and if the second fails, drop the first and retry.** Cost: **livelock** risk (threads politely stepping aside forever) and busy-looping that burns CPU. Benefit: no fixed ordering needed. We did not take it.
- **One global lock around all compiling (rejected).** Cost: kills concurrency — only one coder could ever compile, defeating the entire point. Benefit: trivially deadlock-free. Rejected because it makes the simulation meaningless.
- **One waiter/arbiter thread handing out dongle pairs (rejected for now).** Cost: more complexity, a bottleneck. This is closer to the planned FIFO/EDF scheduler work, not the current design.

---

# PART 3 — EVERY COMMAND WE USE

Ordered as a real working day: build → run → test → debug → commit.

### 1. The raw compiler line (what `make` runs for you)
```
cc -Wall -Wextra -Werror -pthread -c main.c -o objs/main.o
```
- `cc`: the **C compiler** (the tool that turns `.c` text into machine code). On this system it is the system compiler.
- `-Wall`: turn on **all** the common **W**arnings (messages about suspicious code).
- `-Wextra`: turn on **extra** warnings beyond `-Wall`.
- `-Werror`: treat every **W**arning as an **error** — the build fails if there is even one warning. This forces clean code (a 42 requirement).
- `-pthread`: link the **POSIX thread** library and set thread-safe flags. Without it, `pthread_create` and friends will not link.
- `-c`: **compile only** — produce an object file (`.o`), do not yet make the final program.
- `main.c`: the input source file.
- `-o objs/main.o`: `-o` names the **o**utput file.
- **Correct output:** no output at all, and an `objs/main.o` file appears. Silence is success.
- **Failure output:** something like `main.c:36:9: error: ...` — read it as `file:line:column: error: message`. Go to that file and line.
- **When to reach for it:** almost never by hand — `make` does it. Useful to understand what `make` is really doing.

### 2. `make` — build the program
```
make
```
- `make`: reads the `Makefile` and builds the default target, which is `all` → `codexion`.
- It compiles each changed `.c` into `objs/*.o`, then links them into the `codexion` binary.
- The final link line (from the Makefile) is `cc -Wall -Wextra -Werror -pthread objs/*.o -o codexion` (**link** = combine object files into one executable).
- **Correct output:** a list of `cc ... -c ...` lines, then one link line, then back to the prompt, with a `codexion` file present.
- **Failure output:** a compiler error (see above), and `make` stops. Fix the first error first; later ones are often caused by it.
- **When:** every time you change code and want to run it.

### 3. `make re`, `make clean`, `make fclean` — the other targets
```
make clean      # delete objs/ (the .o files)
make fclean     # clean + delete the codexion binary
make re         # fclean + all  → a guaranteed-fresh full rebuild
```
- `clean`: removes the `objs/` folder. Source untouched.
- `fclean`: "full clean" — also removes the `codexion` executable.
- `re`: "rebuild" — wipe everything and build from scratch, so no stale object file can hide a bug.
- **Correct output:** `rm -rf objs` etc., then (for `re`) a full compile. **When:** use `re` when something feels wrong and you want certainty it was truly rebuilt.

### 4. Running the program
```
./codexion 3 800 200 200 200 0 0 fifo
```
- `./codexion`: run the executable in the current folder (`./` means "right here").
- The 8 numbers/words in order: `coders=3`, `time_to_bo=800`, `time_to_compile=200`, `time_to_debug=200`, `time_to_refactor=200`, `number_of_compiles_required=0`, `dongle_cooldown=0`, `scheduler=fifo`.
- **Correct output:** lines like `201 2 is compiling` — `timestamp_ms coder_id status`.
- **Failure output:** `Error: expected 8 arguments` (wrong count), `Error: args must be positive integers` (bad number), or `Error: scheduler must be fifo or edf`.
> **⚠️ GAP / TO VERIFY:** because the monitor is not wired in, this command **never stops on its own** today. You must press `Ctrl-C` (send an interrupt that kills the process) to end it. In a terminal the output also looks "stuck" for a second because stdout is **line-buffered only to a terminal**; piped through other tools it buffers in 4 KB blocks.

### 5. `valgrind` — find memory leaks
```
valgrind --leak-check=full ./codexion 3 800 200 200 200 0 2 fifo
```
- `valgrind`: a tool that runs your program in a sandbox and watches every memory operation.
- `--leak-check=full`: report each leak in detail, with the line that allocated the lost memory.
- **Correct output:** `All heap blocks were freed -- no leaks are possible`.
- **Failure output:** `definitely lost: X bytes in Y blocks`, with a stack showing `malloc` in `init.c`. That means something you `malloc`'d was never `free`'d.
- **When:** before saying "done." Note: you need a run that actually **terminates**, so this is only meaningful once the monitor/quota stop exists (or you use a tiny quota). Today, with no stop, you would have to Ctrl-C, which muddies results.

### 6. `valgrind --tool=helgrind` — find data races and lock-order problems
```
valgrind --tool=helgrind ./codexion 3 800 200 200 200 0 2 fifo
```
- `--tool=helgrind`: switch Valgrind to **Helgrind**, which watches threads and locks.
- It flags: memory touched by two threads without a common lock (a **data race**), and locks taken in inconsistent orders (a **potential deadlock**).
- **Correct output:** `ERROR SUMMARY: 0 errors`.
- **Failure output:** `Possible data race during read/write ...` with two stacks, or `lock order "A before B" violated`. The lock-order warning is exactly what `set_order` is designed to avoid.
- **When:** this is the headline test for a concurrency project — it proves your locking is correct, not just lucky.

### 7. `gdb` — step through and inspect
```
gdb ./codexion
```
- `gdb`: the **GNU debugger**, lets you pause the program, inspect variables, see where each thread is.
- Useful commands inside it: `run 3 800 200 200 200 0 0 fifo`, `Ctrl-C` to pause, `info threads` (list threads), `thread 2` (switch), `bt` ("backtrace" — the call stack).
- **When a deadlock happens**, pause and `info threads`: you will see every thread stuck in `pthread_mutex_lock`. That is the fingerprint of deadlock.
> **⚠️ GAP / TO VERIFY:** the Makefile does **not** pass `-g` (the flag that embeds debug info: variable names and line numbers). `gdb` still works but shows less detail. Add `-g` to `CFLAGS` while debugging for names and line numbers, remove it for the final build.

### 8. Git — the workflow
```
git status                 # what changed since last commit
git diff                   # the exact line-by-line changes
git add -A                 # stage all changes for the next commit
git commit -m "message"    # save a snapshot with a message
git log --oneline          # the history, one line per commit
```
- `git`: a **version control** tool (it remembers every saved snapshot of your code).
- `status`: shows modified/new files. `diff`: shows what changed inside them.
- `add`: choose what goes into the next snapshot. `commit`: save it with a note.
- `log --oneline`: the list of past snapshots.
- **When:** commit after each working step, with a message that says what you did.
> **⚠️ GAP / TO VERIFY:** the current history's messages are mostly `"dasd"` / `"ads"` — not meaningful. Also `objs/` and the `codexion` binary are committed in some snapshots; generated files normally should be ignored via a `.gitignore` and not committed.

---

# PART 4 — THE EXECUTION, STAGE BY STAGE

The full journey from pressing Enter to the output. Each stage: real code, what it does, machine state after.

### Stage 1 — Arguments arrive and are validated (`parser.c`)

You type `./codexion 3 800 200 200 200 0 0 fifo`. The OS hands `main` two things: `argc = 9` (the count, including the program name) and `argv`, an array of 9 strings. Everything is still **text**: `"3"` is characters, not the number 3.

```c
// main.c:30
if (!parse_args(argc, argv, &sim.cfg))
	return (1);
```
Inside `parse_args` (`parser.c:67`):
```c
if (argc != 9)                              // need program + 8 args
	return (fprintf(stderr, "Error: expected 8 arguments\n"), 0);
if (!validate_numbers(argv))                // argv[1..7] digits only
	return (fprintf(stderr, "Error: args must be positive integers\n"), 0);
assign_numbers(cfg, argv);                  // text -> numbers via atoi
if (!parse_scheduler(argv[8], &cfg->scheduler))  // "fifo"/"edf" -> enum
	return (fprintf(stderr, "Error: scheduler must be fifo or edf\n"), 0);
if (cfg->coders < 1)
	return (fprintf(stderr, "Error: need at least one coder\n"), 0);
```
- `validate_numbers` (`parser.c:31`) checks `argv[1]`..`argv[7]` with `is_positive_integer`, which rejects empty strings, anything longer than 10 characters (an overflow guard), and any character that is not `0`–`9`. So `-5` is rejected (the `-` is not a digit), `3a` is rejected.
- `assign_numbers` (`parser.c:45`) calls `atoi` ("ASCII to integer," convert text to a number) on each, filling `cfg`.
- `parse_scheduler` (`parser.c:56`) maps `"fifo"`→`FIFO`, `"edf"`→`EDF`, anything else → reject.
- **`stderr`** is the error output stream (separate from normal output), used so errors don't mix with real results.

**What gets rejected and why:** wrong count (not exactly 8 args), any non-digit numeric arg, a scheduler that is not fifo/edf, or fewer than 1 coder.

**Machine state after:** `sim.cfg` holds real numbers: coders=3, time_to_bo=800, etc. If anything was invalid, the program already returned 1 and stopped.

### Stage 2 — Build the world (`init.c`, `init_sim`)

```c
// main.c:32
if (!init_sim(&sim))
	return (1);
```
```c
// init.c:29
sim->coder   = malloc(sizeof(t_coder)  * sim->cfg.coders);  // array of 3 coders
sim->dongles = malloc(sizeof(t_dongle) * sim->cfg.coders);  // array of 3 dongles
if (!sim->coder || !sim->dongles)
	return (free(sim->dongles), free(sim->coder), 0);        // no leak on failure
memset(sim->coder,   0, sizeof(t_coder)  * sim->cfg.coders); // zero everything
memset(sim->dongles, 0, sizeof(t_dongle) * sim->cfg.coders);
```
- `malloc` ("memory allocate") asks the OS for a block of heap memory and returns its address, or `NULL` if it failed.
- `sizeof(t_coder)` is the byte size of one coder struct, times 3 = room for 3.
- `memset(..., 0, ...)` fills the block with zero bytes, so every field starts at a known 0 (important: `compiles_done=0`, `last_compile_start=0`).

Then the setup loop (`init.c:40–48`):
```c
sim->coder[counter].id   = counter + 1;              // ids 1,2,3
sim->coder[counter].sim  = sim;                      // back-pointer
sim->dongles[counter].id = counter + 1;
sim->dongles[counter].sim = sim;
sim->coder[counter].left  = counter;                 // left dongle index
sim->coder[counter].right = (counter + 1) % sim->cfg.coders; // right, wraps
```
The `% sim->cfg.coders` ("modulo" — the remainder after division) is what makes the ring: coder 3 (index 2) gets `right = (2+1) % 3 = 0`, wrapping back to dongle 0. That wrap is the source of the deadlock risk `set_order` later neutralizes.

```
coder[0] (id1): left=0 right=1
coder[1] (id2): left=1 right=2
coder[2] (id3): left=2 right=0   ← ring closes here
```

Then mutexes and the clock (`init.c:49–53`):
```c
if (!init_mutexes(sim))                 // init every lock, rollback on failure
	return (free(sim->coder), free(sim->dongles), 0);
sim->stop = 0;
sim->seq_counter = 0;
sim->start_time = now_in_ms();          // time "zero" is set HERE
```
`init_mutexes` (`init.c:58`) initializes each dongle `lock`, each coder `state_lock`, then `print_lock` and `sim_lock`, **before any thread exists**, each checked with rollback so a half-built world is torn down cleanly (no leak).

**Machine state after:** 3 coders and 3 dongles on the heap, every mutex live and free, `stop=0`, the clock started.

> **⚠️ GAP / TO VERIFY — the burnout clock is not primed.** `last_compile_start` is 0 for every coder after `memset`, and `init_sim` never sets it to `start_time`. If you wire in the monitor as-is, it will compute `now - 0`, a huge number, and declare burnout **instantly** on the first check. The fix is to set `sim->coder[counter].last_compile_start = sim->start_time;` in the init loop. Until then, do not claim burnout timing is correct.

### Stage 3 — Create the coder threads (`main.c`)

```c
// main.c:34-36
i = -1;
while (++i < sim.cfg.coders)
	pthread_create(&sim.coder[i].thread, NULL, coder_routine, &sim.coder[i]);
```
`pthread_create` does **not** run `coder_routine` inside `main`. It creates a new OS thread that **immediately begins running `coder_routine` on its own**, and returns to `main` right away. So this loop fires 3 times in microseconds, and now 4 things run at once: `main`, plus coder threads 1, 2, 3. Each thread received `&sim.coder[i]` — the address of its own coder struct — which is how it knows who it is.

**Order:** threads are created in index order 0,1,2 (coders 1,2,3). But once created they run in **no guaranteed order** — the OS schedules them however it likes. The sample output shows coder 1 and coder 2 both acting before coder 3 ever appears; that ordering is not promised and can differ run to run.

**Machine state after:** 3 live coder threads executing `coder_routine`; `main` moving to its next loop.

### Stage 4 — The protocol each coder runs (`coder_routine.c`)

```c
// coder_routine.c:39
void  *coder_routine(void *data)
{
	t_coder *coder = (t_coder *)data;     // recover "who am I"
	int first, second;

	set_order(coder, &first, &second);    // compute lock order ONCE (lower first)
	while (!should_stop(coder->sim))      // each pass: still running?
	{
		take_dongles(coder, first, second);                 // lock both dongles
		compile_phase(coder, first, second);                // stamp, compile, release
		log_state(coder, "is debugging");
		precise_wait(coder->sim->cfg.time_to_debug, coder->sim->start_time);
		log_state(coder, "is refactoring");
		precise_wait(coder->sim->cfg.time_to_refactor, coder->sim->start_time);
	}
	return (NULL);
}
```
`set_order` (`coder_routine.c:3`) is computed once, outside the loop, because a coder's two dongles never change. It picks the **lower index as `first`** (the deadlock-prevention rule from Part 2h).

`take_dongles` (`coder_routine.c:17`):
```c
pthread_mutex_lock(&coder->sim->dongles[first].lock);
log_state(coder, "has taken a dongle");
pthread_mutex_lock(&coder->sim->dongles[second].lock);
log_state(coder, "has taken a dongle");
```
Lock the lower dongle, announce it, lock the higher dongle, announce it. If the second is held by a neighbour, the thread **sleeps here** until it is free.

`compile_phase` (`coder_routine.c:25`):
```c
pthread_mutex_lock(&coder->state_lock);
coder->last_compile_start = now_in_ms() - coder->sim->start_time; // stamp start
pthread_mutex_unlock(&coder->state_lock);
log_state(coder, "is compiling");
precise_wait(coder->sim->cfg.time_to_compile, coder->sim->start_time); // ~200ms
pthread_mutex_unlock(&coder->sim->dongles[first].lock);   // release BOTH
pthread_mutex_unlock(&coder->sim->dongles[second].lock);
pthread_mutex_lock(&coder->state_lock);
coder->compiles_done++;                                   // count it
pthread_mutex_unlock(&coder->state_lock);
```
Notice the dongles are released **the instant compiling ends**, before debug and refactor. Debug and refactor hold no dongles — a coder only needs the hardware to compile — so neighbours can grab the freed dongles and work in parallel.

`should_stop` (`utils_time.c:32`) reads `sim->stop` under `sim_lock`. Locking to read one int is not paranoia: without the lock there is no guarantee one thread ever *sees* another thread's write to `stop` (that is the memory-visibility reason in Part 2g).

`precise_wait` (`utils_time.c:23`):
```c
begin = now_in_ms() - start_time;
while ((now_in_ms() - start_time) - begin < duration)
	usleep(200);
```
It spins in 200-microsecond naps, re-checking the clock, until `duration` ms truly passed. A single long `usleep` would oversleep by milliseconds (fatal near a burnout deadline); the short-nap loop lands on target without burning 100% CPU.

**Machine state after one pass:** the coder has compiled once (`compiles_done` up by 1, `last_compile_start` stamped), held then released two dongles, and spent time debugging and refactoring holding nothing.

### Stage 5 — How the program decides to stop

Design intent: the **monitor** thread loops, and when it sees either a burnout (`now - last_compile_start > time_to_bo`) or the full quota met (every coder's `compiles_done >= number_of_compiles_required`), it sets `stop = 1`. Every coder's `while (!should_stop(...))` then sees `stop` is true, finishes its current pass, and returns — ending its thread. That is what lets the joins in `main` return.

The monitor code exists (`monitor.c`):
```c
void *monitor_routine(void *data)
{
	t_sim *sim = (t_sim *)data;
	while (!should_stop(sim))
	{
		i = -1; full = 1;
		while (++i < sim->cfg.coders)
		{
			if (check_burnout(sim, i)) return (NULL);   // prints "burned out", sets stop
			if (!reached_quota(sim, i)) full = 0;
		}
		if (sim->cfg.number_of_compiles_required > 0 && full)
			return (set_stop(sim), NULL);               // quota met -> stop
		usleep(1000);                                   // check ~every 1 ms
	}
	return (NULL);
}
```
`check_burnout` reads `last_compile_start` under `state_lock`, and if overdue, locks `print_lock`, sets stop, prints `burned out`, unlocks. The lock order is always `print_lock → sim_lock`, never reversed, so the monitor cannot deadlock against anything.

> **⚠️ GAP / TO VERIFY — the stop mechanism is not active.**
> 1. `monitor_routine` is never created in `main.c`, so **nothing ever sets `stop`**. Today the program runs forever.
> 2. Even wired, burnout would misfire until `last_compile_start` is primed to `start_time` (Stage 2 gap).
> 3. `log_state` in `sim.c` prints **unconditionally** — it does not check `stop` while holding `print_lock`. So a coder could print `is debugging` *after* the monitor printed `burned out`. The intended fix is for `log_state` to check `should_stop` under `print_lock` and skip printing if stopped.
> Do not claim burnout/quota termination works today. It is built but not connected.

### Stage 6 — Termination, join, cleanup (the correct order)

```c
// main.c:37-41
i = -1;
while (++i < sim.cfg.coders)
	pthread_join(sim.coder[i].thread, NULL);   // wait for each coder to finish
cleanup(&sim);                                 // destroy locks, free memory
```
`main` blocks on `pthread_join(coder 1)` until coder 1 returns, then coder 2, then coder 3. Only after *all* threads have ended does `cleanup` run.

`cleanup` (`init.c:83`):
```c
destroy_mutexes(sim, sim->cfg.coders);   // destroy dongle locks + state locks
pthread_mutex_destroy(&sim->print_lock);
pthread_mutex_destroy(&sim->sim_lock);
free(sim->coder);
free(sim->dongles);
```
**Order matters, and here is what breaks if you get it wrong:**
- **Free before join** → a still-running thread touches freed memory = **use-after-free** (reading/writing memory you already returned), a crash or garbage.
- **Destroy a mutex a thread still holds** → undefined behaviour; `pthread_mutex_destroy` on a locked mutex is illegal.
- **Free the arrays before destroying their mutexes** → you destroy locks living in memory you already freed = use-after-free again.
So the only safe order is: **join all threads → destroy mutexes → free arrays.** That is exactly what the code does.

> **⚠️ GAP / TO VERIFY:** because no thread ever ends today (Stage 5 gap), `main` blocks on `pthread_join(coder 1)` **forever** and `cleanup` is never reached. The teardown code is correct; it just cannot run until stop works.

### A real sample run, annotated line by line

Command (capped at ~1 second because it never ends on its own today):
```
timeout 1 stdbuf -oL ./codexion 3 800 200 200 200 0 0 fifo
```
Actual captured output:
```
1 1 has taken a dongle
1 2 has taken a dongle
1 2 has taken a dongle
1 2 is compiling
201 2 is debugging
201 1 has taken a dongle
201 1 is compiling
401 2 is refactoring
401 1 is debugging
401 3 has taken a dongle
401 3 has taken a dongle
401 3 is compiling
601 3 is debugging
601 1 is refactoring
601 2 has taken a dongle
601 2 has taken a dongle
601 2 is compiling
801 2 is debugging
```
Every line is `timestamp_ms coder_id status`, produced by `log_state` (`sim.c:9`). Reading it:

- `1 1 has taken a dongle` — at 1 ms, coder 1 locked its `first` dongle (index 0). `take_dongles`, first lock.
- `1 2 has taken a dongle` / `1 2 has taken a dongle` — coder 2 locked **both** its dongles (indices 1 then 2). It got both because nobody else wanted them yet.
- `1 2 is compiling` — coder 2 enters `compile_phase`, stamps `last_compile_start`, begins its 200 ms compile.
- `201 2 is debugging` — 200 ms later, coder 2 finished compiling, **released dongles 1 and 2**, and moved to debugging.
- `201 1 has taken a dongle` — now that dongle 1 is free, coder 1 (who was holding dongle 0 and waiting at its second lock) grabs dongle 1 and has both.
- `201 1 is compiling` — coder 1 compiles.
- `401 2 is refactoring` — coder 2's 200 ms debug ended; it refactors (holding nothing).
- `401 1 is debugging` — coder 1's compile ended; it released dongles 0 and 1 and debugs.
- `401 3 has taken a dongle` ×2 — dongle 0 is free again, so coder 3 (needs 0 and 2) grabs both.
- `401 3 is compiling` — coder 3 compiles.
- `601 ...` and `801 ...` — the same rhythm continues: each 200 ms boundary, a phase ends and dongles change hands.

Notice the timestamps step in 200 ms beats because every phase is 200 ms in this run, and notice work genuinely overlaps (at 201, coder 1 compiles while coder 2 debugs) — real concurrency, deadlock-free.

---

# PART 5 — THE HISTORY: HOW WE GOT HERE

Reconstructed from `git log --reverse`. **Honest caveat:** the commit messages are mostly placeholders (`"dasd"`, `"ads"`, `"finish"`), so the *story* below is read from the **file changes** in each commit, not the messages. The most recent work (Day 3 cycle, monitor) is **uncommitted** — it shows only in `git status`.

### Sept 14 — `c0b4107` "dasd" (the seed)
First commit: a `codexion.c` plus a 16-line `codexion.h`. Just a skeleton — a `main` that does almost nothing and the first type definitions. Lesson: start with something that compiles.

### Sept 16 — `55944ed` "dasd" (shaping the data)
`codexion.h` grows (+12 lines) and the `Makefile` appears. The structs are taking shape before any behaviour exists — data first, logic later.

### Sept 17 — `1025d73` "parsing" (input handling)
`parser.c` is born (79 lines), `codexion.c` is renamed to `main.c`, the Makefile fills out. This is the whole argument-validation layer. Lesson learned: reject bad input before building anything.

### Sept 17 — `a879289` "parsing" (cleanup of tracked binaries)
Removes committed build artifacts (`codexion`, `*.o`). A small correction: generated files should not really live in git. (This lesson did not fully stick — binaries reappear later.)

### Sept 17 — `2923106` "time_utils" (the clock)
`utils_time.c` (+30 lines): `now_in_ms` and `precise_wait`. Timing is the sim's heartbeat, built before the threads that depend on it.

### Sept 20 — `d70109b` "finish" (the big push to threads)
Large commit: Makefile rewritten, `codexion.h` +57 lines (the real structs — coder, dongle, sim), `init.c` created (+42), `main.c` reworked. This is where the world-building and threading infrastructure lands.

### Sept 20 — `6d4d6aa` "ads" (create + join)
`main.c` gains the `pthread_create` loop and the `pthread_join` loop; `init.c` grows. The threads can now be born and waited on.

### Sept 20 — `a3a298a` "dasd" (robust init)
`init.c` +44 lines: the mutex-init **rollback** logic (destroy what you built if a later init fails). Lesson: partial initialization must clean up after itself or it leaks.

### Sept 24 — `d868112` and `f34b26d` "dasd" (tidying)
Small `init.c` change, then removal of committed binaries again.

### Now — uncommitted (Day 3–4 work)
`git status` shows new, unsaved files: `coder_routine.c` (the compile/debug/refactor loop and `set_order` deadlock prevention), `sim.c` (`log_state`), `monitor.c` (burnout/quota — not yet wired), plus `should_stop` added to `utils_time.c`. This is the current frontier. **Dead end still open:** the monitor is written but not connected (Part 1 and Part 4 gaps).

---

# PART 6 — THE DANGER ZONES

The realistic ways this project breaks. For each: symptom, cause, the command that proves it, the fix.

### 1. Deadlock (frozen, all threads waiting)
- **Symptom:** a few "has taken a dongle" lines, then total silence, 0% CPU, never ends.
- **Cause:** all coders locking in the same left-then-right order → circular wait.
- **Proof:** `valgrind --tool=helgrind ./codexion ...` reports a lock-order violation; or in `gdb`, `info threads` shows every thread in `pthread_mutex_lock`.
- **Fix (already in code):** `set_order` locks the lower-numbered dongle first (`coder_routine.c:3`).

### 2. Data race on coder state
- **Symptom:** wrong `compiles_done` counts; quota never triggers or triggers early (once the monitor is live).
- **Cause:** reading/writing `compiles_done` or `last_compile_start` without `state_lock`.
- **Proof:** `valgrind --tool=helgrind` prints "Possible data race."
- **Fix (in code):** every access is wrapped in `state_lock` (`coder_routine.c:27`, `:34`; `monitor.c:16`, `:27`).

### 3. Scrambled / interleaved output
- **Symptom:** two log lines merged into gibberish on one line.
- **Cause:** two threads calling `printf` at once without serialization.
- **Proof:** visible in output; Helgrind flags a race on stdout.
- **Fix (in code):** `log_state` holds `print_lock` around the whole `printf` (`sim.c:7–10`).

### 4. Memory leak
- **Symptom:** Valgrind says `definitely lost`.
- **Cause:** a `malloc` without a matching `free`, or returning early without freeing.
- **Proof:** `valgrind --leak-check=full ./codexion ...` (needs a run that ends).
- **Fix (in code):** `init_sim` frees on every failure path; `cleanup` frees both arrays. **Blocked today** because the program never terminates (monitor gap), so you cannot get a clean Valgrind run without a quota + wired monitor.

### 5. Use-after-free at shutdown
- **Symptom:** crash or garbage at exit.
- **Cause:** freeing arrays or destroying mutexes while a thread still runs.
- **Proof:** Valgrind "Invalid read/write."
- **Fix (in code):** `main` joins **all** threads before `cleanup` (`main.c:39–41`).

### 6. Wrong teardown order
- **Symptom:** illegal-operation crash on a mutex.
- **Cause:** destroying a mutex still held, or freeing memory a mutex lives in before destroying it.
- **Proof:** Valgrind / a `pthread_mutex_destroy` returning `EBUSY`.
- **Fix (in code):** join → destroy mutexes → free arrays, in that order (`init.c:83`).

### 7. Bad command-line input
- **Symptom:** nonsense behaviour or crash from garbage args.
- **Cause:** trusting input without checking.
- **Proof:** `./codexion 3 abc 200 200 200 0 0 fifo` → `Error: args must be positive integers`.
- **Fix (in code):** `parse_args` + `validate_numbers` reject it before anything is built.

### 8. **Monitor never stops the sim (ACTIVE BUG)**
- **Symptom:** the program runs forever; needs Ctrl-C.
- **Cause:** `monitor_routine` is never created in `main.c`, not in the Makefile, not prototyped.
- **Proof:** `grep monitor main.c Makefile codexion.h` → no `pthread_create`, no `monitor.c`, no prototype.
- **Fix:** add `monitor.c` to Makefile `SRCS`; add `void *monitor_routine(void *data);` to `codexion.h`; in `main.c` create `sim.monitor` before the join loop and join it after.

### 9. **False instant burnout (ACTIVE BUG, latent)**
- **Symptom:** once the monitor is wired, every coder "burns out" at time ~0.
- **Cause:** `last_compile_start` is 0 after `memset` and never primed to `start_time`.
- **Proof:** wire the monitor and run — immediate `burned out`.
- **Fix:** in `init_sim`'s loop, `sim->coder[counter].last_compile_start = sim->start_time;`.

### 10. **Message printed after burnout (ACTIVE BUG, latent)**
- **Symptom:** a coder prints `is debugging` *after* the monitor printed `burned out`.
- **Cause:** `log_state` prints unconditionally; it does not check `stop` under `print_lock`.
- **Proof:** wire the monitor with a short `time_to_bo`; watch for a status line after the burnout line.
- **Fix:** in `log_state`, after locking `print_lock`, if `should_stop(sim)` is true, unlock and return without printing.

---

# PART 7 — GLOSSARY

- **argc / argv** — the count of command-line words and the array holding them, given to `main`. Used in `main.c`/`parser.c`.
- **atoi** — "ASCII to integer," converts a text string to a number. Used in `assign_numbers` (`parser.c`).
- **binary / executable** — the runnable file produced by compiling; here, `codexion`.
- **blocking** — a thread stopping and waiting (using no CPU) until something frees up. Happens at `pthread_mutex_lock` and `pthread_join`.
- **buffer (line/full buffering)** — output held temporarily before being shown; to a terminal it flushes per line, to a pipe in ~4 KB blocks. Why `timeout` showed nothing without `stdbuf`.
- **compile (two meanings)** — (1) translating source to a binary (the build); (2) in the sim, the named phase where a coder holds both dongles for `time_to_compile` ms.
- **concurrency** — multiple streams of work progressing at the same time. The whole point of the project.
- **condition variable (`pthread_cond_t`)** — a tool to make a thread sleep until signalled. Declared in `t_dongle.cond`; unused yet.
- **contention** — many threads wanting the same resource at once. Here, coders wanting shared dongles.
- **Coffman conditions** — the four conditions all required for deadlock; break one to prevent it. Mutual exclusion, hold-and-wait, no preemption, circular wait.
- **data race / race condition** — two threads touching the same memory unsafely, result depends on timing. Prevented by the locks.
- **deadlock** — a cycle of threads each waiting on a lock another holds; frozen forever. Prevented by `set_order`.
- **dongle** — in this project, a `pthread_mutex_t` a coder must hold to compile; a "fork" in philosophers. `t_dongle`.
- **enum** — a type with a small set of named values. `t_scheduler` (FIFO/EDF).
- **flag** — a variable used as an on/off switch. `sim->stop`.
- **free** — return heap memory to the system. In `init_sim` failure paths and `cleanup`.
- **gettimeofday** — reads the wall clock. Wrapped by `now_in_ms` (`utils_time.c`).
- **gdb** — the debugger; pause and inspect threads.
- **handle** — an ID the system gives you to refer to a resource; `pthread_t` for a thread.
- **heap** — runtime-allocated memory from `malloc`, freed with `free`. The coder/dongle arrays live here.
- **Helgrind** — Valgrind's thread-error tool; finds data races and lock-order bugs.
- **index** — a position number in an array, starting at 0. `coder->left`, `coder->right`.
- **init (initialize)** — set something up for first use; `pthread_mutex_init`, `init_sim`.
- **malloc** — "memory allocate," request heap memory; returns an address or NULL.
- **memset** — fill a block of memory with a byte value; used to zero the arrays.
- **modulo (%)** — the remainder of a division; `(counter+1) % coders` makes the ring.
- **mutex (`pthread_mutex_t`)** — a lock only one thread can hold at a time. Dongles, state_lock, print_lock, sim_lock.
- **OS (operating system)** — the master program managing the machine; schedules threads. Linux here.
- **pointer** — a variable holding a memory address. `sim`, `coder`, `data`.
- **POSIX** — a Unix standard; `pthread` is its threading API.
- **precise_wait** — our accurate millisecond wait built from short `usleep` naps. `utils_time.c`.
- **process** — a live instance of a running program with its own memory. One per `./codexion` run.
- **pthread_create / _join** — start a thread / wait for it to finish. `main.c`.
- **quota** — `number_of_compiles_required`; stop when every coder reached it (once monitor is wired).
- **refactor / debug** — named sim phases where a coder holds no dongles and just spends time.
- **scheduler (OS)** — the part of the OS that decides which thread runs when; it is why thread order is unpredictable.
- **scheduler (FIFO/EDF)** — the project's planned ordering policy. Stored, not used yet.
- **stack** — a thread's private memory for local variables. Each coder thread has its own.
- **start_time** — the absolute ms when the sim began; timestamps are measured from it. `sim->start_time`.
- **stderr / stdout** — the error and normal output streams.
- **stop flag** — `sim->stop`; tells all threads to end.
- **struct** — a custom box grouping related variables. `t_sim`, `t_coder`, etc.
- **usleep** — sleep this thread for N microseconds. In `precise_wait`.
- **use-after-free** — touching memory you already freed; a bug. Prevented by the join-before-cleanup order.
- **Valgrind** — a tool that watches memory/threads to find leaks and races.
- **version control (git)** — tool that saves snapshots of code over time.

---

# PART 8 — PROVE I LEARNED IT

Try to answer before looking at the answers that follow.

1. What is the difference between a process and a thread, and which does Codexion use?
2. In `t_dongle`, which single field actually matters today, and what are three fields that are declared but unused?
3. Why does `log_state` lock `print_lock` around its `printf`?
4. Why must `compiles_done++` be done while holding `state_lock`, even though it is "just adding one"?
5. In `set_order`, coder 3 has left=2 and right=0. Which dongle does it lock first, and why does that prevent deadlock?
6. Why are the dongles released at the end of `compile_phase` rather than at the very end of the loop pass?
7. Why does `precise_wait` loop with `usleep(200)` instead of one single long `usleep`?
8. What are the four Coffman conditions, and which one does Codexion attack?
9. **What would happen** if you changed every coder to lock `left` first then `right` (removing `set_order`'s flip)? Describe the exact frozen state for 3 coders.
10. **What would happen** if you called `cleanup` before the `pthread_join` loop instead of after?
11. **What would happen today** if you ran `./codexion 3 800 200 200 200 3 0 fifo` (quota = 3)? Does it stop? Why or why not?
12. **What would happen** if you wired in the monitor but did NOT add `last_compile_start = start_time` to init?
13. **What would happen** if `log_state` kept printing unconditionally after the monitor set `stop` and printed `burned out`?
14. Why is locking required merely to *read* the single integer `stop` in `should_stop`, when reading an int seems harmless?
15. If `pthread_create` failed for coder 2 (returned non-zero) in the current `main.c`, what would go wrong, and what is missing that would catch it?

### ANSWERS

1. A process is a running program with its own private memory; a thread is one stream of execution inside a process, and threads of one process **share** memory. Codexion uses **one process with N threads** (coders), which is why locks are needed.
2. The field that matters is `lock` (the mutex). Unused: `cond`, `available`, `available_at` (and `id` is only for printing). They are placeholders for the cooldown/waiting-queue features.
3. So two threads printing at the same instant do not interleave character-by-character into one scrambled line; `print_lock` serializes output so each line prints whole.
4. Because `++` is really read-modify-write (three steps). Two threads interleaving those steps can lose an increment (5→6 instead of 5→7). The lock makes the three steps atomic relative to the other thread (here, the monitor reading it).
5. It locks dongle **0 first** (lower index wins). Because every coder takes the lower-numbered dongle first, no cycle of "everyone holds one and waits for the next" can form — the circular-wait condition is broken.
6. Because a coder only needs both dongles to compile; debug and refactor need none. Releasing immediately after compiling lets neighbours use those dongles in parallel, which is the concurrency the sim is demonstrating.
7. A single long `usleep` can oversleep by several milliseconds, which would blow a burnout deadline. The short-nap loop re-checks the real clock and lands on target, while `usleep(200)` keeps it from busy-spinning at 100% CPU.
8. Mutual exclusion, hold-and-wait, no preemption, circular wait. Codexion attacks **circular wait** (via consistent lock ordering).
9. All three grab their left dongle at once: coder 1 holds 0 wants 1, coder 2 holds 1 wants 2, coder 3 holds 2 wants 0. Every dongle held, every coder waiting on the next → a closed cycle → deadlock: a few "has taken a dongle" lines then permanent silence.
10. `cleanup` would free the arrays and destroy mutexes while coder threads are still running and using them → use-after-free and illegal mutex operations → crashes or garbage. Join-before-cleanup exists precisely to prevent this.
11. **It does not stop.** Even with quota=3, nothing checks the quota, because the monitor thread is never created (not in `main.c`, Makefile, or header). The coders loop forever and `main` blocks on `join(coder 1)`. You must Ctrl-C.
12. Every coder would "burn out" almost instantly, because `last_compile_start` is 0 after `memset`, so `now - 0` is a huge number that exceeds `time_to_bo` on the monitor's first check.
13. A coder could print a status line (e.g. `is debugging`) *after* the `burned out` line, making the log look like work continued past the end. The fix is for `log_state` to check `should_stop` under `print_lock` and skip printing once stopped.
14. Because without a lock (a memory barrier) there is no guarantee one thread's write to `stop` ever becomes visible to another thread; it could read a stale cached 0 indefinitely. Locking forces fresh, published memory, so the stop is actually seen.
15. `main.c` ignores `pthread_create`'s return value, so it would proceed as if coder 2 exists and later `pthread_join` a thread that was never created — undefined behaviour. What is missing is a check of the return value (non-zero = failure) to stop creating and shut down cleanly.

---

# QUESTIONS FOR YOU (genuine ambiguities in the code)

1. **Monitor wiring:** `monitor.c` exists but is not in the Makefile, header, or `main.c`. Do you want it connected now (I can give the exact 4 edits), or left as the next step?
2. **Burnout clock priming:** `last_compile_start` is never set to `start_time` in `init.c`. Confirm you want it primed there — without it, burnout fires instantly.
3. **`log_state` stop-gate:** right now it prints unconditionally. Should it check `stop` under `print_lock` (the fix for "no message after burnout")?
4. **Unused fields/feature flags:** `cond`, `available`, `available_at`, `dongle_cooldown`, `seq_counter`, and the `FIFO`/`EDF` scheduler are all declared but unused. Confirm these are intended for later days (cooldown, waiting queues, scheduling) and not dead code to remove.
5. **`pthread_create` return not checked** in `main.c`. Do you want error handling added, or is it acceptable for this project's scope?
6. **Committed build artifacts:** `codexion` and `objs/` appear in git history. Want a `.gitignore` so generated files stop being committed?
