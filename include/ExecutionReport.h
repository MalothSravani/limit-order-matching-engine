#ifndef EXECUTION_REPORT_H
#define EXECUTION_REPORT_H

#include <string>

enum class ExecutionType
{
    ACCEPTED,
    FILLED,
    PARTIALLY_FILLED,
    CANCELLED,
    REJECTED,
    MODIFIED
};

struct ExecutionReport
{
    int orderId;

    ExecutionType type;

    int quantity;

    double price;

    std::string message;
};

#endif