# PostgreSQL Connector
PostgreSQL also known as Postgres, is a free and open-source relational database management system (RDBMS) emphasizing extensibility and SQL compliance.

Steps to create the database and get it running:
1. Install PostgreSQL (via brew/apt/etc.);
2. Start the service (e.g. `brew services start postgresql`);
3. Create the database:
```shell
createdb <DATABASE_NAME>
```
4. Run interactive PostgreSQL terminal:
```shell
psql <DATABASE_NAME>
```
You are ready to use the database.
