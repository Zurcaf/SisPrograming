# Space Trash Project - Part B Implementation Checklist

## 📋 Project Setup

- [ ] Create project structure with `shared/`, `new-universe-server/`, `new-trash-ship-client/`, `new-dashboard/`
- [ ] Set up `configs/` folder with `server.cfg`, `client.cfg`, `dashboard.cfg`
- [ ] Create shared Makefile
- [ ] Test compilation of Part A code in new structure

---

## 🔧 Phase 1: Server Foundation (Days 1-2)

### 1. Rename & Restructure

- [x] Copy `universe-server` → `new-universe-server`
- [x] Move shared files to `shared/` directory
- [x] Update include paths in all files
- [ ] Create basic thread scaffolding (6 threads)
- [ ] Add `pthread_mutex_t universe_mutex`
- [ ] Test compilation with empty thread functions

### 2. Configuration System

- [x] Create `shared/config-reader.c` and `config-reader.h`
- [x] Implement `read_config()` function with libconfig
- [ ] Create `configs/server.cfg` with all parameters
- [ ] Test reading configuration in server
- [ ] Add command-line argument for config file path

### 3. Periodic Operations (Non-threaded First)

- [ ] Implement `generate_new_trash()` function
- [ ] Add timer for 10s trash generation
- [ ] Implement `switch_recycling_planet()` function
- [ ] Add timer for 30s planet switching
- [ ] Test both periodic operations in main loop
- [ ] Mark recycling planet visually in display

### 4. Display Rate Control

- [ ] Implement 30Hz (33ms) display update timer
- [ ] Decouple display from physics calculations
- [ ] Use `SDL_GetTicks()` or `clock_gettime()` for timing
- [ ] Test display smoothness

---

## ⚙️ Phase 2: Physics Integration (Days 3-4)

### 5. Ship Data Structures

- [ ] Add velocity vector to ship structure (`vector velocity`)
- [ ] Add acceleration vector to ship structure (`vector acceleration`)
- [ ] Add cargo counter (`int cargo_count`)
- [ ] Add active flag (`bool active`)
- [ ] Initialize ship physics variables

### 6. Ship Physics Implementation

- [ ] Copy `new_trash_acceleration()` → `new_ship_acceleration()`
- [ ] Copy `new_trash_velocity()` → `new_ship_velocity()`
- [ ] Copy `new_trash_position()` → `new_ship_position()`
- [ ] Apply 1% friction to ships
- [ ] Test ship gravitational movement
- [ ] Verify ship wraps around universe edges

### 7. Thrust Mechanism

- [ ] Create `apply_thrust()` function
- [ ] Handle UP arrow → thrust in +Y direction
- [ ] Handle DOWN arrow → thrust in -Y direction
- [ ] Handle LEFT arrow → thrust in -X direction
- [ ] Handle RIGHT arrow → thrust in +X direction
- [ ] Test thrust changes velocity (not position directly)
- [ ] Verify no thrust = only gravity affects ship

### 8. Physics Thread

- [ ] Create physics update thread (10ms loop)
- [ ] Add mutex lock before updating trash positions
- [ ] Add mutex lock before updating ship positions
- [ ] Use `nanosleep()` or `usleep()` for 10ms timing
- [ ] Verify thread doesn't block main loop
- [ ] Test physics calculations are smooth

---

## 🌐 Phase 3: Communication System (Days 5-7)

### 9. Protocol Buffers Setup

- [ ] Install protobuf-c library
- [ ] Create `messages.proto` file
- [ ] Define `ConnectRequest` message
- [ ] Define `ConnectResponse` message (ship_id, success)
- [ ] Define `ThrustCommand` message (direction, ship_id)
- [ ] Define `UniverseState` message (planets, trash, ships)
- [ ] Define `Statistics` message (for dashboard)
- [ ] Define `DisconnectNotification` message
- [ ] Compile `.proto` → `.pb-c.c` and `.pb-c.h`
- [ ] Add protobuf files to Makefile

### 10. Server Communication - REP Socket

- [ ] Create REP socket thread
- [ ] Bind REP socket to configured port
- [ ] Implement `handle_connect_request()`
- [ ] Assign ship to new client (find free slot)
- [ ] Return error if max ships reached
- [ ] Implement `handle_thrust_command()`
- [ ] Apply thrust to correct ship
- [ ] Add mutex locks for ship modifications
- [ ] Validate ship_id in all requests (prevent cheating)

### 11. Server Communication - PUB Socket

