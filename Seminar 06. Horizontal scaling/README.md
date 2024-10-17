# Seminar 05. Horizontal Scaling. Multithreading vs Multiprocessing
## Horizontal vs Vertical Scaling: Motivation and Differences

### **Horizontal Scaling**
- Horizontal scaling refers to increasing performance by adding more machines or nodes to a system, allowing for the **distribution of tasks**. This is particularly useful in large-scale systems that need to process vast amounts of data or perform computationally intensive tasks in parallel.
- **Advantages**:
  - Improved **fault tolerance** (if one machine fails, the system remains operational)
  - Better scalability, especially for high-velocity data streams like those in HFT.
- **Advantages**:
  - Requires more complex architecture, such as distributed systems or cloud-based setups.

### **Vertical Scaling**
- Vertical scaling improves performance by **upgrading hardware components** (e.g., more CPU cores, increased memory, or faster storage on a single machine).
- **Motivation**:
  - Easier to implement than horizontal scaling
  - Simpler infrastructure, no need for distributed system complexities
- **Drawback**:
  - **Diminishing returns** as hardware improvements can be costly and are limited by physical constraints.

### **When to Use Horizontal vs Vertical Scaling**
- **Horizontal Scaling** is more suitable for distributed systems requiring **fault tolerance**, such as HFT platforms processing market data in real time.
- **Vertical Scaling** works well in systems where hardware improvements are enough to handle the workload, but is limited when you need to handle massive amounts of data, like backtesting complex trading algorithms over historical datasets.

Use Cases in Quantitative Finance:
- **Risk engines** (e.g., Monte Carlo simulations for VaR).
- **Real-time analytics** for trading strategies (e.g., machine learning or AI models).
- **Distributed backtesting** across multiple machines to analyze performance under varying market conditions.

## Multithreading vs Multiprocessing

### **Multithreading**
- **Multithreading** refers to running multiple threads within a single process. Each thread runs concurrently and shares the same memory space, making this technique highly efficient for tasks that need to frequently communicate or share resources.

- **Advantages**:
  - Low overhead due to shared memory space
  - Ideal for tasks that require real-time data updates, such as **order book processing** in HFT
  - Efficient for CPU-bound tasks where threads work in parallel to complete a task faster.

- **Drawbacks**:
  - **Thread contention** and synchronization issues can arise, requiring careful management of shared resources.
  - More complex to implement due to race conditions and deadlocks.

### **Multiprocessing**
- **Multiprocessing** refers to running multiple processes concurrently. Each process has its own memory space, making it suitable for tasks that are more independent from one another.

- **Advantages**:
  - Better isolation of tasks (no shared memory issues)
  - More stable: if one process crashes, the others remain unaffected.

- **Drawbacks**:
  - Higher overhead due to inter-process communication and memory isolation.
  - Less efficient for tightly coupled tasks compared to multithreading.

### **Choosing Between Multithreading and Multiprocessing**
- **Multithreading** is suitable for real-time, data-intensive applications like market data processing in HFT systems, where minimal latency and fast communication between tasks is critical.
- **Multiprocessing** works well for independent tasks, such as running multiple, isolated Monte Carlo simulations in parallel.

## Seminar Takeaways

- **Horizontal vs Vertical Scaling**: Horizontal scaling is key for distributing workloads in systems that handle large amounts of data, such as backtesting or real-time market data processing in HFT, whereas vertical scaling is limited by hardware.

- **Multithreading vs Multiprocessing**: Multithreading provides faster communication for tightly coupled tasks but introduces complexity, while multitasking offers more isolation and fault tolerance for independent tasks.

- **CRTP vs Virtual Functions**: CRTP is ideal for high-performance applications requiring compile-time resolution, such as HFT systems, while virtual functions offer the flexibility needed for dynamic, runtime systems.
