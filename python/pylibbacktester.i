%module(directors="1") pylibbacktester

%{
    #include <string>
    #include <vector>

    #include "libbacktester/types.hpp"
    #include "libbacktester/strategy.hpp"
    #include "libbacktester/strategies/strategy_factory.hpp"
    #include "libbacktester/bindings.hpp"
%}

/*** Standard SWIG helpers ***/
%include "std_string.i"
%include "std_vector.i"
%include "stdint.i"

/* Expose std::vector<OrderIntent> to Python as a list-like container. */
namespace std {
  %template(OrderIntentVector) vector<libbacktester::OrderIntent>;
}

/* Namespace-level alias (if used in public APIs): using Ms = std::chrono::milliseconds; */
%typemap(in) libbacktester::Ms {
  long long ms_val;
  if (!PyLong_Check($input)) {
    SWIG_exception_fail(
        SWIG_TypeError,
        "Expected integer milliseconds for libbacktester::Ms");
  }
  ms_val = PyLong_AsLongLong($input);
  if (PyErr_Occurred()) {
    SWIG_exception_fail(
        SWIG_OverflowError,
        "Milliseconds value out of range for libbacktester::Ms");
  }
  $1 = libbacktester::Ms(ms_val);
}

%typemap(out) libbacktester::Ms {
  $result = PyLong_FromLongLong($1.count());
}

%typemap(directorin) libbacktester::Ms {
  $input = PyLong_FromLongLong($1.count());
}

%include "libbacktester/types.hpp"                       // OrderSide, OrderIntent, FillEvent, Ms
%include "libbacktester/strategy.hpp"                    // Strategy
%include "libbacktester/strategies/strategy_factory.hpp" // TWAPParams, StrategyFactory
%include "libbacktester/bindings.hpp"                    // RunSpec, RunResult, run_backtest, etc.

%feature("director") libbacktester::Strategy;
