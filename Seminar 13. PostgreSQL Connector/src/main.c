#include "stdio.h"
#include "stdio.h"
#include "libpq-fe.h"
#include <stdio.h>

// C PostgreSQL library: libpq
// C++ PostgreSQL library: pqxx

int main(int argc, char **argv)
{
    char *connection_info;
    PGconn *connection;
    PGresult *result;

    int n_fields;
    int i;
    int j;

    if (argc > 1)
    {
        connection_info = argv[1];
    }
    else {
        connection_info = "dbname = sample_database";
    }

    connection = PQconnectdb(connection_info);
    if (PQstatus(connection) != CONNECTION_OK)
    {
        fprintf(stderr, "Error message: %s", PQerrorMessage(connection));
        PQfinish(connection);
        return -1;
    }

    result = PQexec(connection, "CREATE TABLE order_log (order_no integer NOT NULL, ticker char ( 4 ) NOT NULL, price float NOT NULL);");
    if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "CREATE TABLE failed. Error message: %s", PQerrorMessage(connection));
        PQfinish(connection);
        return -1;
    }

    result = PQexec(connection, "INSERT INTO order_log VALUES ( 1, 'TCKR', 100.2 );");
    if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "INSERT failed. Error message: %s", PQerrorMessage(connection));
        PQfinish(connection);
        return -1;
    }

    result = PQexec(connection, "SELECT * FROM order_log;");
    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "SELECT failed. Error message: %s", PQerrorMessage(connection));
        PQfinish(connection);
        return -1;
    }

    n_fields = PQnfields(result);
    for (i = 0; i < n_fields; i++)
    {
        printf("%-15s", PQfname(result, i));
    }
    printf("\n\n");

    for (i = 0; i < PQntuples(result); i++)
    {
        for (j = 0; j < n_fields; j++)
        {
            printf("%-15s", PQgetvalue(result, i, j));
        }
        printf("\n");
    }

    result = PQexec(connection, "DROP TABLE order_log;");
    if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "CREATE TABLE failed. Error message: %s", PQerrorMessage(connection));
        PQfinish(connection);
        return -1;
    }

    PQclear(result);
    PQfinish(connection);
    return 0;
}
