
# Distributed Inventory Management System

This repository contains a C++ project that implements a distributed inventory management system using inter-process communication (IPC) through named pipes and standard pipes. The system manages product inventories across multiple stores and parts.

**Course**: Operating System – Autumn 2024  
**University of Tehran | School of Electrical & Computer Engineering**

---

## Project Contents

- **main.cpp** - The main process responsible for system initialization, creating processes, and managing communication.
- **logger.cpp** - A simple logger for printing system events.
- **part.cpp** - Manages inventory for specific parts.
- **store.cpp** - Handles product transactions for individual stores.
- **Makefile** - Build configuration for compiling the project.

---

## System Features

### Core Features:
- **Multi-Process System:**
  - The system spawns processes for stores and parts.
  - Each process communicates using pipes.

- **Inventory Management:**
  - Manages parts and products through transactions (input/output).
  - Calculates total inventory and profits.

- **Logging:**
  - Logs system events (INFO, ERROR) with process-specific details.

### Communication:
- **Named Pipes:** For inter-process communication between stores and parts.
- **Standard Pipes:** For process management and parent-child communication.

---

## How to Build and Run

1. **Clone the repository:**
   ```bash
   git clone https://github.com/YourUsername/YourRepo.git
   ```

2. **Navigate to the project directory:**
   ```bash
   cd YourRepo
   ```

3. **Build the project:**
   ```bash
   make
   ```

4. **Run the system:**
   ```bash
   ./main <path_to_stores_directory>
   ```

---

## How It Works

1. **System Initialization:**
   - Reads the list of stores and parts from a CSV file.
   - Creates necessary named pipes.

2. **Process Creation:**
   - Spawns processes for each store and part.
   - Initializes inter-process communication.

3. **Data Processing:**
   - Each store reads its data file and processes transactions.
   - Each part aggregates results from relevant stores.

4. **Result Calculation:**
   - Final results, including profits and product summaries, are displayed.

5. **Clean-up:**
   - Deletes all named pipes and closes processes.
