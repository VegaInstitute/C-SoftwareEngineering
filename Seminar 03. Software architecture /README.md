# Seminar 3 – Software Architecture
## Introduction

**Question 1**. Why is it important to have a well-though project structure?

There are two fundamental laws in software architecture:
* Everything is a trade-off;
* "Why" is more important than "how".

We won't consider many of the possible patterns and shall look at the most suitable ones for our purposes.

## Monolithic Architecture
Monolithic architecture is a traditional approach where the entire financial application is built as a single, self-contained unit.

Key characteristics:
* All components (UI, business logic, data access) are tightly integrated in one codebase
* Simpler to develop and deploy initially
* Can offer good performance due to direct function calls

However, monolithic architectures have some significant drawbacks for large financial systems:
* Limited scalability - difficult to scale individual components
* Reduced flexibility and agility as the system grows
* Challenging to maintain and update as complexity increases

**Question 2**. How can we improve the monolithic architecture?

## Modular Monoliths
Modular monolith is an evolution of monolithic architecture that aims to address some of its limitations while retaining some benefits.

Key aspects:
* Application is divided into distinct modules, but still deployed as a single unit
* Improves code organization and maintainability
* Allows for better separation of concerns

Benefits for financial systems:
* Improved modularity and maintainability compared to monoliths
* Easier to manage changes and improve code clarity
* Can be a good intermediate step between monolith and microservices

**Question 3**. What are the drawbacks of monoliths that the modular architecture failed to fix?


## Microservices Architecture
Microservices architecture is an approach to building a software system as a collection of small, independent services. Each service is self-contained and can be deployed, scaled, and updated independently. 

Key features:
* Each service is responsible for specific functionality
* Services communicate via well-defined APIs
* Can be developed, deployed, and scaled independently

Benefits for financial systems:
* Enhanced scalability - services can be scaled individually based on demand
* Improved fault isolation and resilience
* Accelerated development and deployment of new features
* Better security through service isolation

**Question 4**. What are the main challenges of microservices architecture for trading systems?

## Conclusion
When choosing an architecture for financial software, especially HFT systems, it's important to consider factors like:
* Ultra-low latency requirements
* High throughput needs
* Real-time processing capabilities
* Scalability and flexibility demands
* Regulatory compliance and security requirements

Often, financial institutions may use a combination of these patterns or evolve from one to another as they grow and requirements change.