- [ ] Create PUB socket thread
- [ ] Bind PUB socket to configured port
- [ ] Implement `serialize_universe_state()`
- [ ] Publish universe state at 30Hz
- [ ] Add mutex lock when reading universe data
- [ ] Publish statistics for dashboard

### 12. Client Disconnect Detection

- [ ] Implement timeout on REP socket (e.g., 5 seconds)
- [ ] Add heartbeat mechanism (optional)
- [ ] Mark ship as inactive on timeout
- [ ] Implement `remove_ship()` function
- [ ] Test graceful disconnect (client sends disconnect message)
- [ ] Test abrupt disconnect (client crashes)

---

## 💻 Phase 4: Client Implementation (Days 8-9)

### 13. Client Configuration

- [ ] Create `configs/client.cfg`
- [ ] Implement config reading in client
- [ ] Get server address and ports from config

### 14. Client Communication - REQ Socket

- [ ] Create REQ socket thread
- [ ] Connect to server REP socket
- [ ] Send `ConnectRequest` on startup
- [ ] Receive and store assigned `ship_id`
- [ ] Handle connection rejection (max ships)
- [ ] Send thrust commands from keyboard thread
- [ ] Add error handling for send failures

### 15. Client Communication - SUB Socket

- [ ] Create SUB socket thread
- [ ] Connect to server PUB socket
- [ ] Subscribe to universe state topic
- [ ] Receive and deserialize `UniverseState`
- [ ] Update local universe data copy
- [ ] Add mutex for local data protection

### 16. Client Display

- [ ] Copy `display.c/h` to client (or use shared)
- [ ] Create SDL window in client
- [ ] Render universe from received state
- [ ] Highlight controlled ship (different color)
- [ ] Show cargo count on screen
- [ ] Show current recycling planet
- [ ] Test display matches server exactly

### 17. Client Keyboard Processing

- [ ] Create keyboard thread or handle in main
- [ ] Detect SDL_KEYDOWN events
- [ ] Send thrust command to REQ thread
- [ ] Handle ESC key for graceful disconnect
- [ ] Send disconnect message to server

### 18. Client Threads Coordination

- [ ] Main thread: SDL event loop + display (30Hz)
- [ ] Thread 2: REQ socket (send commands)
- [ ] Thread 3: SUB socket (receive state)
- [ ] Add mutexes for shared data
- [ ] Test all threads running concurrently

---

## 📊 Phase 5: Dashboard (Day 10)

### 19. Dashboard Setup

- [ ] Choose language (Python recommended)
- [ ] Install ZMQ library (`pip install pyzmq`)
- [ ] Install protobuf library (`pip install protobuf`)
- [ ] Create `new-dashboard.py`
- [ ] Create `configs/dashboard.cfg`

### 20. Dashboard Communication

- [ ] Connect SUB socket to server PUB
- [ ] Subscribe to statistics topic
- [ ] Deserialize statistics messages
- [ ] Parse planet data (recycled trash counts)
- [ ] Parse ship data (current cargo)
- [ ] Parse universe data (total roaming trash)

### 21. Dashboard Display

- [ ] Print header with timestamp
- [ ] Print per-planet recycled trash
- [ ] Print per-ship cargo status
- [ ] Print total trash / max trash ratio
- [ ] Print warning if near universe collapse
- [ ] Update display in real-time (clear screen between updates)
- [ ] Handle connection loss gracefully

---

## 🎯 Phase 6: Game Logic Completion (Days 11-12)

### 22. Ship-Trash Interaction

- [ ] Implement collision detection (ship touches trash)
- [ ] Add trash to ship cargo on collision
- [ ] Remove trash from universe
- [ ] Check cargo capacity before collection
- [ ] Display warning if cargo full
- [ ] Test trash collection multiple times

### 23. Ship-Planet Interaction

- [ ] Detect ship collision with planet (distance < radius)
- [ ] Check if planet is recycling planet
- [ ] If recycling: add cargo to planet count, reset ship cargo
- [ ] If non-recycling: spill all cargo as new trash
- [ ] Update planet statistics
- [ ] Test deposit and spill scenarios

### 24. Universe Limits

- [ ] Check trash count vs max_trash every update
- [ ] Trigger universe collapse if limit reached
- [ ] Send collapse notification to all clients
- [ ] Display collapse message on all screens
- [ ] Implement restart mechanism (or graceful shutdown)
- [ ] Test universe collapse scenario

### 25. Trash Generation Rules

