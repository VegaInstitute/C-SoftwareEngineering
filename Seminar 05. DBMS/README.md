# Database Management Systems

*“There are no solutions. There are only trade-offs.”* — Thomas Sowell, A Conflict of Visions: Ideological Origins of Political Struggles

## Why DB and not a bunch of files?
* selective work with data,
* concurrent access,
* fault tolerance,
* effective operations with data
* …

If you don't need these features, it's OK to store data in files.

## Which trade-offs we encounter when choosing the DBMS?
- **Specialized solutions always beat non-specialized, but only in the field of their specialization.**
- OLAP / OLTP
- Centralized / Distributed / Federated
- hierarchical / network / relational / object-oriented / object-relational / NoSQL
Many DBMS combines several variants, specializing on some more than on others.

## Links
DMBS:
- [ClickHouse](https://clickhouse.com)
- [PostgreSQL](https://www.postgresql.org/)
- [MySQL](https://www.mysql.com/)
- [MongoDB](https://www.mongodb.com/)
- [KDB+](https://kx.com/products/kdb/)
- [Influx](https://www.influxdata.com/)
- [Oracle](https://www.oracle.com/database/)
- [SQLite](https://www.sqlite.org/)

Around:

- [Spark](https://spark.apache.org/)
- [SQLAlchemy](https://www.sqlalchemy.org/)
- [DBeaver](https://dbeaver.io/)

## OK, how to work with DBMS?
**SQL – absolute base.**

How to learn SQL?

1. Read the official docs to chosen DBMS.
2. [SQLBolt](https://sqlbolt.com/)
3. …

I chose the DBMS, what's next? What do I need to figure out?
1. How to install it.
2. How does management work. How to create roles, users, volumes, etc. 
3. How the data is stored: data types, structures, etc.
4. How to work with data: interfaces, libraries, etc.