- [ ] Only generate trash if active ships exist
- [ ] Generate trash at random positions
- [ ] Avoid spawning trash inside planets (optional)
- [ ] Stop generation when no ships active

---

## 🔒 Phase 7: Error Handling & Security (Day 13)

### 26. Server-Side Validation

- [ ] Validate all message types from clients
- [ ] Check ship_id belongs to requesting client
- [ ] Prevent clients from moving other ships
- [ ] Validate thrust magnitude/direction
- [ ] Handle malformed protobuf messages
- [ ] Log suspicious activity

### 27. Network Error Handling

- [ ] Handle ZMQ socket creation failures
- [ ] Handle bind/connect failures
- [ ] Retry connections with exponential backoff
- [ ] Display user-friendly error messages
- [ ] Handle partial message receives
- [ ] Handle serialization/deserialization errors

### 28. Resource Management

- [ ] Check malloc/calloc return values
- [ ] Free all allocated memory on shutdown
- [ ] Close all sockets properly
- [ ] Destroy all mutexes
- [ ] Join all threads before exit
- [ ] Run with `valgrind` to check leaks

---

## 🧪 Phase 8: Testing & Polish (Day 14)

### 29. Functionality Testing

- [ ] Test single client connection
- [ ] Test multiple clients (3+) simultaneously
- [ ] Test client disconnect during gameplay
- [ ] Test client reconnect
- [ ] Test max ships limit enforcement
- [ ] Test cargo capacity enforcement
- [ ] Test recycling planet changes
- [ ] Test universe collapse
- [ ] Test all ship movements (all directions)
- [ ] Test trash generation periodicity

### 30. Stress Testing

- [ ] Run with maximum planets
- [ ] Run with maximum trash
- [ ] Run with maximum ships
- [ ] Test rapid client connect/disconnect
- [ ] Test rapid thrust commands
- [ ] Monitor CPU usage
- [ ] Monitor memory usage
- [ ] Check for deadlocks

### 31. Code Quality

- [ ] Add comments to all functions
- [ ] Add header comments to all files
- [ ] Check code formatting consistency
- [ ] Remove debug print statements
- [ ] Remove unused variables/functions
- [ ] Verify no compiler warnings
- [ ] Run static analysis (cppcheck, clang-tidy)

### 32. Documentation

- [ ] Write README.md with compilation instructions
- [ ] Document configuration file format
- [ ] Document protocol buffer messages
- [ ] Add usage examples
- [ ] List required dependencies
- [ ] Include troubleshooting section

---

## 📦 Phase 9: Submission Preparation

### 33. Final Checks

- [ ] Test complete build from scratch (`make clean && make`)
- [ ] Verify all Makefiles work
- [ ] Test on clean system (if possible)
- [ ] Check file structure matches requirements
- [ ] Verify no absolute paths in code
- [ ] Remove temporary files (\*.o, executables from testing)

### 34. Submission Package

- [ ] Create clean directory structure
- [ ] Include all source files
- [ ] Include all header files
- [ ] Include all Makefiles
- [ ] Include configuration files
- [ ] Include README.md
- [ ] Include .proto files
- [ ] Create zip file: `project-b-groupXX.zip`
- [ ] Verify zip extracts correctly
- [ ] Submit to FENIX before deadline (Jan 9, 2026, 19:00)

---

## 🎓 Extra Credit / Advanced Features

- [ ] Implement continuous thrust (pedal down/up)
- [ ] Add ship-to-ship collision detection
- [ ] Add visual effects (trails, explosions)
- [ ] Add sound effects
- [ ] Implement ship health/damage system
- [ ] Add replay functionality
- [ ] Create spectator mode
- [ ] Add AI-controlled ships
- [ ] Implement powerups
- [ ] Add high score table

---

## 📝 Notes & Debugging

### 35. Common Issues Checklist

- [ ] Checked that ZMQ sockets aren't shared between threads?
- [ ] Verified all mutexes are locked/unlocked properly?
- [ ] Confirmed SDL functions only called from main thread?
- [ ] Tested with different config values?
- [ ] Checked for race conditions?
- [ ] Verified proper cleanup on exit?

### 36. Performance Issues

- [ ] Physics calculations taking too long?
- [ ] Display updates causing lag?
- [ ] Network messages too large?
- [ ] Too many mutex contentions?

### 37. Bugs Found

```
Date:
Issue:
Fix:

Date:
Issue:
Fix:
```

---

**Total Checkboxes: ~200**  
**Estimated Time: 14 days**  
**Deadline: January 9, 2026, 19:00**

Good luck! 🚀✨
